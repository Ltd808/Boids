// Fill out your copyright notice in the Description page of Project Settings.
#include "Octant.h"

void AOctant::Init(UStaticMesh* bodyRef)
{
	body->SetStaticMesh(bodyRef);
}

// Called when the game starts or when spawned
void AOctant::BeginPlay()
{
	if (this == root)
	{
		ConstructTree();
	}
}

// Called every frame
void AOctant::Tick(float DeltaTime)
{

}

//child constructor
AOctant::AOctant()
{
	PrimaryActorTick.bCanEverTick = true;

	body = CreateDefaultSubobject<UStaticMeshComponent>("Body");
	RootComponent = body;

	center = GetActorLocation();
	min = center - (FVector(size / 2));
	max = center + (FVector(size / 2));

	m_pParent = nullptr;

	root = nullptr;
}

//root constructor
AOctant::AOctant(int a_nMaxLevel, int a_nIdealEntityCount, float size)
{
	PrimaryActorTick.bCanEverTick = true;

	body = CreateDefaultSubobject<UStaticMeshComponent>("Body");
	RootComponent = body;

	octantCount = 1;
	maxLevel = a_nMaxLevel;
	idealEntityCount = a_nIdealEntityCount;

	ID = 0;
	level = 0;

	size = size;

	center = FVector(0.0f);
	min = -FVector(size / 2);
	max = FVector(size / 2);

	m_pParent = nullptr;

	root = this;
}


//destuctor
AOctant::~AOctant()
{
	KillBranches();
}

//Creates a tree from root to max level
void AOctant::ConstructTree()
{
	//only use on root
	if (level != 0) return;

	//subdivide (recursive function)
	if (ContainsMoreThan(idealEntityCount))
	{
		Subdivide();
	}

	//assign IDS
	AssignIDtoEntity();
}

//allocate child octants
void AOctant::Subdivide()
{
	if (level == maxLevel || children.Num() != 0) { return; }

	//child positions and dimensions
	float childSize = size / 2;
	float childSizeS = size / 4;
	FVector childCenter = center;

	//bottom left
	childCenter.X -= childSizeS;
	childCenter.Y -= childSizeS;
	childCenter.Z -= childSizeS;

	FTransform newTransform;
	FActorSpawnParameters spawnParams;

	newTransform.SetLocation(childCenter);
	newTransform.SetScale3D(FVector(childSize));
	children.Add(GetWorld()->SpawnActor<AOctant>(AOctant::StaticClass(), newTransform, spawnParams));

	//bottom right
	childCenter.X += childSize;
	newTransform.SetLocation(childCenter);
	newTransform.SetScale3D(FVector(childSize));
	children.Add(GetWorld()->SpawnActor<AOctant>(AOctant::StaticClass(), newTransform, spawnParams));

	//bottom right front
	childCenter.Z += childSize;
	newTransform.SetLocation(childCenter);
	newTransform.SetScale3D(FVector(childSize));
	children.Add(GetWorld()->SpawnActor<AOctant>(AOctant::StaticClass(), newTransform, spawnParams));

	//bottom left front
	childCenter.X -= childSize;
	newTransform.SetLocation(childCenter);
	newTransform.SetScale3D(FVector(childSize));
	children.Add(GetWorld()->SpawnActor<AOctant>(AOctant::StaticClass(), newTransform, spawnParams));

	//top left front
	childCenter.Y += childSize;
	newTransform.SetLocation(childCenter);
	newTransform.SetScale3D(FVector(childSize));
	children.Add(GetWorld()->SpawnActor<AOctant>(AOctant::StaticClass(), newTransform, spawnParams));

	//top right front
	childCenter.X += childSize;
	newTransform.SetLocation(childCenter);
	newTransform.SetScale3D(FVector(childSize));
	children.Add(GetWorld()->SpawnActor<AOctant>(AOctant::StaticClass(), newTransform, spawnParams));

	//top right
	childCenter.Z -= childSize;
	newTransform.SetLocation(childCenter);
	newTransform.SetScale3D(FVector(childSize));
	children.Add(GetWorld()->SpawnActor<AOctant>(AOctant::StaticClass(), newTransform, spawnParams));

	//top left 
	childCenter.X -= childSize;
	newTransform.SetLocation(childCenter);
	newTransform.SetScale3D(FVector(childSize));
	children.Add(GetWorld()->SpawnActor<AOctant>(AOctant::StaticClass(), newTransform, spawnParams));

	for (int i = 0; i < children.Num(); i++)
	{
		//set new variables
		children[i]->level = level + 1;

		children[i]->m_pParent = this;

		children[i]->root = root;

		root->octantCount++;

		children[i]->ID = root->octantCount - 1;

		//has more than idea and isnt max level call resursively
		if (children[i]->ContainsMoreThan(idealEntityCount) && children[i]->level < root->maxLevel)
		{
			children[i]->Subdivide();
		}
	}
}

