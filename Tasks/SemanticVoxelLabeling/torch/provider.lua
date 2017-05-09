require 'torch'
require 'hdf5'


function trim(s)
    return (s:gsub("^%s*(.-)%s*$", "%1"))
end

function readClassesHist(file, num_classes)
    assert(paths.filep(file))
    counts = torch.zeros(num_classes)
    for line in io.lines(file) do
        --split line by whitespace
        parts = {}
        for p in line:gmatch("%w+") do table.insert(parts, p) end
        if #parts == 2 then
            counts[tonumber(parts[1])] = tonumber(parts[2])
        else
            break
        end
    end
    counts = counts / counts:sum() --normalize hist
    return counts
end

function readClassMapFile(file)
    assert(paths.filep(file))
    local class_map = {}
    local max_class = 0
    local skip = true --hack to skip first line
    for line in io.lines(file) do
        if not skip then
            --split line by whitespace
            local parts = {}
            for p in line:gmatch("%w+") do table.insert(parts, p) end
            if #parts >= 2 then
                local id = tonumber(parts[2])
                class_map[tonumber(parts[1])] = id
                if id > max_class then max_class = id end
                else break end
        else skip = false end --skip the first line
    end
    return class_map, max_class
end

-- read h5 filename list
function getDataFiles(input_file)
    local train_files = {}
    for line in io.lines(input_file) do
        train_files[#train_files+1] = trim(line)
    end
    return train_files
end

-- load h5 file data into memory
function loadDataFile(file_name, num_classes, class_map)
    assert(paths.filep(file_name))
    local current_file = hdf5.open(file_name,'r')
    local current_data = current_file:read('data'):all():float()
    local current_label = current_file:read('label'):all()
    current_file:close()

    if class_map ~= nil then
        current_label = current_label:int()
        current_label:mul(-1)
        for k,v in pairs(class_map) do
            current_label[current_label:eq(-k)] = v 
        end
        current_label[current_label:lt(0)] = 255
        current_label:byte()
    end

    current_label[current_label:eq(255)] = num_classes-1 --unlabeled is last class (num_classes-1 since adding the one after)

    current_label:add(1) --needs to be 1-indexed

    return current_data, current_label
end

-- input data sdf 2 channels: abs(sdf), known/unknown
function loadSdf2DataFile(file_name, truncation, num_classes, class_map)
    local current_data, current_label = loadDataFile(file_name, num_classes, class_map) 
    local sdf_data = torch.FloatTensor(current_data:size(1), 2, current_data:size(3), current_data:size(4), current_data:size(5))
    sdf_data[{{},1,{},{},{}}] = torch.abs(current_data)               --abs(sdf)
    sdf_data[{{},2,{},{},{}}] = torch.ge(current_data, -1):float():mul(2):add(-1) --make known/unknown 1/-1
    sdf_data[{{},1,{},{},{}}]:clamp(0, truncation)
    return sdf_data, current_label
end

-- input data sdf 2 channels: abs(sdf), known/unknown
function loadSdf2DataScene(file_name, truncation, num_classes, class_map)
    local current_data, current_label = loadDataFile(file_name, num_classes, class_map) 
    local sdf_data = torch.FloatTensor(2, current_data:size(1), current_data:size(2), current_data:size(3))
    sdf_data[{1,{},{},{}}] = torch.abs(current_data)               --abs(sdf)
    sdf_data[{2,{},{},{}}] = torch.ge(current_data, -1):float():mul(2):add(-1) --make known/unknown 1/-1
    sdf_data[{1,{},{},{}}]:clamp(0, truncation)
    return sdf_data, current_label
end

function serialize (o)
    if type(o) == "number" then
        io.write(o)
    elseif type(o) == "string" then
        io.write(string.format("%q", o))
    elseif type(o) == "boolean" then
        io.write(tostring(o))
    elseif type(o) == "table" then
        io.write("{\n")
        for k,v in pairs(o) do
            io.write("  ", k, " = ")
            serialize(v)
            io.write(",\n")
        end
        io.write("}\n")
    else
        error("cannot serialize a " .. type(o))
    end
end
