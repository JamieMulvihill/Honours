#include "DepthCamera.h"


DepthCamera::DepthCamera(Input* in, int width, int height, HWND hnd)
{
	input = in;
	winWidth = width;
	winHeight = height;
	wnd = hnd;

	pos = XMFLOAT3(0.f, 0.f, 0.f);
	rot = XMFLOAT3(0.f, 0.f, 0.f);

	XMVECTOR up, positionVec, lookAt;
	up = XMVectorSet(0.0f, 1.0, 0.0, 1.0f);
	positionVec = XMVectorSet(0.0f, 0.0, -10.0, 1.0f);
	lookAt = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0);
	orthoMatrix = XMMatrixLookAtLH(positionVec, lookAt, up);
}
// Create a projection matrix for the for camera to be used in deformation map.
void DepthCamera::generateProjectionMatrix(float screenNear, float screenFar)
{
	float fieldOfView, screenAspect;

	// Setup field of view and screen aspect for a square light source.
	fieldOfView = (float)XM_PI / 2.0f;
	screenAspect = 1.0f;

	// Create the projection matrix.
	projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenFar);
}

// Create orthomatrix for camera to be used in deformation map.
void DepthCamera::generateOrthoMatrix(float screenWidth, float screenHeight, float nearplane, float farplane)
{
	orthoMatrix = XMMatrixOrthographicLH(screenWidth, screenHeight, nearplane, farplane);
}

void DepthCamera::generateViewMatrix()
{
	XMVECTOR up, positionv, lookAt;
	float yaw, pitch, roll;
	XMMATRIX rotationMatrix;

	// Setup the vectors
	up = XMVectorSet(0.0f, 1.0, 0.0, 1.0f);
	positionv = XMLoadFloat3(&pos);
	lookAt = XMVectorSet(0.0, 0.0, 1.0f, 1.0f);

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	pitch = rot.x * 0.0174532f;
	yaw = rot.y * 0.0174532f;
	roll = rot.z * 0.0174532f;

	// Create the rotation matrix from the yaw, pitch, and roll values.
	rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	// Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
	lookAt = XMVector3TransformCoord(lookAt, rotationMatrix);
	up = XMVector3TransformCoord(up, rotationMatrix);

	// Translate the rotated camera position to the location of the viewer.
	lookAt = positionv + lookAt;

	// Finally create the view matrix from the three updated vectors.
	viewMatrix = XMMatrixLookAtLH(positionv, lookAt, up);
}

XMMATRIX DepthCamera::getViewMatrix()
{
	return viewMatrix;
}

XMMATRIX DepthCamera::getProjectionMatrix()
{
	return projectionMatrix;
}

XMMATRIX DepthCamera::getOrthoMatrix()
{
	return orthoMatrix;
}
void DepthCamera::setPos(float lx, float ly, float lz)
{
	pos.x = lx;
	pos.y = ly;
	pos.z = lz;
}

void DepthCamera::setRot(float lx, float ly, float lz)
{
	rot.x = lx;
	rot.y = ly;
	rot.z = lz;
}

XMFLOAT3 DepthCamera::getPos()
{
	return pos;
}

XMFLOAT3 DepthCamera::getRot()
{
	return rot;
}