#pragma once
#include <DirectXMath.h>
#include <Windows.h>
#include <string>
using namespace DirectX;
class Player
{
public:
	Player();
	~Player();

	void Update(float deltaTime);
	DirectX::XMFLOAT4X4 GetViewMat();
	DirectX::XMFLOAT4X4 GetProjectionMat();
	DirectX::XMFLOAT4 GetPosition();
	DirectX::XMFLOAT4 GetDirection();
	DirectX::XMFLOAT3 GetPosition3();
	DirectX::XMFLOAT3 GetDirection3();
	void RotatePlayer(float changeInX);
	void SetProjectionMat(float width, float height);

private:
	DirectX::XMFLOAT4X4 viewMat;
	DirectX::XMFLOAT4X4 projectionMat;
	/**
	Use XMFLOAT in class-level variables.  XMVECTOR and XMMATRIX should ONLY be used in local scope.
	position and direction being XMVECTORS at this level was causing sporadic errors in Release mode builds
	*/
	//DirectX::XMVECTOR position;
	//DirectX::XMVECTOR direction;
	DirectX::XMFLOAT4 position;
	DirectX::XMFLOAT4 direction;
	float rotX;
	float rotY;
};

