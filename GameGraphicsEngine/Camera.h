#pragma once
#include <DirectXMath.h>;
#include <Windows.h>;
#include <string>;
using namespace DirectX;
class Camera
{
public:
	Camera();
	~Camera();

	DirectX::XMFLOAT4X4 GetViewMat();
	DirectX::XMFLOAT4X4 GetProjectionMat();
	void RotateCamera(float changeInX, float changeInY);
	void Update(float deltaTime);
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

