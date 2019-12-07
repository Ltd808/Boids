//// Fill out your copyright notice in the Description page of Project Settings.
//
//#pragma once
//
//#include "CoreMinimal.h"
//#include "GameFramework/Actor.h"
//#include "Octant.generated.h"
//
//UCLASS()
//class FLOCKING_API AOctant : public AActor
//{
//	GENERATED_BODY()
//	
//public:	
//
//	int m_uOctantCount; 
//	int m_uMaxLevel;
//	int m_uIdealEntityCount;
//
//	int m_uID = 0; 
//	int m_uLevel = 0; 
//	int m_uChildren = 0;
//
//	float m_fSize = 0.0f; 
//
//	FVector m_v3Center = FVector(0.0f); 
//	FVector m_v3Min = FVector(0.0f);
//	FVector m_v3Max = FVector(0.0f); 
//
//	AOctant* m_pParent = nullptr;
//	AOctant* m_pChild[8];
//
//	TArray<int> m_EntityList; 
//
//	AOctant* m_pRoot = nullptr;
//
//	//root node constructor
//	AOctant(int a_nMaxLevel = 2, int a_nIdealEntityCount = 5, float size = 34.0f);
//
//	//child constructor
//	AOctant(FVector a_v3Center, float a_fSize);
//
//	//BIG THREE
//	//copy Constructor
//	AOctant(AOctant const& other);
//
//	//Copy Assignment Operator
//	AOctant& operator=(AOctant const& other);
//
//	//Destructor
//	~AOctant(void);
//
//	//Asks if there is a collision with the Entity specified by index from
//	bool IsColliding(int a_uRBIndex);
//
//	//Displays the MyOctant volume in the color specified
//	void Display();
//
//	//allocates 8 smaller octants in the child pointers
//	void Subdivide(void);
//
//	//Asks the MyOctant if it contains more than this many Bounding Objects
//	bool ContainsMoreThan(int a_nEntities);
//
//	//Deletes all children and the children of their children (almost sounds apocalyptic)
//	void KillBranches(void);
//
//	//Creates a tree using subdivisions, the max number of objects and levels
//	void ConstructTree();
//
//	//Traverse the tree up to the leafs and sets the objects in them to the index
//	void AssignIDtoEntity(void);
//
//protected:
//	// Called when the game starts or when spawned
//	virtual void BeginPlay() override;
//
//public:	
//	// Called every frame
//	virtual void Tick(float DeltaTime) override;
//
//};
