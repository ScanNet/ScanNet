#pragma once 

#include "../common/json.h"
#include "../common/Segmentation.h"
#include "../common/Aggregation.h"

struct ConstantBuffer
{
	mat4f worldViewProj;
	ml::vec4f modelColor;
};

class Visualizer : public ApplicationCallback
{
public:
	void init(ApplicationData &app);
	void render(ApplicationData &app);
	void keyDown(ApplicationData &app, UINT key);

	void keyPressed(ApplicationData &app, UINT key);
	void mouseDown(ApplicationData &app, MouseButtonType button);
	void mouseMove(ApplicationData &app);
	void mouseWheel(ApplicationData &app, int wheelDelta);
	void resize(ApplicationData &app);

private:
	//returns #unique objects/colors
	unsigned int computeObjectIdsAndColorsPerVertex(const Aggregation& aggregation, const Segmentation& segmentation, MeshDataf& meshData, const MeshDataf& meshHi);

	static inline float gaussD(float sigma, int x, int y)
	{
		return exp(-((x*x + y*y) / (2.0f*sigma*sigma)));
	}
	static inline float gaussR(float sigma, float dist)
	{
		return exp(-(dist*dist) / (2.0f*sigma*sigma));
	}

	void propagateAnnotations(const MeshDataf& meshSrc, MeshDataf& meshDst);

	ml::D3D11TriMesh m_mesh;

	ml::D3D11ShaderManager m_shaderManager;
	D3D11RenderTarget m_renderTarget;

	FrameTimer m_timer;

	D3D11ConstantBuffer<ConstantBuffer> m_constants;
	Cameraf m_camera;

	//scan data
	SensorData m_sensorData;
	std::string m_sensorName;
	float m_fieldOfView;
};