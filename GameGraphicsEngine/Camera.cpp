#include "Camera.h"

Camera::Camera()
{
	// Load in position and direction to local XMVECTORs
	XMVECTOR posVec = XMLoadFloat4(&position);
	XMVECTOR dirVec = XMLoadFloat4(&direction);
	posVec = XMVectorSet(0, 0, -5, 0);
	dirVec = XMVectorSet(0, 0, 1, 0);
	rotX = 0.0f;
	rotY = 0.0f;
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX V = XMMatrixLookToLH(
		posVec,     // The position of the camera
		dirVec,     // Direction the camera is looking
		up);
	// Store everything back into the corresponding member variables
	XMStoreFloat4x4(&viewMat, XMMatrixTranspose(V)); // Transpose for HLSL!
	XMStoreFloat4(&position, posVec);
	XMStoreFloat4(&direction, dirVec);
}


Camera::~Camera()
{
}

DirectX::XMFLOAT4X4 Camera::GetViewMat()
{
	return viewMat;
}

DirectX::XMFLOAT4X4 Camera::GetProjectionMat()
{
	return projectionMat;
}

void Camera::RotateCamera(float changeInX, float changeInY)
{
	rotX += changeInX*0.001;
	rotY += changeInY*0.001;
}

void Camera::Update(float deltaTime)
{
	// Load in position and direction to local XMVECTORs
	DirectX::XMVECTOR posVec = XMLoadFloat4(&position);
	DirectX::XMVECTOR dirVec = XMLoadFloat4(&direction);
	DirectX::XMVECTOR upVector = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR rotation = DirectX::XMQuaternionRotationRollPitchYaw(rotY, rotX, 0.0f);
	dirVec = DirectX::XMVector3Rotate(dirVec, rotation);

	DirectX::XMVECTOR moveVector = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	if (GetAsyncKeyState('W') & 0x8000) { moveVector = DirectX::XMVectorAdd(moveVector, dirVec); }
	if (GetAsyncKeyState('S') & 0x8000) { moveVector = DirectX::XMVectorAdd(moveVector, (-1.0f * dirVec)); };
	if (GetAsyncKeyState('A') & 0x8000) { moveVector = DirectX::XMVectorAdd(moveVector, DirectX::XMVector3Cross(dirVec, upVector)); };
	if (GetAsyncKeyState('D') & 0x8000) { moveVector = DirectX::XMVectorAdd(moveVector, -1.0 * DirectX::XMVector3Cross(dirVec, upVector)); };
	if (GetAsyncKeyState(VK_SPACE) & 0x8000) { moveVector = DirectX::XMVectorAdd(moveVector, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)); };
	if (GetAsyncKeyState('X') & 0x8000) { moveVector = DirectX::XMVectorAdd(moveVector, DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f)); };

	posVec = XMVectorAdd(moveVector * deltaTime, posVec);

	// Store everything back into the corresponding member variables
	XMStoreFloat4x4(&viewMat, XMMatrixTranspose(DirectX::XMMatrixLookToLH(posVec, dirVec, upVector)));
	XMStoreFloat4(&position, posVec);
	XMStoreFloat4(&direction, dirVec);
	rotX = 0.0f;
	rotY = 0.0f;
}

void Camera::SetProjectionMat(float width, float height)
{
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,		// Field of View Angle
		(float)width / height,		// Aspect ratio
		0.1f,						// Near clip plane distance
		100.0f);					// Far clip plane distance
	XMStoreFloat4x4(&projectionMat, XMMatrixTranspose(P)); // Transpose for HLSL!
}
