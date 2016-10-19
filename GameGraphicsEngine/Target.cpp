#include "Target.h"



Target::Target()
{
}


Target::~Target()
{
}

Target::Target(DirectX::XMFLOAT3 pos, Mesh * mesh, Material * targetMat, Material * chainMat)
{
	position = pos;
	CreateTarget(mesh, targetMat, chainMat);
}

vector<Entity*> Target::GetTarget()
{
	targetChain.push_back(mainTarget);
	return vector<Entity*>(targetChain);
	targetChain.pop_back();
}

void Target::Update()
{
}

void Target::CreateTarget(Mesh* mesh, Material* targetMat, Material* chainMat)
{
	mainTarget = new Entity(mesh, targetMat);
	mainTarget->SetPosition(position);
	for (int i = 0; i < 3; i++)
	{
		targetChain.push_back(new Entity(mesh, chainMat));
		targetChain[i]->SetScale(DirectX::XMFLOAT3(0.1, 0.1, 0.1));
		targetChain[i]->SetPosition(DirectX::XMFLOAT3(position.x+0.5f, position.y+(0.5f*(i+1)), position.z));
	}
}
