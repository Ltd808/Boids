//Fill out your copyright notice in the Description page of Project Settings
#include "BoidManager.h"

//Sets default values
ABoidManager::ABoidManager()
{
	//Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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


	//init after array in initialized so you can pass it along with everything else from the editor
	for (int i = 0; i < boidCount; i++)
	{
		boids[i]->Init(bodyRef, (TArray<AActor*>)boids, target, avoidWeight, seperationWeight, cohesionWeight, alignmentWeight, targetWeight, maxSpeed, minSpeed, maxForce, boundsRadius, viewRadius, avoidRadius, collisionCheckDistance, numViewDirections, traceChannel, points);
	}
}

//Called every frame
void ABoidManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (int i = 0; i < boids.Num(); i++)
	{
		//create threads or run on main
		if (IsRunningOnMain) 
		{
			RunFlockTaskOnMain(boids, i, DeltaTime);
		}
		else
		{
			RunFlockTask(boids, i, DeltaTime);
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

void ABoidManager::RunFlockTask(TArray<ABoid*> a_boids, int a_boidCount, float a_deltaTime)
{
	(new FAutoDeleteAsyncTask<CalcFlockTask>(a_boids, a_boidCount, a_deltaTime))->StartBackgroundTask();
}

void ABoidManager::RunFlockTaskOnMain(TArray<ABoid*> a_boids, int a_boidCount, float a_deltaTime)
{
	CalcFlockTask* task = new CalcFlockTask(a_boids, a_boidCount, a_deltaTime);

	task->DoWorkMain();

	delete task;
}

//==============================================================================================================================================================
//threading

CalcFlockTask::CalcFlockTask(TArray<ABoid*> a_boids, int a_boidIndex, float a_deltaTime)
{
	boids = a_boids;
	index = a_boidIndex;
	deltaTime = a_deltaTime;
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
	
			if (sqrDst < boids[index]->viewRadius * boids[index]->viewRadius)
			{
				boids[index]->numPerceivedFlockmates++;
				boids[index]->avgBoidDir += otherBoid->direction;
				boids[index]->centroid += otherBoid->position;
	
				if (sqrDst < boids[index]->avoidRadius * boids[index]->avoidRadius)
				{
					boids[index]->avgAvoidDir -= offset / sqrDst;
				}
			}
		}
	}

	boids[index]->acceleration = FVector(0);

	//target seek force
	if (boids[index]->target != nullptr) {
		FVector offsetToTarget = (boids[index]->target->GetActorLocation() - boids[index]->position);
		boids[index]->acceleration = boids[index]->GetForceToDirection(offsetToTarget) * boids[index]->targetWeight;
	}

	//flocking forces
	if (boids[index]->numPerceivedFlockmates != 0)
	{
		boids[index]->centroid /= boids[index]->numPerceivedFlockmates;

		boids[index]->acceleration += boids[index]->GetForceToDirection(boids[index]->avgBoidDir) * boids[index]->alignmentWeight;
		boids[index]->acceleration += boids[index]->GetForceToDirection(boids[index]->centroid - boids[index]->position) * boids[index]->cohesionWeight;
		boids[index]->acceleration += boids[index]->GetForceToDirection(boids[index]->avgAvoidDir) * boids[index]->seperationWeight;
	}

	//object avoidance
	if (boids[index]->IsCloseToObject())
	{
		FVector collisionAvoidForce = boids[index]->GetForceToDirection(boids[index]->GetAvoidDir()) * boids[index]->avoidWeight;
		boids[index]->acceleration += collisionAvoidForce;
	}

	boids[index]->velocity += boids[index]->acceleration * deltaTime;
	float speed = boids[index]->velocity.Size();
	boids[index]->direction = boids[index]->velocity / speed;
	speed = FMath::Clamp(speed, boids[index]->minSpeed, boids[index]->maxSpeed);
	boids[index]->velocity = boids[index]->direction * speed;
	boids[index]->position += boids[index]->velocity * deltaTime;
}

void CalcFlockTask::DoWorkMain()
{
	DoWork();
}


