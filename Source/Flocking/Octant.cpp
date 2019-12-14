// Fill out your copyright notice in the Description page of Project Settings.
#include "Octant.h"

void AOctant::InitRoot(int a_maxLevel, int a_idealEntityCount, float a_size, TArray<FBoidInfo*> a_boids)
{
	boids = a_boids;

	size = a_size;
	maxLevel = a_maxLevel;
	idealEntityCount = a_idealEntityCount;

	octantCount = 1;
	ID = 0;
	level = 0;


	center = FVector(0.0f);
	min = -FVector(size / 2);
	max = FVector(size / 2);

	root = this;

	ConstructTree();
}

//child constructo
void AOctant::InitChild()
{
	min = center - FVector(size / 2);
	max = center + FVector(size / 2);
}

AOctant::AOctant()
{

}

//Creates a tree from root to max level
void AOctant::ConstructTree()
{
	//only use on root
	if (level != 0) return;

	//subdivide (recursive function)
	Subdivide();
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

	for (size_t i = 0; i < 8; i++)
	{
		children.Add(GetWorld()->SpawnActor<AOctant>(AOctant::StaticClass(), newTransform, spawnParams));
	}

	children[0]->center = childCenter;

	//bottom right
	childCenter.X += childSize;
	children[1]->center = childCenter;

	//bottom right front
	childCenter.Z += childSize;
	children[2]->center = childCenter;

	//bottom left front
	childCenter.X -= childSize;
	children[3]->center = childCenter;

	//top left front
	childCenter.Y += childSize;
	children[4]->center = childCenter;

	//top right front
	childCenter.X += childSize;
	children[5]->center = childCenter;

	//top right
	childCenter.Z -= childSize;
	children[6]->center = childCenter;

	//top left 
	childCenter.X -= childSize;
	children[7]->center = childCenter;

	for (int i = 0; i < children.Num(); i++)
	{
		children[i]->size = childSize;

		//set new variables
		children[i]->level = level + 1;

		children[i]->root = root;

		children[i]->boids = boids;

		children[i]->InitChild();

		root->octantCount++;

		children[i]->ID = root->octantCount - 1;

		children[i]->idealEntityCount = idealEntityCount;

		//has more than idea and isnt max level call resursively
		if (children[i]->ContainsMoreThan(idealEntityCount) && children[i]->level < root->maxLevel)
		{
			children[i]->Subdivide();
		}
	}
}

//Deletes all children and the children of their children recursively
void AOctant::KillBranches()
{
	for (int i = 0; i < children.Num(); i++)
	{
		////if there are children call recursively
		if (children[i]->children.Num() > 0)
		{
			children[i]->KillBranches();
		}

		//deallocate
		GetWorld()->DestroyActor(children[i]);
	}
	children.Empty();
}

//check if entity is in this octant
bool AOctant::IsColliding(int index)
{
	//index goes over
	if (index >= boids.Num()) { return false; }

	//get bounds
	FVector entityMin = boids[index]->Transform.GetLocation() - FVector(25, 25, 25);
	FVector entityMax = boids[index]->Transform.GetLocation() + FVector(25, 25, 25);

	//AABB
	return (min.X <= entityMax.X && max.X >= entityMin.X) &&
		(min.Y <= entityMax.Y && max.Y >= entityMin.Y) &&
		(min.Z <= entityMax.Z && max.Z >= entityMin.Z);
}

//display going down the tree
void AOctant::Display(FColor color)
{
	//display and get out if no children
	if (children.Num() == 0) 
	{ 
		DrawDebugBox(GetWorld(), center, FVector(size / 2), FQuat::Identity, color, false, 2.0f, 0, 10);
		return; 
	}

	//recursively call display on children
	for (int i = 0; i < children.Num(); i++)
	{
		children[i]->Display(color);
	}
}


void AOctant::AssignIDtoEntity(void)
{
	////call recursively
	for (int i = 0; i < children.Num(); i++)
	{
		children[i]->AssignIDtoEntity();
	}

	//only assign for end nodes
	if (children.Num() == 0)
	{
		//go through all boids
		for (int i = 0; i < boids.Num(); i++)
		{
			//add if colliding
			if (IsColliding(i))
			{
				//add entity to list
				//boidsInOctant.Add(boids[i]);

				//set dimension of entity to current octant
				boids[i]->dimensionID = ID;
			}
		}
	}
}

//Asks the MyOctant if it contains more than this many Bounding Objects
bool AOctant::ContainsMoreThan(int a_nEntities)
{
	//counter
	int collidingObjects = 0;
	for (int i = 0; i < boids.Num(); i++)
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
