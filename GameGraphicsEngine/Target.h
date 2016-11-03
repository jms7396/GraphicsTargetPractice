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
	void Update(float deltTime);
	bool GetActive();
	bool GetFall();

private:
	Entity* mainTarget;
	vector<Entity*> targetChain;
	DirectX::XMFLOAT3 position;
	void CreateTarget(Mesh*, Material*, Material*);
	int chainCount;
	float targetScale = 0.57f;
	float chainScale = 0.1f;
	bool active = true;
	bool drawTarget = true;
	bool fall = false;
	float targetVelocity = 0.0f;
	float targetAcceleration = -19.6f;
	float floor = -3.0f;
};

