require 'nn'
require 'cunn'
require 'cudnn'

local net = nn.Sequential()
    
-- conv + batchnorm + relu
local function Block(...)
    local arg = {...}
    net:add(cudnn.VolumetricConvolution(...))
    net:add(cudnn.VolumetricBatchNormalization(arg[2]))
    net:add(cudnn.ReLU(true))
    return net
end

Block(1,48,6,6,6,2,2,2)
Block(48,48,1,1,1)
Block(48,48,1,1,1)
net:add(nn.Dropout(0.2))

Block(48,96,5,5,5,2,2,2)
Block(96,96,1,1,1)
Block(96,96,1,1,1)
net:add(nn.Dropout(0.2))

Block(96,512,3,3,3,2,2,2)
Block(512,512,1,1,1)
Block(512,512,1,1,1)
net:add(nn.Dropout(0.2))

net:add(nn.View(4096))
net:add(nn.Linear(4096,512))
net:add(cudnn.ReLU(true))
net:add(nn.Dropout(0.5))

--net:add(nn.Linear(512,40))
net:add(nn.Linear(512,55))

local function MSRinit(net)
    local function init(name)
        for k,v in pairs(net:findModules(name)) do
            local n = v.kT*v.kW*v.kH*v.nOutputPlane
            v.weight:normal(0,math.sqrt(2/n))
            v.bias:zero()
        end
    end
    init'VolumetricConvolution'
    return net
end

MSRinit(net)

return net
