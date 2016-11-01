#pragma once
#include "Entity.h"
using namespace std;
using namespace DirectX;
class Target
{
public:
	Target();
	~Target();

	Target(DirectX::XMFLOAT3, Mesh*, Material*, Material*);
	bool CheckShot(DirectX::XMFLOAT3 playerDirection, DirectX::XMFLOAT3 playerPos);
	vector<Entity*> GetTarget();
	void Update();

private:
	Entity* mainTarget;
	vector<Entity*> targetChain;
	DirectX::XMFLOAT3 position;
	void CreateTarget(Mesh*, Material*, Material*);
	int chainCount;
	float targetScale = 1.0f;
	float chainScale = 0.1f;
};

