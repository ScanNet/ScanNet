
#pragma once

#include "mLibInclude.h"

class Aligner {
public:
	Aligner(GraphicsDevice& g) {
		m_graphics = &g;

		m_shaderManager.init(*m_graphics);
		std::string shaderPath = util::getExecutablePath() + "../../";
		const std::string shaderFile = "shaders/aligner.hlsl";
		if (!util::fileExists(shaderPath + shaderFile)) shaderPath = util::getExecutablePath();
		m_shaderManager.registerShaderWithGS(shaderPath + shaderFile, "aligner");

		m_constantBuffer.init(*m_graphics);
	}

	~Aligner() {

	}

	DepthImage32 depthToColor(const DepthImage32& input, const mat4f& depthIntrinsic, const mat4f& depthExtrinsic, const mat4f& colorIntrinsic, unsigned int colorWidth, unsigned int colorHeight) {
		
		CB cb;
		cb.intrinsicInverse = depthIntrinsic.getInverse();
		cb.extrinsic = depthExtrinsic;
		cb.intrinsicNew = colorIntrinsic;
		cb.width = input.getWidth();
		cb.height = input.getHeight();
		cb.widthNew = colorWidth;
		cb.heightNew = colorHeight;
		cb.depthThreshLin = 0.05f;		// additional discontinuity threshold per meter
		cb.depthThreshOffset = 0.01f;  // discontinuity offset in meter
		m_constantBuffer.update(cb);

		D3D11Buffer<float> depthGPU;
		std::vector<float> inputVec(input.getWidth()*input.getHeight());
		for (size_t i = 0; i < inputVec.size(); i++) inputVec[i] = input.getData()[i];
		depthGPU.init(*m_graphics, inputVec);

		D3D11RenderTarget renderTarget;
		renderTarget.init(m_graphics->castD3D11(), input.getWidth(), input.getHeight(), std::vector < DXGI_FORMAT > {DXGI_FORMAT_R8G8B8A8_UNORM}, true);
		renderTarget.clear();
		renderTarget.bind(); 

		ID3D11DeviceContext& context = m_graphics->castD3D11().getContext();

		context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		context.IASetInputLayout(NULL);
		unsigned int stride = 0;
		unsigned int offset = 0;
		context.IASetVertexBuffers(0, 0, NULL, &stride, &offset);

		m_shaderManager.bindShaders("aligner");
		depthGPU.bindSRV(0);
		m_constantBuffer.bind(0);

		m_graphics->castD3D11().setCullMode(D3D11_CULL_NONE);
		unsigned int numQuads = input.getWidth()*input.getHeight();
		context.Draw(numQuads, 0);
		

		//! reset the state
		m_shaderManager.unbindShaders();
		depthGPU.unbindSRV(0);

		renderTarget.unbind();
		
		DepthImage32 res;
		res.setInvalidValue(input.getInvalidValue());
		renderTarget.captureDepthBuffer(res);
		for (auto& r : res) {
			if (r.value == 1.0f) r.value = res.getInvalidValue();
			else  r.value = kinectProjZToCamera(r.value);
		}

		context.Flush();

		return res;		
	}

	//projects a depth image into color space (debug version with aliasing)
	static DepthImage32 depthToColorDebug(const DepthImage32& input, const mat4f& depthIntrinsic, const mat4f& depthExtrinsic, const mat4f& colorIntrinsic, unsigned int colorWidth, unsigned int colorHeight) {

		mat4f depthIntrinsicInv = depthIntrinsic.getInverse();

		float scalarWidth = (float)(input.getWidth()-1) / (float)(colorWidth-1);
		float scalarHeight = (float)(input.getHeight()-1) / (float)(colorHeight-1);

		DepthImage32 res(input.getWidth(), input.getHeight());
		res.setInvalidValue(input.getInvalidValue());
		res.setPixels(input.getInvalidValue());

		for (auto& o : input) {
			float d = o.value;
			if (d != res.getInvalidValue()) {
				vec3f p = depthIntrinsicInv*vec3f((float)o.x*d, (float)o.y*d, d);	//back-project to camera space
				p = depthExtrinsic * p;												//project to color frame
				vec3f colorCoord = colorIntrinsic * p;								//project to color image space
				colorCoord.x /= colorCoord.z;	colorCoord.y /= colorCoord.z;
				vec2i colorCoordi = math::round(colorCoord);	//use that to get color values
				colorCoordi.x = math::round(colorCoord.x * scalarWidth);
				colorCoordi.y = math::round(colorCoord.y * scalarHeight);

				if (colorCoordi.x >= 0 && colorCoordi.x < (int)res.getWidth() && colorCoordi.y >= 0 && colorCoordi.y < (int)res.getHeight()) {
					res(colorCoordi.x, colorCoordi.y) = colorCoord.z;
				}
			}
		}
		return res;
	}
private:
#define DEPTH_WORLD_MIN 0.1f
#define DEPTH_WORLD_MAX 10.0f

	float cameraToKinectProjZ(float z)
	{
		return (z - DEPTH_WORLD_MIN) / (DEPTH_WORLD_MAX - DEPTH_WORLD_MIN);
	}

	float kinectProjZToCamera(float z)
	{
		return DEPTH_WORLD_MIN + z*(DEPTH_WORLD_MAX - DEPTH_WORLD_MIN);
	}

	GraphicsDevice* m_graphics;
	D3D11ShaderManager m_shaderManager;

	struct CB {
		mat4f intrinsicInverse;
		mat4f intrinsicNew;
		mat4f extrinsic;
		unsigned int width;
		unsigned int height;
		unsigned int widthNew;
		unsigned int heightNew;
		float depthThreshOffset;
		float depthThreshLin;
		vec2f dummy;
	};
	D3D11ConstantBuffer<CB> m_constantBuffer;

};