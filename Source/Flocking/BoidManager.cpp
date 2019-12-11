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
		FVector newPosition = FVector(FMath::RandRange(-100.0f, 100.0f), FMath::RandRange(-100.0f, 100.0f), FMath::RandRange(-100.0f, 100.0f));
		FActorSpawnParameters spawnParams;
		newTransform.SetLocation(newPosition);
		boids.Add(GetWorld()->SpawnActor<ABoid>(ABoid::StaticClass(), newTransform, spawnParams));

		boids[i]->Init(bodyRef);
	}

	if (IsSpatialPartitioningEnabled)
	{
		FTransform newTransform;
		FActorSpawnParameters spawnParams;

		myOctant = GetWorld()->SpawnActor<AOctant>(AOctant::StaticClass(), newTransform, spawnParams);

		myOctant->InitRoot(maxOctreeLevel, octreeIdealBoidCount, 3000, boids);

		GetWorld()->GetTimerManager().SetTimer(timerHandle, this, &ABoidManager::CalcOctree, octreeTimerInterval, true, 0.0f);
	}
}

void ABoidManager::CalcOctree()
{
	myOctant->KillBranches();
	myOctant->ConstructTree();
	myOctant->AssignIDtoEntity();

	if (IsSpatialPartitioningDisplayOn) {
		myOctant->Display(FColor::Yellow);
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
			RunFlockTaskOnMain(i, DeltaTime, boids, points, boidInfo);
		}
		else
		{
			RunFlockTask(i, DeltaTime, boids, points, boidInfo);
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

void ABoidManager::RunFlockTask(int a_boidIndex, float a_deltaTime, TArray<ABoid*> a_boids, TArray<FVector> a_points, FBoidInfo a_boidInfo)
{
	(new FAutoDeleteAsyncTask<BoidWorker>(a_boidIndex, a_deltaTime, a_boids,  a_points, a_boidInfo))->StartBackgroundTask();
}

void ABoidManager::RunFlockTaskOnMain(int a_boidIndex, float a_deltaTime, TArray<ABoid*> a_boids, TArray<FVector> a_points, FBoidInfo a_boidInfo)
{
	BoidWorker* task = new BoidWorker(a_boidIndex, a_deltaTime, a_boids, a_points, a_boidInfo);

	task->DoWorkMain();

	delete task;
}

BoidWorker::BoidWorker(int a_boidIndex, float a_deltaTime, TArray<ABoid*> a_boids, TArray<FVector> a_points, FBoidInfo a_boidInfo)
{
	index = a_boidIndex;
	deltaTime = a_deltaTime;
	boids = a_boids;
	points = a_points;
	boidInfo = a_boidInfo;
}

BoidWorker::~BoidWorker()
{
	//UE_LOG(LogTemp, Warning, TEXT("Task Finished."));
}

void BoidWorker::DoWork()
{
	boids[index]->numPerceivedFlockmates = 0;
	boids[index]->avgBoidDir = FVector(0.0f);
	boids[index]->avgAvoidDir = FVector(0.0f);
	boids[index]->centroid = FVector(0.0f);
	
	for (int i = 0; i < boids.Num(); i++)
	{
		ABoid* otherBoid = boids[i];
	
		if (index != i && boids[index]->dimensionID == otherBoid->dimensionID)
		{
			FVector offset = otherBoid->position - boids[index]->position;
			float sqrDst = offset.X * offset.X + offset.Y * offset.Y + offset.Z * offset.Z;
	
			if (sqrDst < boidInfo.viewRadius * boidInfo.viewRadius)
			{
				boids[index]->numPerceivedFlockmates++;
				boids[index]->avgBoidDir += otherBoid->direction;
				boids[index]->centroid += otherBoid->position;
	
				if (sqrDst < boidInfo.avoidRadius * boidInfo.avoidRadius)
				{
					boids[index]->avgAvoidDir -= offset / sqrDst;
				}
			}
		}
	}

	boids[index]->acceleration = FVector(0);

	//target seek force CAUSES MASSIVE FRAME DROPS DUE TO GET ACTOR LOCATION
	//if (boidInfo.target != nullptr) {
	//	FVector offsetToTarget = (boidInfo.target->GetActorLocation() - boids[index]->position); 
	//	boids[index]->acceleration = GetForceToDirection(offsetToTarget) * boidInfo.targetWeight;
	//}

	//flocking forces
	if (boids[index]->numPerceivedFlockmates != 0)
	{
		boids[index]->centroid /= boids[index]->numPerceivedFlockmates;

		boids[index]->acceleration += GetForceToDirection(boids[index]->avgBoidDir) * boidInfo.alignmentWeight;
		boids[index]->acceleration += GetForceToDirection(boids[index]->centroid - boids[index]->position) * boidInfo.cohesionWeight;
		boids[index]->acceleration += GetForceToDirection(boids[index]->avgAvoidDir) * boidInfo.seperationWeight;
	}

	//object avoidance
	if (IsCloseToObject())
	{
		FVector collisionAvoidForce = GetForceToDirection(GetAvoidDir()) * boidInfo.avoidWeight;
		boids[index]->acceleration += collisionAvoidForce;
	}

	boids[index]->velocity += boids[index]->acceleration * deltaTime;
	float speed = boids[index]->velocity.Size();
	boids[index]->direction = boids[index]->velocity / speed;
	speed = FMath::Clamp(speed, boidInfo.minSpeed, boidInfo.maxSpeed);
	boids[index]->velocity = boids[index]->direction * speed;
	boids[index]->position += boids[index]->velocity * deltaTime;
}

void BoidWorker::DoWorkMain()
{
	DoWork();
}

bool BoidWorker::IsCloseToObject()
{
	//hit result
	FHitResult hit;

	if (UKismetSystemLibrary::SphereTraceSingle((UObject*)boids[index]->GetWorld(), boids[index]->position, boids[index]->position + boids[index]->direction * boidInfo.collisionCheckDistance, 
		boidInfo.boundsRadius, boidInfo.traceChannel, false, (TArray<AActor*>)boids, EDrawDebugTrace::None, hit, true))
	{
		return true;
	}

	return false;
}

FVector BoidWorker::GetAvoidDir()
{
	for (int i = 0; i < points.Num(); i++)
	{
		//rotate the point torwards forward
		FTransform newTransform = boids[index]->GetTransform();
		FVector viewDirection = UKismetMathLibrary::TransformDirection(newTransform, points[i]);

		FHitResult hit;

		TArray<AActor*> empty;

		if (!UKismetSystemLibrary::SphereTraceSingle((UObject*)boids[index]->GetWorld(), boids[index]->position, boids[index]->position + viewDirection * boidInfo.collisionCheckDistance, 
			boidInfo.boundsRadius, boidInfo.traceChannel, false, (TArray<AActor*>)boids, EDrawDebugTrace::None, hit, true))
		{
			return viewDirection;
		}
	}

	return boids[index]->direction;
}

FVector BoidWorker::GetForceToDirection(FVector a_direction)
{
	FVector direction = (a_direction.GetSafeNormal() * boidInfo.maxSpeed) - boids[index]->velocity;
	return direction.GetClampedToMaxSize(boidInfo.maxForce);
}




