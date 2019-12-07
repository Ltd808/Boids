//// Fill out your copyright notice in the Description page of Project Settings.
//#include "Octant.h"
//#include "BoidManager.h"
//
//// Called when the game starts or when spawned
//void AOctant::BeginPlay()
//{
//	Super::BeginPlay();
//	
//}
//
//// Called every frame
//void AOctant::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}
////root constructor
//AOctant::AOctant(int a_nMaxLevel, int a_nIdealEntityCount, float size)
//{
//	PrimaryActorTick.bCanEverTick = true;
//	m_uOctantCount = 1;
//	m_uMaxLevel = a_nMaxLevel;
//	m_uIdealEntityCount = a_nIdealEntityCount;
//
//	m_uID = 0;
//	m_uLevel = 0;
//	m_uChildren = 0;
//
//	m_fSize = size;
//
//	m_v3Center = FVector(0.0f);
//	m_v3Min = -FVector(m_fSize / 2);
//	m_v3Max = FVector(m_fSize / 2);
//
//	m_pParent = nullptr;
//	for (int i = 0; i < 8; i++)
//	{
//		m_pChild[i] = nullptr;
//	}
//
//	m_pRoot = this;
//
//	ConstructTree();
//}
//
////child constructor
//AOctant::AOctant(FVector a_v3Center, float a_fSize)
//{
//	PrimaryActorTick.bCanEverTick = true;
//	m_uChildren = 0;
//
//	m_fSize = a_fSize;
//
//	m_v3Center = a_v3Center;
//	m_v3Min = m_v3Center - (FVector(m_fSize / 2));
//	m_v3Max = m_v3Center + (FVector(m_fSize / 2));
//
//	m_pParent = nullptr;
//	for (int i = 0; i < 8; i++)
//	{
//		m_pChild[i] = nullptr;
//	}
//
//	m_pRoot = nullptr;
//}
//
////BIG THREE
////copy constructor
//AOctant::AOctant(AOctant const& other)
//{
//	m_uOctantCount = other.m_uOctantCount;
//	m_uMaxLevel = other.m_uMaxLevel;
//	m_uIdealEntityCount = other.m_uIdealEntityCount;
//
//	m_uID = other.m_uID;
//	m_uLevel = 0; other.m_uLevel;
//	m_uChildren = 0; other.m_uChildren;
//
//	m_fSize = other.m_fSize;
//
//	m_v3Center = other.m_v3Center;
//	m_v3Min = other.m_v3Min;
//	m_v3Max = other.m_v3Max;
//
//	m_pParent = other.m_pParent;
//	for (int i = 0; i < 8; i++)
//	{
//		m_pChild[i] = other.m_pChild[i];
//	}
//
//	m_EntityList = other.m_EntityList;
//
//	m_pRoot = other.m_pRoot;
//}
//
////copy assignment
//AOctant& AOctant::operator=(AOctant const& other)
//{
//	if (this != &other)
//	{
//		///get rid of old memory
//		delete m_pParent;
//		delete[] m_pChild;
//		delete m_pRoot;
//
//		//copy other
//		m_uOctantCount = other.m_uOctantCount;
//		m_uMaxLevel = other.m_uMaxLevel;
//		m_uIdealEntityCount = other.m_uIdealEntityCount;
//
//		m_uID = other.m_uID;
//		m_uLevel = 0; other.m_uLevel;
//		m_uChildren = 0; other.m_uChildren;
//
//		m_fSize = other.m_fSize;
//
//		m_v3Center = other.m_v3Center;
//		m_v3Min = other.m_v3Min;
//		m_v3Max = other.m_v3Max;
//
//		m_pParent = other.m_pParent;
//		for (int i = 0; i < 8; i++)
//		{
//			m_pChild[i] = other.m_pChild[i];
//		}
//
//		m_EntityList = other.m_EntityList;
//
//		m_pRoot = other.m_pRoot;
//	}
//
//	return *this;
//}
//
////destuctor
//AOctant::~AOctant(void)
//{
//	KillBranches();
//}
////Creates a tree from root to max level
//void AOctant::ConstructTree()
//{
//	//only use on root
//	if (m_uLevel != 0) return;
//
//	//subdivide (recursive function)
//	if (ContainsMoreThan(m_uIdealEntityCount))
//	{
//		Subdivide();
//	}
//
//	//assign IDS
//	AssignIDtoEntity();
//}
//
////allocate child octants
//void AOctant::Subdivide(void)
//{
//	if (m_uLevel == m_uMaxLevel || m_uChildren != 0) { return; }
//
//	//create children
//	m_uChildren = 8;
//
//	//child positions and dimensions
//	float childSize = m_fSize / 2;
//	float childSizeS = m_fSize / 4;
//	FVector childCenter = m_v3Center;
//
//	//bottom left
//	childCenter.X -= childSizeS;
//	childCenter.Y -= childSizeS;
//	childCenter.Z -= childSizeS;
//	m_pChild[0] = new AOctant(childCenter, childSize);
//
//	//bottom right
//	childCenter.X += childSize;
//	m_pChild[1] = new AOctant(childCenter, childSize);
//
//	//bottom right front
//	childCenter.Z += childSize;
//	m_pChild[2] = new AOctant(childCenter, childSize);
//
//	//bottom left front
//	childCenter.X -= childSize;
//	m_pChild[3] = AOctant(childCenter, childSize);
//
//	//top left front
//	childCenter.Y += childSize;
//	m_pChild[4] = AOctant(childCenter, childSize);
//
//	//top right front
//	childCenter.X += childSize;
//	m_pChild[5] = new AOctant(childCenter, childSize);
//
//	//top right
//	childCenter.Z -= childSize;
//	m_pChild[6] = new AOctant(childCenter, childSize);
//
//	//top left 
//	childCenter.X -= childSize;
//	m_pChild[7] = new AOctant(childCenter, childSize);
//
//	for (int i = 0; i < m_uChildren; i++)
//	{
//		//set new variables
//		m_pChild[i]->m_uLevel = m_uLevel + 1;
//
//		m_pChild[i]->m_pParent = this;
//
//		m_pChild[i]->m_pRoot = m_pRoot;
//
//		m_pRoot->m_uOctantCount++;
//
//		m_pChild[i]->m_uID = m_pRoot->m_uOctantCount - 1;
//
//		//has more than idea and isnt max level call resursively
//		if (m_pChild[i]->ContainsMoreThan(m_uIdealEntityCount) && m_pChild[i]->m_uLevel < m_pRoot->m_uMaxLevel)
//		{
//			m_pChild[i]->Subdivide();
//		}
//	}
//}
//
////Asks the MyOctant if it contains more than this many Bounding Objects
//bool AOctant::ContainsMoreThan(int a_nEntities)
//{
//	int entityCount = 200;
//
//	//counter
//	int collidingObjects = 0;
//	for (int i = 0; i < entityCount; i++)
//	{
//		if (IsColliding(i))
//		{
//			collidingObjects++;
//		}
//		if (collidingObjects > a_nEntities)
//		{
//			return true;
//		}
//	}
//
//	return false;
//}
//
//
////check if entity is in this octant
//bool AOctant::IsColliding(int a_uRBIndex)
//{
//	int entityCount = 200;
//
//	//index goes over
//	if (a_uRBIndex >= entityCount || entityCount < 0) { return false; }
//
//	//get rigidbody
//	MyEntity* entityToCheck = m_pEntityMngr->GetEntity(a_uRBIndex);
//	MyRigidBody* rigidBodyToCheck = entityToCheck->GetRigidBody();
//
//	//get bounds
//	vector3 entityMin = rigidBodyToCheck->GetMinGlobal();
//	vector3 entityMax = rigidBodyToCheck->GetMaxGlobal();
//
//	//AABB
//	return (m_v3Min.x <= entityMax.x && m_v3Max.x >= entityMin.x) &&
//		(m_v3Min.y <= entityMax.y && m_v3Max.y >= entityMin.y) &&
//		(m_v3Min.z <= entityMax.z && m_v3Max.z >= entityMin.z);
//}
//
////Deletes all children and the children of their children recursively
//void MyOctant::KillBranches(void)
//{
//	for (size_t i = 0; i < m_uChildren; i++)
//	{
//		//if there are children call recursively
//		if (m_pChild[i]->m_uChildren > 0)
//		{
//			m_pChild[i]->KillBranches();
//		}
//
//		//deallocate
//		delete m_pChild[i];
//	}
//
//	m_uChildren = 0;
//}
//
////display going down the tree
//void MyOctant::Display(vector3 a_v3Color)
//{
//	//display self
//	matrix4 boxMatrix = glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize));
//	m_pMeshMngr->AddCubeToRenderList(boxMatrix, a_v3Color, RENDER_WIRE);
//
//	//get out if no children
//	if (m_uChildren == 0) { return; }
//
//	//recursively call display on children
//	for (uint i = 0; i < m_uChildren; i++)
//	{
//		m_pChild[i]->Display(a_v3Color);
//	}
//}
//
//
//void MyOctant::AssignIDtoEntity(void)
//{
//	//call recursively
//	for (size_t i = 0; i < m_uChildren; i++)
//	{
//		m_pChild[i]->AssignIDtoEntity();
//	}
//
//	//only assign for end nodes
//	if (m_uChildren == 0)
//	{
//		uint entityCount = m_pEntityMngr->GetEntityCount();
//
//		//go through all entities
//		for (uint i = 0; i < entityCount; i++)
//		{
//			//add if colliding
//			if (IsColliding(i))
//			{
//				//add entity to list
//				m_EntityList.push_back(i);
//
//				//set dimension of entity to current octant
//				m_pEntityMngr->AddDimension(i, m_uID);
//			}
//		}
//	}
//}
