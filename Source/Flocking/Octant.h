// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "BoidManager.h"

#include "Octant.generated.h"

UCLASS()
class FLOCKING_API AOctant : public AActor
{
	GENERATED_BODY()

public:

	int octantCount;
	int maxLevel;
	int idealEntityCount;

	int ID;
	int level;

	float size;

	FVector center;
	FVector min;
	FVector max;

	TArray<FBoidInfo*> boids;

	TArray<AOctant*> children;

	AOctant* root;

	//constructor
	AOctant();

	void InitChild();

	//init the root of the tree
	void InitRoot(int maxLevel, int idealEntityCount, float size, TArray<FBoidInfo*> boids);

	//Asks if there is a collision with the Entity specified by index from
	bool IsColliding(int index);

	//Displays the MyOctant volume in the color specified
	void Display(FColor color);

	//allocates 8 smaller octants in the child pointers
	void Subdivide();

	//Asks the MyOctant if it contains more than this many Bounding Objects
	bool ContainsMoreThan(int entities);

	//Creates a tree using subdivisions, the max number of objects and levels
	void ConstructTree();

	//Traverse the tree up to the leafs and sets the objects in them to the index
	void AssignIDtoEntity();

	void KillBranches();

};
