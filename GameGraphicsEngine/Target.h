#pragma once
#include "Entity.h";
using namespace std;
class Target
{
public:
	Target();
	~Target();

	Target(DirectX::XMFLOAT3, Mesh*, Material*, Material*);

	vector<Entity*> GetTarget();
	void Update();

private:
	Entity* mainTarget;
	vector<Entity*> targetChain;
	DirectX::XMFLOAT3 position;
	void CreateTarget(Mesh*, Material*, Material*);
};

