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
	chainCount = rand() % 4 + 2;
	position = pos;
	CreateTarget(mesh, targetMat, chainMat);
}

vector<Entity*> Target::GetTarget()
{
	// Not sure if this is the most efficient way to handle this, but it works so there's that
	vector<Entity*> tempTargets = targetChain;
	if (drawTarget)
		tempTargets.push_back(mainTarget);
	return vector<Entity*>(tempTargets);
}

void Target::Update(float deltaTime)
{
	if (drawTarget && !active && fall)
	{
		targetVelocity += targetAcceleration * deltaTime;
		mainTarget->Move(DirectX::XMFLOAT3(0.0f, targetVelocity * deltaTime, 0.0f));
		if (mainTarget->GetPosition().y <= ((targetScale * 0.5f) + floor))
		{
			targetVelocity =  -0.85f * targetVelocity;
			if (abs(targetVelocity) < 0.005f)
			{
				fall = false;
			}
		}
	}
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
		targetChain[i]->SetPosition(DirectX::XMFLOAT3(position.x, position.y - (i * chainScale), position.z));
	}

	mainTarget->SetPosition(DirectX::XMFLOAT3(position.x, position.y - ((chainCount - 0.5f) * chainScale) - targetScale * 0.5f, position.z));
	mainTarget->SetScale(DirectX::XMFLOAT3(targetScale, targetScale, targetScale));
}

bool Target::CheckShot(DirectX::XMFLOAT3 playerDirection, DirectX::XMFLOAT3 playerPos)
{
	bool collision = false;
	if (!active)
		return collision; // don't check for collision if its already been hit

	for (int i = 0; i < chainCount; i++)
	{
		//Get a normalized player direction
		DirectX::XMVECTOR direction = XMLoadFloat3(&playerDirection);
		direction = DirectX::XMVector3Normalize(direction);

		//Get a vector from player to sphere center
		DirectX::XMVECTOR sphereLoc = XMLoadFloat3(&targetChain[i]->GetPosition());
		DirectX::XMVECTOR playerLoc = XMLoadFloat3(&playerPos);
		DirectX::XMVECTOR aimDirection = DirectX::XMVectorSubtract(sphereLoc, playerLoc);

		//Project the new vector onto the player direction and get the length of the new projected vector
		DirectX::XMFLOAT3 aimDirVec;
		DirectX::XMFLOAT3 dirVec;
		DirectX::XMStoreFloat3(&aimDirVec, aimDirection);
		DirectX::XMStoreFloat3(&dirVec, direction);

		float dotProduct = (aimDirVec.x * dirVec.x) + (aimDirVec.y * dirVec.y) + (aimDirVec.z * dirVec.z);

		//Find the point in the player's direction the length away
		direction = direction * dotProduct;
		DirectX::XMVECTOR pointOnLine = playerLoc + direction;
		DirectX::XMVECTOR distance = pointOnLine - sphereLoc;
		distance = DirectX::XMVector3Length(distance);
		float dist;
		DirectX::XMStoreFloat(&dist, distance);

		//Check to see if it is shorter than the radius of the sphere
		if (abs(dist) < chainScale * 0.5f)
		{
			printf("%f", dist);
			active = false;
			fall = true;
			targetChain.erase(targetChain.begin() + i, targetChain.begin() + chainCount);
			chainCount = i;
			return true;
		}
	}

	//do the same for the target
	//Get a normalized player direction
	DirectX::XMVECTOR direction = XMLoadFloat3(&playerDirection);
	direction = DirectX::XMVector3Normalize(direction);

	//Get a vector from player to sphere center
	DirectX::XMVECTOR sphereLoc = XMLoadFloat3(&mainTarget->GetPosition());
	DirectX::XMVECTOR playerLoc = XMLoadFloat3(&playerPos);
	DirectX::XMVECTOR aimDirection = DirectX::XMVectorSubtract(sphereLoc, playerLoc);

	//Project the new vector onto the player direction and get the length of the new projected vector
	DirectX::XMFLOAT3 aimDirVec;
	DirectX::XMFLOAT3 dirVec;
	DirectX::XMStoreFloat3(&aimDirVec, aimDirection);
	DirectX::XMStoreFloat3(&dirVec, direction);

	float dotProduct = (aimDirVec.x * dirVec.x) + (aimDirVec.y * dirVec.y) + (aimDirVec.z * dirVec.z);

	//Find the point in the player's direction the length away
	direction = direction * dotProduct;
	DirectX::XMVECTOR pointOnLine = playerLoc + direction;
	DirectX::XMVECTOR distance = pointOnLine - sphereLoc;
	distance = DirectX::XMVector3Length(distance);
	float dist;
	DirectX::XMStoreFloat(&dist, distance);

	//Check to see if it is shorter than the radius of the sphere
	if (abs(dist) < targetScale * 0.5f)
	{
		drawTarget = false;
		active = false;
		printf(" %f", dist);
		return true;
	}

	return collision;
}

bool Target::GetActive()
{
	return active;
}

bool Target::GetFall()
{
	return fall;
}