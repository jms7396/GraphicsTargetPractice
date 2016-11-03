#include "Player.h"

Player::Player()
{
	// Load in position and direction to local XMVECTORs
	XMVECTOR posVec = XMLoadFloat3(&position);
	XMVECTOR dirVec = XMLoadFloat3(&direction);
	// Set initial position and direction
	posVec = XMVectorSet(0, 0, -5, 0);
	dirVec = XMVectorSet(0, 0, 1, 0);
	rotX = 0.0f;
	rotY = 0.0f;
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX V = XMMatrixLookToLH(
		posVec,     // The position of the camera
		dirVec,     // Direction the camera is looking
		up);
	// Store everything back in the corresponding member variables
	XMStoreFloat4x4(&viewMat, XMMatrixTranspose(V)); // Transpose for HLSL!
	XMStoreFloat3(&position, posVec);
	XMStoreFloat3(&direction, dirVec);
}


Player::~Player()
{
}

void Player::Update(float deltaTime)
{
	if (GetAsyncKeyState('W') & 0x8000) { MovePlayerForward(1.0f * deltaTime); }
	if (GetAsyncKeyState('S') & 0x8000) { MovePlayerForward(-1.0f * deltaTime); }
	if (GetAsyncKeyState('A') & 0x8000) { MovePlayerSideways(-1.0f * deltaTime); }
	if (GetAsyncKeyState('D') & 0x8000) { MovePlayerSideways(1.0f * deltaTime); }

	XMVECTOR rotation = XMQuaternionRotationRollPitchYaw(rotX, rotY, 0.0f);
	XMFLOAT3 forward = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMVECTOR forwardV = XMLoadFloat3(&forward);
	XMVECTOR directionV = XMVector3Rotate(forwardV, rotation);
	XMVECTOR positionV = XMLoadFloat3(&position);
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMVECTOR upV = XMLoadFloat3(&up);
	XMMATRIX view = XMMatrixLookToLH(positionV, directionV, upV);
	view = XMMatrixTranspose(view);
	XMStoreFloat4x4(&viewMat, view);
	XMStoreFloat3(&direction, directionV);
}

DirectX::XMFLOAT4X4 Player::GetViewMat()
{
	return viewMat;
}

DirectX::XMFLOAT4X4 Player::GetProjectionMat()
{
	return projectionMat;
}

DirectX::XMFLOAT3 Player::GetPosition()
{
	return position;
}

DirectX::XMFLOAT3 Player::GetDirection()
{
	return direction;
}

void Player::RotatePlayer(float changeInX, float changeInY)
{
	rotX += changeInY * 0.005f;
	rotY += changeInX * 0.005f;
}

void Player::SetProjectionMat(float width, float height)
{
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,		// Field of View Angle
		(float)width / height,		// Aspect ratio
		0.1f,						// Near clip plane distance
		100.0f);					// Far clip plane distance
	XMStoreFloat4x4(&projectionMat, XMMatrixTranspose(P)); // Transpose for HLSL!
}

void Player::MovePlayerSideways(float amount)
{
	XMVECTOR rotation = XMQuaternionRotationRollPitchYaw(rotX, rotY, 0.0f);
	XMFLOAT3 forward = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMVECTOR forwardV = XMLoadFloat3(&forward);
	XMVECTOR directionV = XMVector3Rotate(forwardV, rotation);
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMVECTOR upV = XMLoadFloat3(&up);
	XMVECTOR side = XMVector3Cross(directionV, upV);
	XMVECTOR movement = side * -amount;
	XMVECTOR positionV = XMLoadFloat3(&position);
	XMVECTOR newPos = XMVectorAdd(positionV, movement);
	XMStoreFloat3(&position, newPos);
}

void Player::MovePlayerForward(float amount)
{
	XMVECTOR rotation = XMQuaternionRotationRollPitchYaw(rotX, rotY, 0.0f);
	XMFLOAT3 forward = XMFLOAT3(0.0f, 0.0f, amount);
	XMVECTOR forwardV = XMLoadFloat3(&forward);
	XMVECTOR directionV = XMVector3Rotate(forwardV, rotation);
	XMVECTOR positionV = XMLoadFloat3(&position);
	XMVECTOR newPos = XMVectorAdd(positionV, directionV);
	XMStoreFloat3(&position, newPos);
}