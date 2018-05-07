require 'torch'
require 'cutorch'
require 'optim'
require 'xlua'
require 'nn'
dofile './provider.lua'

torch.manualSeed(1)

opt_string = [[
    -h,--help                                       print help
    -s,--save               (default "logs")        subdirectory to save logs
    -b,--batchSize          (default 64)            batch size
    -r,--learningRate       (default 0.01)          learning rate
    --learningRateDecay     (default 1e-7)          learning rate decay
    --weigthDecay           (default 0.001)        weight decay
    -m,--momentum           (default 0.9)           mementum
    --epoch_step            (default 20)            epoch step
    -g,--gpu_index          (default 0)             GPU index (start from 0)
    --max_epoch             (default 200)           maximum number of epochs
    --jitter_step           (default 2)             jitter augmentation step size
    --train_data            (default "data/h5_scannet_samples/trainval_shape_voxel_data_list.txt")     txt file containing train h5 filenames
    --test_data             (default "data/h5_scannet_samples/test_shape_voxel_data_list.txt")      txt file containing test h5 filenames
    --retrain               (default "")            retrain model
    --class_hist_file       (default "classes_hist.txt")        histogram for weight norm
    --class_map_file        (default "")
    --orig_num_classes      (default 42)
]]

opt = lapp(opt_string)

-- print help or chosen options
if opt.help == true then
    print('Usage: th train.lua')
    print('Options:')
    print(opt_string)
    os.exit()
else
    print(opt)
end

-- set gpu
cutorch.setDevice(opt.gpu_index+1)

