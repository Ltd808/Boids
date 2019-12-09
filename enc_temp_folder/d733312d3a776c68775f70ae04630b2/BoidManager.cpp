// Fill out your copyright notice in the Description page of Project Settings
#include "BoidManager.h"

// Sets default values
ABoidManager::ABoidManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

//called when the game starts or when spawned
void ABoidManager::BeginPlay()
{
	Super::BeginPlay();

	CalcPoints();

	for (int i = 0; i < boidCount; i++)
	{
		FTransform newTransform;
		FVector newPosition = FVector(FMath::RandRange(-1000.0f, 1000.0f), FMath::RandRange(-1000.0f, 1000.0f), FMath::RandRange(-1000.0f, 1000.0f));
		FActorSpawnParameters spawnParams;
		newTransform.SetLocation(newPosition);
		boids.Add(GetWorld()->SpawnActor<ABoid>(ABoid::StaticClass(), newTransform, spawnParams));
	}


	//init after array in initialized
	for (int i = 0; i < boidCount; i++)
	{
		boids[i]->Init(bodyRef, (TArray<AActor*>)boids, target, avoidWeight, seperationWeight, cohesionWeight, alignmentWeight, targetWeight, maxSpeed, minSpeed, maxForce, boundsRadius, collisionCheckDistance, numViewDirections, traceChannel, points);
	}
}

// Called every frame
void ABoidManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (int i = 0; i < boids.Num(); i++)
	{
		//create threads or run on main
		if (IsRunningOnMain) 
		{
			RunFlockTaskOnMain(boids, i, viewRadius, avoidRadius);
		}
		else
		{
			RunFlockTask(boids, i, viewRadius, avoidRadius);
		}
	}
}

//golden spiral method
void ABoidManager::CalcPoints()
{
	float goldenRatio = (1 + FMath::Sqrt(5)) / 2;
	float angleIncrement = PI * 2 * goldenRatio;

	for (int i = 0; i < numViewDirections; i++)
	{
		float t = (float)i / numViewDirections;
		float inclination = FMath::Acos(1 - 2 * t);
		float azimuth = angleIncrement * i;

		float x = FMath::Sin(inclination) * FMath::Cos(azimuth);
		float y = FMath::Sin(inclination) * FMath::Sin(azimuth);
		float z = FMath::Cos(inclination);

		//rotate torward forward like boid cone
		FTransform newTransform;
		FQuat deltaRotate;
		newTransform.SetRotation(newTransform.GetRotation() * deltaRotate.MakeFromEuler(FVector(0, -90, 0)));
		FVector viewDirection = UKismetMathLibrary::TransformDirection(newTransform, FVector(x, y, z));

		points.Add(viewDirection);
	}
}

void ABoidManager::RunFlockTask(TArray<ABoid*> a_boids, int a_boidCount, float a_viewRadius, float a_avoidRadius)
{
	(new FAutoDeleteAsyncTask<CalcFlockTask>(a_boids, a_boidCount, a_viewRadius, a_avoidRadius))->StartBackgroundTask();
}

void ABoidManager::RunFlockTaskOnMain(TArray<ABoid*> a_boids, int a_boidCount, float a_viewRadius, float a_avoidRadius)
{
	CalcFlockTask* task = new CalcFlockTask(a_boids, a_boidCount, a_viewRadius, a_avoidRadius);

	task->DoWorkMain();

	delete task;
}

//==============================================================================================================================================================
//threading

CalcFlockTask::CalcFlockTask(TArray<ABoid*> a_boids, int a_boidIndex, float a_viewRadius, float a_avoidRadius)
{
	boids = a_boids;
	index = a_boidIndex;
	viewRadius = a_viewRadius;
	avoidRadius = a_avoidRadius;
}

CalcFlockTask::~CalcFlockTask()
{
	//UE_LOG(LogTemp, Warning, TEXT("Task Finished."));
}

void CalcFlockTask::DoWork()
{
	boids[index]->numPerceivedFlockmates = 0;
	boids[index]->avgBoidDir = FVector(0.0f);
	boids[index]->avgAvoidDir = FVector(0.0f);
	boids[index]->centroid = FVector(0.0f);
	
	for (int i = 0; i < boids.Num(); i++)
	{
		ABoid* otherBoid = boids[i];
	
		if (index != i)
		{
			FVector offset = otherBoid->position - boids[index]->position;
			float sqrDst = offset.X * offset.X + offset.Y * offset.Y + offset.Z * offset.Z;
	
			if (sqrDst < viewRadius * viewRadius)
			{
				boids[index]->numPerceivedFlockmates++;
				boids[index]->avgBoidDir += otherBoid->direction;
				boids[index]->centroid += otherBoid->position;
	
				if (sqrDst < avoidRadius * avoidRadius)
				{
					boids[index]->avgAvoidDir -= offset / sqrDst;
				}
			}
		}
	}
}

void CalcFlockTask::DoWorkMain()
{
	DoWork();
}


