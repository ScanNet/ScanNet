require 'torch'
require 'cutorch'
require 'optim'
require 'xlua'
require 'nn'
dofile './provider.lua'

opt_string = [[
    -h,--help                                       print help
    -s,--save               (default "logs")        subdirectory to save logs
    -b,--batchSize          (default 64)            batch size
    -r,--learningRate       (default 0.01)          learning rate
    --learningRateDecay     (default 1e-7)          learning rate decay
    --weigthDecay           (default 0.0005)        weight decay
    -m,--momentum           (default 0.9)           mementum
    --epoch_step            (default 20)            epoch step
    -g,--gpu_index          (default 0)             GPU index (start from 0)
    --max_epoch             (default 200)           maximum number of epochs
    --jitter_step           (default 2)             jitter augmentation step size
    --model                 (default 3dnin_fc)      model name (voxnet, 3dnin, 3dnin_fc, subvolume_sup, aniprobing)
    --train_data            (default "data/h5_shapenet/train_shape_voxel_data_list.txt")     txt file containing train h5 filenames
    --test_data             (default "data/h5_shapenet/test_shape_voxel_data_list.txt")      txt file containing test h5 filenames
    --retrain               (default "")            retrain model
    --classCountFile        (default "")            counts per class to weight the criterion
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
num_classes = 55

-- load model
local model, criterion = dofile('models/'..opt.model..'.lua')
if opt.retrain ~= nil and opt.retrain ~= '' then
    print('loading model ' .. opt.retrain .. ' for retrain')
    model = torch.load(opt.retrain)
end
model = model:cuda()
model:zeroGradParameters()
parameters, gradParameters = model:getParameters()
print(model)

-- set criterion
criterion_weights = torch.ones(num_classes)
if opt.classCountFile ~= "" then
    print('setting criterion weights')
    criterion_weights = readClassWeights(opt.classCountFile, num_classes)
    criterion_weights = criterion_weights / criterion_weights:sum() --normalize
    for i = 1,num_classes do --weight by log hist
        if criterion_weights[i] > 0 then criterion_weights[i] = 1 / torch.log(1.2 + criterion_weights[i]) end
    end
    --print('criterion weights:')
    --print(criterion_weights)
end
criterion = nn.CrossEntropyCriterion(criterion_weights):cuda()

-- load training and testing files
train_files = getDataFiles(opt.train_data)
test_files = getDataFiles(opt.test_data)
print(train_files)
print(test_files)

-- config for SGD solver
optimState = {
    learningRate = opt.learningRate,
    weightDecay = opt.weigthDecay,
    momentum = opt.momentum,
    learningRateDecay = opt.learningRateDecay,
}

-- config logging
testLogger = optim.Logger(paths.concat(opt.save, 'test.log'))
testLogger:setNames{'% mean class accuracy (train set)', '% mean class accuracy (test set)'}
testLogger.showPlot = 'false'

-- confusion matrix
confusion = optim.ConfusionMatrix(num_classes)


------------------------------------
-- Training routine
--

function train()
    model:training()
    epoch = epoch or 1 -- if epoch not defined, assign it as 1
    
    if epoch % opt.epoch_step == 0 then optimState.learningRate = optimState.learningRate/2 end

    -- shuffle train files
    local train_file_indices = torch.randperm(#train_files)

    local tic = torch.tic()
    for fn = 1, #train_files do
        current_data, current_label = loadDataFile(train_files[train_file_indices[fn]])
        current_data = jitter_chunk(current_data, opt.jitter_step)

        local filesize = (#current_data)[1]
        local targets = torch.CudaTensor(opt.batchSize)
        local indices = torch.randperm(filesize):long():split(opt.batchSize)
        -- remove last mini-batch so that all the batches have equal size
        indices[#indices] = nil 

        for t, v in ipairs(indices) do 
            -- print progress bar :D
            xlua.progress(t, #indices)
            
            local inputs = current_data:index(1,v):cuda()
            targets:copy(current_label:index(1,v))
            
            -- a function that takes single input and return f(x) and df/dx
            local feval = function(x)
                if x ~= parameters then parameters:copy(x) end
                gradParameters:zero()
                
                local outputs = model:forward(inputs)
                local f = criterion:forward(outputs, targets)
                local df_do = criterion:backward(outputs, targets)
                model:backward(inputs, df_do) -- gradParameters in model have been updated
                
            if torch.type(outputs) == 'table' then -- multiple outputs, take the last one
                    confusion:batchAdd(outputs[#outputs], targets)
                else
                    confusion:batchAdd(outputs, targets)    
                end
                return f, gradParameters
            end
            
            -- use SGD optimizer: parameters as input to feval will be updated
            optim.sgd(feval, parameters, optimState)
        end
    end
    
    confusion:updateValids()
    print(('Train accuracy: '..'%.2f'..' %%\t time: %.2f s'):format(
            confusion.totalValid * 100, torch.toc(tic)))

    train_acc = confusion.totalValid * 100

    confusion:zero()
    epoch = epoch + 1
end     


-------------------------------------
-- Test routine
--

function test()
    model:evaluate()
    
    for fn = 1, #test_files do
        current_data, current_label = loadDataFile(test_files[fn])
        
        -- notice: volumetric batchnorm requires that both 
        -- train and test are of the same ndim.        
        local filesize = (#current_data)[1]
        local indices = torch.randperm(filesize):long():split(opt.batchSize)
        for t, v in ipairs(indices) do
            local inputs = current_data:index(1,v):cuda()
            local targets = current_label:index(1,v)
            local outputs = model:forward(inputs)

            if torch.type(outputs) == 'table' then -- multiple outputs, take the last one
                confusion:batchAdd(outputs[#outputs], targets)
            else
                confusion:batchAdd(outputs, targets)    
            end
        end
    end
    confusion:updateValids()
    print('Test accuracy:', confusion.totalValid * 100)
    
    -- logging test result to txt and html files
    if testLogger then
        paths.mkdir(opt.save)
        testLogger:add{train_acc, confusion.totalValid * 100}
        testLogger:style{'-','-'}
--[[        testLogger:plot()

        local base64im
        do
          os.execute(('convert -density 200 %s/test.log.eps %s/test.png'):format(opt.save,opt.save))
          os.execute(('openssl base64 -in %s/test.png -out %s/test.base64'):format(opt.save,opt.save))
          local f = io.open(opt.save..'/test.base64')
          if f then base64im = f:read'*all' end
        end
--]]
        local file = io.open(opt.save..'/report.html','w')
        file:write(([[
        <!DOCTYPE html>
        <html>
        <body>
        <title>%s - %s</title>
        <img src="data:image/png;base64,%s">
        <h4>optimState:</h4>
        <table>
        ]]):format(opt.save,epoch,base64im))
        for k,v in pairs(optimState) do
          if torch.type(v) == 'number' then
            file:write('<tr><td>'..k..'</td><td>'..v..'</td></tr>\n')
          end
        end
        file:write'</table><pre>\n'
        file:write(tostring(confusion)..'\n')
        file:write(tostring(model)..'\n')
        file:write'</pre></body></html>'
        file:close()
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