-- load model
num_classes = opt.orig_num_classes
local class_map = nil
if paths.filep(opt.class_map_file) then
    class_map, num_classes = readClassMap(opt.class_map_file)
    num_classes = num_classes + 2 --for empty and unannotated
    print('using class map ' .. #class_map .. ' -> ' .. num_classes)
    io.read()
end
local model, criterion = dofile('model.lua')
model = model:cuda()
model:zeroGradParameters()
parameters, gradParameters = model:getParameters()
print(model)

-- set criterion
if not criterion then
    local criterion_weights = torch.ones(num_classes)
    --empty and unannotated are 0 weight
    criterion_weights[1] = 0
    criterion_weights[num_classes] = 0
    if paths.filep(opt.class_hist_file) then 
        criterion_weights = readClassesHist(opt.class_hist_file, num_classes)
        for i = 1,num_classes do
            if criterion_weights[i] > 0 then criterion_weights[i] = 1 / torch.log(1.2 + criterion_weights[i]) end
        end
    end
    print(criterion_weights)
    --io.read()
    criterion = cudnn.SpatialCrossEntropyCriterion(criterion_weights):cuda()
end

-- load training and testing files
train_files = getDataFiles(opt.train_data)
test_files = getDataFiles(opt.test_data)
print('#train_files = ' .. #train_files)
print('#test_files = ' .. #test_files)

if not paths.dirp(opt.save) then paths.mkdir(opt.save) end
do
    local optfile = assert(io.open(paths.concat(opt.save, 'options.txt'), 'w'))
    local cur = io.output()
    io.output(optfile)
    serialize(opt)
    serialize(train_files)
    serialize(test_files)
    io.output(cur)
    optfile:close()
end

-- config for SGD solver
optimState = {
    learningRate = opt.learningRate,
    weightDecay = opt.weigthDecay,
    momentum = opt.momentum,
    learningRateDecay = opt.learningRateDecay,
}

-- config logging
testLogger = optim.Logger(paths.concat(opt.save, 'test.log'))
testLogger:setNames{'% mean accuracy (train set)', '% mean accuracy (test set)', '% mean class accuracy (train set)', '% mean class accuracy (test set)'}
testLogger.showPlot = 'false'

-- confusion matrix
confusion = optim.ConfusionMatrix(num_classes)


------------------------------------
-- Training routine
--

function train()
    model:training()
    epoch = epoch or 1 -- if epoch not defined, assign it as 1
    print('epoch ' .. epoch)
    if epoch % opt.epoch_step == 0 then optimState.learningRate = optimState.learningRate/2 end

    -- shuffle train files
    local train_file_indices = torch.randperm(#train_files)

    local tic = torch.tic()
    for fn = 1, #train_files do
        local current_data, current_label = loadDataFile(train_files[train_file_indices[fn]], num_classes, class_map)
        local column_zsize = current_data:size(3) 

        local filesize = (#current_data)[1]
        local targets = torch.CudaTensor(opt.batchSize, 1, column_zsize)
        local mask = torch.CudaTensor(opt.batchSize*column_zsize)
        local indices = torch.randperm(filesize):long():split(opt.batchSize)
        -- remove last mini-batch so that all the batches have equal size
        indices[#indices] = nil 

        for t, v in ipairs(indices) do 
            -- print progress bar :D
            xlua.progress(t, #indices)

            local inputs = current_data:index(1,v):cuda()
            targets:copy(current_label:index(1,v))

            if targets:numel() ~= mask:numel() then mask:resize(targets:numel()) end
            mask:copy(targets:view(-1))
            mask[mask:eq(1)] = 0           --empty
            mask[mask:eq(num_classes)] = 0 --unlabeled
            local maskindices = mask:float():nonzero()            

            -- a function that takes single input and return f(x) and df/dx
            local feval = function(x)
                if x ~= parameters then parameters:copy(x) end
                gradParameters:zero()
                
                local outputs = model:forward(inputs)
                local f = criterion:forward(outputs, targets)
                local df_do = criterion:backward(outputs, targets)
                model:backward(inputs, df_do) -- gradParameters in model have been updated
              
                local y = outputs:transpose(2, 4):transpose(2, 3)
                y = y:reshape(y:numel()/y:size(4), num_classes):sub(1,-1,1,num_classes-1)
                local _, predictions = y:max(2)
                predictions = predictions:view(-1)
                local k = targets:view(-1)
                confusion:batchAdd(predictions:index(1,maskindices), k:index(1,maskindices))
                return f, gradParameters
            end
            
            if maskindices:numel() ~= 0 then
                maskindices = torch.squeeze(maskindices,2)
                optim.sgd(feval, parameters, optimState)
            end
        end
    end
    
    confusion:updateValids()
    print(('Train accuracy: '..'%.2f | %.2f'..' %%\t time: %.2f s'):format(
            confusion.totalValid * 100, confusion.averageValid * 100, torch.toc(tic)))

    train_acc = confusion.totalValid * 100
	train_avg = confusion.averageValid * 100

    confusion:zero()
    epoch = epoch + 1
end     


-------------------------------------
-- Test routine
--

function test()
    model:evaluate()
    for fn = 1, #test_files do
        local current_data, current_label = loadDataFile(test_files[fn], num_classes, class_map)
        local column_zsize = current_data:size(3)
            
        local filesize = (#current_data)[1]
        local indices = torch.randperm(filesize):long():split(opt.batchSize)
        local mask = torch.CudaTensor(opt.batchSize*column_zsize)
        for t, v in ipairs(indices) do
            local inputs = current_data:index(1,v):cuda()
            local targets = current_label:index(1,v)
            if targets:numel() ~= mask:numel() then mask:resize(targets:numel()) end
            mask:copy(targets:view(-1))
            mask[mask:eq(1)] = 0           --empty
            mask[mask:eq(num_classes)] = 0 --unlabeled
            local maskindices = mask:float():nonzero()
            if maskindices:numel() ~= 0 then
                maskindices = torch.squeeze(maskindices,2)
                local outputs = model:forward(inputs)

                local y = outputs:transpose(2, 4):transpose(2, 3)
                y = y:reshape(y:numel()/y:size(4), num_classes):sub(1,-1,1,num_classes-1)
                local _, predictions = y:max(2)
                predictions = predictions:view(-1)
                local k = targets:view(-1)
                confusion:batchAdd(predictions:index(1,maskindices), k:index(1,maskindices))
            end
        end
    end
    confusion:updateValids()
    print('Test accuracy:', confusion.totalValid * 100, ' | ', confusion.averageValid * 100)
    -- logging test result to txt and html files
    if testLogger then
        paths.mkdir(opt.save)
        testLogger:add{train_acc, train_avg, confusion.totalValid * 100, confusion.averageValid * 100}
        testLogger:style{'-','-'}
    end

    -- save model every 10 epochs
    if epoch % 10 == 0 then
      local filename = paths.concat(opt.save, 'model.net')
      print('==> saving model to '..filename)
      torch.save(filename, model:clearState())
    end 
    
    confusion:zero()
end

-----------------------------------------
-- Start training
--
for i = 1,opt.max_epoch do
    train()
    test()
end
