// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Engine/World.h"
#include "Boid.h"

#include "Octant.generated.h"

UCLASS()
class FLOCKING_API AOctant : public AActor
{
	GENERATED_BODY()

public:

	UStaticMeshComponent* body;

	int octantCount;
	int maxLevel;
	int idealEntityCount;

	int ID = 0;
	int level = 0;

	float size = 0.0f;

	FVector center = FVector(0.0f);
	FVector min = FVector(0.0f);
	FVector max = FVector(0.0f);

	AOctant* m_pParent = nullptr;

	TArray<ABoid*> boidList;
	TArray<AOctant*> children;

	AOctant* root = nullptr;

	void Init(UStaticMesh* bodyRef);

	//child constructor
	AOctant();

	//root node constructor
	AOctant(int a_MaxLevel, int a_IdealEntityCount, float a_size);

	//Destructor
	~AOctant(void);

	//Asks if there is a collision with the Entity specified by index from
	bool IsColliding(int a_index);

	//Displays the MyOctant volume in the color specified
	void Display();

	//allocates 8 smaller octants in the child pointers
	void Subdivide();

	//Asks the MyOctant if it contains more than this many Bounding Objects
	bool ContainsMoreThan(int a_Entities);

	//Deletes all children and the children of their children (almost sounds apocalyptic)
	void KillBranches();

	//Creates a tree using subdivisions, the max number of objects and levels
	void ConstructTree();

	//Traverse the tree up to the leafs and sets the objects in them to the index
	void AssignIDtoEntity();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
