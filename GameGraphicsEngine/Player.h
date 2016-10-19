#pragma once
#include <DirectXMath.h>;
#include <Windows.h>;
#include <string>;
using namespace DirectX;
class Player
{
public:
	Player();
	~Player();

	void Update(float deltaTime);
	DirectX::XMFLOAT4X4 GetViewMat();
	DirectX::XMFLOAT4X4 GetProjectionMat();
	void RotatePlayer(float changeInX);
	void SetProjectionMat(float width, float height);

private:
	DirectX::XMFLOAT4X4 viewMat;
	DirectX::XMFLOAT4X4 projectionMat;
	DirectX::XMVECTOR position;
	DirectX::XMVECTOR direction;
	float rotX;
	float rotY;
};

