// Fill out your copyright notice in the Description page of Project Settings
#include "BoidManager.h"

// Sets default values
ABoidManager::ABoidManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABoidManager::BeginPlay()
{
	Super::BeginPlay();

	for (int i = 0; i < boidCount; i++)
	{
		FTransform newTransform;
		FVector newPosition = FVector(FMath::RandRange(-1000.0f, 1000.0f), FMath::RandRange(-1000.0f, 1000.0f), FMath::RandRange(-1000.0f, 1000.0f));
		FActorSpawnParameters spawnParams;
		newTransform.SetLocation(newPosition);
		boids.Add(GetWorld()->SpawnActor<ABoid>(ABoid::StaticClass(), newTransform, spawnParams));
	}

	for (int i = 0; i < boidCount; i++)
	{
		boids[i]->Init(bodyRef, (TArray<AActor*>)boids);
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
	boids[index]->avgBoidDir = FVector(0);
	boids[index]->avgAvoidDir = FVector(0);
	boids[index]->centroid = FVector(0);
	
	for (int i = 0; i < boids.Num(); i++)
	{
		ABoid* currentBoid = boids[i];
	
		if (index != i)
		{
			FVector offset = currentBoid->position - boids[index]->position;
			float sqrDst = offset.X * offset.X + offset.Y * offset.Y + offset.Z * offset.Z;
	
			if (sqrDst < viewRadius * viewRadius)
			{
				boids[index]->numPerceivedFlockmates++;
				boids[index]->avgBoidDir += currentBoid->direction;
				boids[index]->centroid += currentBoid->position;
	
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
