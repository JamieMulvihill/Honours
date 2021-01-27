#pragma once

#include "camera.h"
#include "Input.h"

using namespace DirectX;

class DepthCamera: public Camera
{
public:
	DepthCamera(Input* in, int width, int height, HWND hnd);	///< Initialised default camera object

	void generateProjectionMatrix(float screenNear, float screenFar);			///< Generate project matrix based on current rotation and provided near & far plane
	void generateOrthoMatrix(float screenWidth, float screenHeight, float near, float far);		///< Generates orthographic matrix based on supplied screen dimensions and near & far plane.
	void generateViewMatrix();
	XMMATRIX getViewMatrix();
	XMMATRIX getProjectionMatrix();
	XMMATRIX getOrthoMatrix();

	void setPos(float lx, float ly, float lz);

	void setRot(float lx, float ly, float lz);

	XMFLOAT3 getPos();

	XMFLOAT3 getRot();

private:
	XMFLOAT3 pos;		///< float3 for position
	XMFLOAT3 rot;		///< float3 for rotation (angles)
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
	XMMATRIX orthoMatrix;
	XMVECTOR lookAt;

	Input* input;
	int winWidth, winHeight;///< stores window width and height
	int deltax, deltay;		///< for mouse movement
	POINT cursor;			///< Used for converting mouse coordinates for client to screen space
	HWND wnd;				///< handle to the window
};

