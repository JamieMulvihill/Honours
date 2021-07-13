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
#include "DisplacementShader.h"
#include "DisplacementMethodBShader.h"
#include "PenetrationShader.h"
#include "JumpFloodingShader.h"
#include "PongJumpFloodShader.h"
#include "JumpFloodInitializer.h"
#include "VolumePreservationShader.h"
#include "NormalMapShader.h"
#include "VolumePreservationPongShader.h"

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
	void PenetrationPass();
	void JumpFloodingPass();
	void DisplacementPass();
	void VolumePreservationPass();
	void GenerateNormalsPass();
	void finalPass();
	bool VolumePreservationTest();

	void FootprintDemo();
	void FootprintMovement(bool &direction, float &height, XMFLOAT4 &vector);
	void gui();

private:
	PlaneMesh* mesh;
	OrthoMesh* orthoMeshLeft;
	Light* light;
	Light* directionalLight;
	Model* model;
	Model* shoeA;
	Model* shoeB;

	TextureShader* textureShader;
	DepthShader* depthShader;
	ManipulationShader* manipulationShader;
	ComputeShader* computeShader;
	WriteComputeShader* writeComputeShader;
	PenetrationShader* penetrationShader;
	DisplacementShader* displacementShader;
	DisplacementMethodBShader* displacementMethodBShader;
	JumpFloodingShader* jumpFloodingShader;
	PongJumpFloodShader* pongJumpFLoodShader;
	JumpFloodInitializer* jumpFloodInitShader;
	VolumePreservationShader* volumePreservationShader;
	VolumePreservationPongShader* volumePreservationPongShader;
	NormalMapShader* normalMapShader;

	DepthCamera* depthCamera;
	DeformationMap* deformationMap;

	GPUProfiler* gpuProfiler;
	VolumePreservationTestShader* vpTestShader;

	XMFLOAT4 footprintPositionA, footprintPositionB;
	XMFLOAT4 volume;

	int threadCountHeight, threadCountWidth;
	float lightX, lightY, lightZ, lightDirX, lightDirY, scale, newLightDirX, newLightDirY, newLightDirZ, teapotX, teapotY, teapotZ;
	float depthLightX, depthLightY, depthLightZ, depthDirX, depthDirY, depthDirZ;
	float heightTreshold;

	float heightA, heightB;
	bool downA = false;
	bool downB = false;
	bool hasReset, volumePong, inIterativeDisplacement, inDemo;
};

#endif