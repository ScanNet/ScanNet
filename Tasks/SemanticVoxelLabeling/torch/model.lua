
-- from net-orig, add more to fc and make another deconv layer

require 'nn'
require 'cunn'
require 'cudnn'

local net = nn.Sequential()

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

-- create net
if (opt.retrain == '' or opt.retrain == nil) then

    local nf0 = 32
    net:add(cudnn.VolumetricConvolution(2, nf0, 4, 3, 3, 2, 2, 2))   -- output nf0 x 30x15x15
    net:add(cudnn.VolumetricBatchNormalization(nf0))
    net:add(cudnn.ReLU())
    net:add(cudnn.VolumetricConvolution(nf0, nf0, 1, 1, 1))
    net:add(cudnn.VolumetricBatchNormalization(nf0))
    net:add(cudnn.ReLU())
    net:add(cudnn.VolumetricConvolution(nf0, nf0, 1, 1, 1))
    net:add(cudnn.VolumetricBatchNormalization(nf0))
    net:add(cudnn.ReLU())
    net:add(nn.VolumetricDropout(0.2))

    local nf1 = 64
    net:add(cudnn.VolumetricConvolution(nf0, nf1, 4, 3, 3, 2, 2, 2))  -- output nf1 x 14x7x7
    net:add(cudnn.VolumetricBatchNormalization(nf1))
    net:add(cudnn.ReLU())
    net:add(cudnn.VolumetricConvolution(nf1, nf1, 1, 1, 1))
    net:add(cudnn.VolumetricBatchNormalization(nf1))
    net:add(cudnn.ReLU())
    net:add(cudnn.VolumetricConvolution(nf1, nf1, 1, 1, 1))
    net:add(cudnn.VolumetricBatchNormalization(nf1))
    net:add(cudnn.ReLU())
    net:add(nn.VolumetricDropout(0.2))

    local nf2 = 128
    net:add(cudnn.VolumetricConvolution(nf1, nf2, 4, 3, 3, 2, 2, 2))  -- output nf x 6x3x3
    net:add(cudnn.VolumetricBatchNormalization(nf2))
    net:add(cudnn.ReLU())
    net:add(cudnn.VolumetricConvolution(nf2, nf2, 1, 1, 1))
    net:add(cudnn.VolumetricBatchNormalization(nf2))
    net:add(cudnn.ReLU())
    net:add(cudnn.VolumetricConvolution(nf2, nf2, 1, 1, 1))
    net:add(cudnn.VolumetricBatchNormalization(nf2))
    net:add(cudnn.ReLU())
    net:add(nn.VolumetricDropout(0.2))

	local bf = 1024
    net:add(nn.View(nf2 * 54))
    net:add(nn.Linear(nf2 * 54, bf))
    net:add(cudnn.ReLU())
    net:add(nn.Dropout(0.5))

    net:add(nn.Linear(bf, num_classes*62))
    net:add(nn.View(num_classes, 1, 62)) 

    MSRinit(net)

else --preload network
    assert(paths.filep(opt.retrain), 'File not found: ' .. opt.retrain)
    print('loading previously trained network: ' .. opt.retrain)
    net = torch.load(opt.retrain)
end
cudnn.convert(net, cudnn)
print('net:')

return net

