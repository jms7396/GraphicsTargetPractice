#include "Player.h"



Player::Player()
{
	position = XMVectorSet(0, 0, -5, 0);
	direction = XMVectorSet(0, 0, 1, 0);
	rotX = 0.0f;
	rotY = 0.0f;
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX V = XMMatrixLookToLH(
		position,     // The position of the camera
		direction,     // Direction the camera is looking
		up);
	XMStoreFloat4x4(&viewMat, XMMatrixTranspose(V)); // Transpose for HLSL!
}


Player::~Player()
{
}

void Player::Update(float deltaTime)
{
	DirectX::XMVECTOR upVector = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	DirectX::XMVECTOR moveVector = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	if (GetAsyncKeyState('W') & 0x8000) { moveVector = DirectX::XMVectorAdd(moveVector, direction); }
	if (GetAsyncKeyState('S') & 0x8000) { moveVector = DirectX::XMVectorAdd(moveVector, (-1.0f * direction)); };
	if (GetAsyncKeyState('A') & 0x8000) { RotatePlayer(-0.1); };
	if (GetAsyncKeyState('D') & 0x8000) { RotatePlayer(0.1); };

	DirectX::XMVECTOR rotation = DirectX::XMQuaternionRotationRollPitchYaw(rotY, rotX, 0.0f);
	direction = DirectX::XMVector3Rotate(direction, rotation);

	position = XMVectorAdd(moveVector * deltaTime, position);

	XMStoreFloat4x4(&viewMat, XMMatrixTranspose(DirectX::XMMatrixLookToLH(position, direction, upVector)));
	rotX = 0.0f;
	rotY = 0.0f;
}

DirectX::XMFLOAT4X4 Player::GetViewMat()
{
	return viewMat;
}

DirectX::XMFLOAT4X4 Player::GetProjectionMat()
{
	return projectionMat;
}

void Player::RotatePlayer(float changeInX)
{
	rotX += changeInX*0.001;
}

void Player::SetProjectionMat(float width, float height)
{

}
