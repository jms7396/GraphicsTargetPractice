#include "Target.h"



Target::Target()
{
}


Target::~Target()
{
	// Delete Entity pointers for each part of the Target and Chain
	if (mainTarget)
		delete mainTarget;

	for (int i = 0; i < targetChain.size(); ++i) 
	{
		if (targetChain[i])
			delete targetChain[i];
	}
}

Target::Target(DirectX::XMFLOAT3 pos, Mesh * mesh, Material * targetMat, Material * chainMat)
{
	position = pos;
	CreateTarget(mesh, targetMat, chainMat);
}

vector<Entity*> Target::GetTarget()
{
	// Not sure if this is the most efficient way to handle this, but it works so there's that
	vector<Entity*> tempTargets = targetChain;
	tempTargets.push_back(mainTarget);
	return vector<Entity*>(tempTargets);
}

void Target::Update()
{
}

void Target::CreateTarget(Mesh* mesh, Material* targetMat, Material* chainMat)
{
	mainTarget = new Entity(mesh, targetMat);
	mainTarget->SetPosition(position);
	mainTarget->SetScale(DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f));
	for (int i = 0; i < 3; i++)
	{
		targetChain.push_back(new Entity(mesh, chainMat));
		targetChain[i]->SetScale(DirectX::XMFLOAT3(0.1f, 0.1f, 0.1f));
		targetChain[i]->SetPosition(DirectX::XMFLOAT3(position.x, position.y+(0.25f + (0.5f*(i+1))), position.z));
	}
}
