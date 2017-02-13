require 'nn'
require 'cunn'
require 'cudnn'
require 'optim'
require 'xlua'
require 'torch'
require 'hdf5'

opt_string = [[
    --model         (default "logs/model.net")              torch model file path
    --h5_list_path       (default "data/volume_data0.h5")        h5 data path
    --gpu_index     (default 0)                             GPU index
    --output_file   (default "feat.txt")                    Ouput filename
    --output_name_file (default "names.txt")
    --file_label_file (default "train_file_label.txt")
    --partial_data                                         use partial data as input
    --classes_file  (default "")                                            limit classes
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

nchannels = 1
if opt.partial_data then nchannels = 2 end
print('#channels = ' .. nchannels)

function getClassesSet(file)
    assert(paths.filep(file))
    classes = {}
    for line in io.lines(file) do
        --split line by whitespace
        parts = {}
        for p in line:gmatch("%w+") do table.insert(parts, p) end
        classes[tonumber(parts[1])+1] = true
    end
    return classes
end
-- find set of classes
class_set = getClassesSet(opt.classes_file)



-- set gpu
cutorch.setDevice(opt.gpu_index+1)

-- output file
outfile = assert(io.open(opt.output_file, "w"))
outnamefile = assert(io.open(opt.output_name_file, "w"))

-- specify which layer's output we would use as feature 
OUTPUT_LAYER_INDEX = 33

print('Loading model...')
model = torch.load(opt.model):cuda()
model:evaluate()
print(model)

function getLinesFromFile(file)
    assert(paths.filep(file))
    lines = {}
    for line in io.lines(file) do
    lines[#lines + 1] = line
    end
    return lines
end
-- load h5 file data into memory
function loadDataFile(file_name)
    local current_file = hdf5.open(file_name,'r')
    local current_data = current_file:read('data'):all():float()
    local current_label = torch.squeeze(current_file:read('label'):all():add(1))
    current_file:close()
    return current_data, current_label
end

constant_ones = torch.ones(1,1,30,30,30):float()

filenames = getLinesFromFile(opt.h5_list_path)
print('#filenames = ' .. #filenames)
instancenames = getLinesFromFile(opt.file_label_file)
print('first instance:')
print(instancenames[1])

fidx = 1
local count = 0
local total = 0
for fn = 1,#filenames do
    print('Loading data...')
    local current_data, current_label = loadDataFile(filenames[fn])
    print(#current_data)

    print('Starting testing...')
    for t = 1,current_data:size(1) do
        local inputs = current_data[t][{{1,nchannels},{},{},{}}]:reshape(1,nchannels,30,30,30)
        --print(inputs:size())
        --print(inputs:sum())
        if not opt.partial_data then inputs = torch.cat(inputs, constant_ones, 2) end -- all voxels are known
        --print(inputs:size())
        --print(inputs:sum())
        local target = current_label[t]

        if class_set[target] then
            total = total + 1
            local outputs = model:forward(inputs:cuda())
            val, idx = torch.max(outputs:double(), 1)
            if idx[1] == target then count = count + 1 end
            --print('pred ' .. idx[1] .. ', target ' .. target)
            --print(instancenames[fidx])
            --io.read()
            --print(outputs)
            feat = model:get(OUTPUT_LAYER_INDEX).output:double()
            splitter = ','
            for i=1,feat:size(1) do
                outfile:write(string.format("%.6f", feat[i]))
                if i < feat:size(1) then
                    outfile:write(splitter)
                end
            end
            outfile:write('\n')
            outnamefile:write(string.format("%s\n", instancenames[fidx]))
        --else
        --    print('warning: ignoring target ' .. target .. ' not in class set')
        --    io.read()
        end
        fidx = fidx + 1
    end
    print('\tcur acc = ' .. count/total .. '\t(' .. count .. '/' .. total .. ')')
end
