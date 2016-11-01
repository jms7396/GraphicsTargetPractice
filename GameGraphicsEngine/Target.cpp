#include "Target.h"

Target::Target()
{
}


Target::~Target()
{
	// Delete Entity pointers for each part of the Target and Chain
	if (mainTarget)
		delete mainTarget;

	for (unsigned int i = 0; i < targetChain.size(); ++i) 
	{
		if (targetChain[i])
			delete targetChain[i];
	}
}

Target::Target(DirectX::XMFLOAT3 pos, Mesh * mesh, Material * targetMat, Material * chainMat)
{
	chainCount = rand() %  4+ 2;
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
	targetChain.push_back(new Entity(mesh, chainMat));
	targetChain[0]->SetScale(DirectX::XMFLOAT3(chainScale, chainScale, chainScale));
	targetChain[0]->SetPosition(position);
	
	for (int i = 1; i < chainCount; i++)
	{
		targetChain.push_back(new Entity(mesh, chainMat));
		targetChain[i]->SetScale(DirectX::XMFLOAT3(chainScale, chainScale, chainScale));
		targetChain[i]->SetPosition(DirectX::XMFLOAT3(position.x, position.y-( i * chainScale), position.z));
	}

	mainTarget->SetPosition(DirectX::XMFLOAT3(position.x, position.y - ((chainCount - 0.5f) * chainScale) - targetScale * 0.5f, position.z));
	mainTarget->SetScale(DirectX::XMFLOAT3(targetScale, targetScale, targetScale));
}

bool Target::CheckShot(DirectX::XMFLOAT3 playerDirection, DirectX::XMFLOAT3 playerPos)
{
	bool collision = false;
	for (int i = 1; i < chainCount; i++)
	{
		//Get a normalized player direction
		DirectX::XMVECTOR direction = XMLoadFloat3(&playerDirection);
		direction = DirectX::XMVector3Normalize(direction);

		//Get a vector from player to sphere center
		DirectX::XMVECTOR sphereLoc = XMLoadFloat3(&targetChain[i]->GetPosition());
		DirectX::XMVECTOR playerLoc = XMLoadFloat3(&playerPos);
		DirectX::XMVECTOR aimDirection = DirectX::XMVectorSubtract(sphereLoc,playerLoc);

		//Project the new vector onto the player direction and get the length of the new projected vector
		DirectX::XMVECTOR projection = DirectX::XMVector3Dot(aimDirection, direction);
		DirectX::XMFLOAT3 length;
		DirectX::XMStoreFloat3(&length, projection);
		
		//Find the point in the player's direction the length away
		direction = direction * length.x;
		DirectX::XMVECTOR pointOnLine = playerLoc + direction;
		DirectX::XMVECTOR distance = pointOnLine - sphereLoc;
		DirectX::XMVector3Length(distance);
		DirectX::XMFLOAT3 dist;
		DirectX::XMStoreFloat3(&dist, distance);
		
		//Check to see if it is shorter than the radius of the sphere
		if (abs(dist.x) < chainScale)
		{
			//add a  or drop method
			printf("%f", i);
			return true;
		}
	}


		// find the distance between the closest point to the ray from the center of the sphere and check if shorter than the radius

		return collision;

	//Vector3 direction = grapple.transform.position - transform.position;
	//direction.y = 0f;
	//direction.Normalize();
	//Vector3 movingVec = transform.position - prevPos;
	//movingVec.y = 0f;
	//float movement = Vector3.Dot(movingVec, direction);
}