//Asks the MyOctant if it contains more than this many Bounding Objects
bool AOctant::ContainsMoreThan(int a_nEntities)
{
	int entityCount = 200;

	//counter
	int collidingObjects = 0;
	for (int i = 0; i < entityCount; i++)
	{
		if (IsColliding(i))
		{
			collidingObjects++;
		}
		if (collidingObjects > a_nEntities)
		{
			return true;
		}
	}

	return false;
}


//check if entity is in this octant
bool AOctant::IsColliding(int a_uRBIndex)
{
	//int entityCount = 200;

	////index goes over
	//if (a_uRBIndex >= entityCount || entityCount < 0) { return false; }

	////get rigidbody
	//MyEntity* entityToCheck = m_pEntityMngr->GetEntity(a_uRBIndex);
	//MyRigidBody* rigidBodyToCheck = entityToCheck->GetRigidBody();

	////get bounds
	//vector3 entityMin = rigidBodyToCheck->GetMinGlobal();
	//vector3 entityMax = rigidBodyToCheck->GetMaxGlobal();

	////AABB
	//return (m_v3Min.x <= entityMax.x && m_v3Max.x >= entityMin.x) &&
	//	(m_v3Min.y <= entityMax.y && m_v3Max.y >= entityMin.y) &&
	//	(m_v3Min.z <= entityMax.z && m_v3Max.z >= entityMin.z);
	return true;
}

//Deletes all children and the children of their children recursively
void AOctant::KillBranches()
{
	for (size_t i = 0; i < children.Num(); i++)
	{
		//if there are children call recursively
		if (children[i]->children.Num() > 0)
		{
			children[i]->KillBranches();
		}

		//deallocate
		children[i]->~AOctant();
	}
}

//display going down the tree
void AOctant::Display()
{
	////display self
	//matrix4 boxmatrix = glm::translate(identity_m4, m_v3center) * glm::scale(vector3(m_fsize));
	//m_pmeshmngr->addcubetorenderlist(boxmatrix, a_v3color, render_wire);

	////get out if no children
	//if (m_uchildren == 0) { return; }

	////recursively call display on children
	//for (uint i = 0; i < m_uchildren; i++)
	//{
	//	m_pchild[i]->display(a_v3color);
	//}
}


void AOctant::AssignIDtoEntity(void)
{
	////call recursively
	//for (size_t i = 0; i < m_uChildren; i++)
	//{
	//	m_pChild[i]->AssignIDtoEntity();
	//}

	////only assign for end nodes
	//if (m_uChildren == 0)
	//{
	//	uint entityCount = m_pEntityMngr->GetEntityCount();

	//	//go through all entities
	//	for (uint i = 0; i < entityCount; i++)
	//	{
	//		//add if colliding
	//		if (IsColliding(i))
	//		{
	//			//add entity to list
	//			m_EntityList.push_back(i);

	//			//set dimension of entity to current octant
	//			m_pEntityMngr->AddDimension(i, m_uID);
	//		}
	//	}
	//}
}
