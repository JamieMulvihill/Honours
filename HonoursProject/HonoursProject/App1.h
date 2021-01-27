// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "TextureShader.h"
#include "DepthShader.h"
#include "ManipulationShader.h"
#include "ComputeShader.h"
#include "WriteComputeShader.h"
#include "DepthCamera.h"
#include "DeformationMap.h"
#include "GPUProfiler.h"
#include "VolumePreservationTestShader.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void depthPass();
	void ComputePass();
	void WriteComputePass();
	void finalPass();
	bool VolumePreservationTest();
	void gui();

private:
	PlaneMesh* mesh;
	OrthoMesh* orthoMesh;
	OrthoMesh* orthoMeshLeft;
	Light* light;
	Light* directionalLight;
	Model* model;

	DepthCamera* depthCamera;
	Camera* blackCamera;

	TextureShader* textureShader;
	DepthShader* depthShader;
	ManipulationShader* manipulationShader;
	ComputeShader* computeShader;
	WriteComputeShader* writeComputeShader;

	DeformationMap* deformationMap;
	ShadowMap* shadowMap;
	RenderTexture* renderTexture;

	GPUProfiler* gpuProfiler;
	VolumePreservationTestShader* vpTestShader;


	XMFLOAT4 volume;
	float lightX, lightY, lightZ, lightDirX, lightDirY, scale, newLightDirX, newLightDirY, newLightDirZ, teapotX, teapotY, teapotZ;
	float depthLightX, depthLightY, depthLightZ, depthDirX, depthDirY, depthDirZ;

	ID3D11ShaderResourceView* computeResult;
	ID3D11ShaderResourceView* computeResultTemp;

	bool hasStarted;
	bool isTesting;
};

#endif