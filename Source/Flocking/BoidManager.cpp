//Fill out your copyright notice in the Description page of Project Settings
#include "BoidManager.h"
#include "Octant.h"
//Sets default values
ABoidManager::ABoidManager(const FObjectInitializer& ObjectInitializer)
{
	//Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	boids = ObjectInitializer.CreateDefaultSubobject<UInstancedStaticMeshComponent>(this, TEXT("Boifeefed"));
	RootComponent = boids;
}

//called when the game starts or when spawned
void ABoidManager::BeginPlay()
{
	Super::BeginPlay();

	CalcPoints();



	for (size_t i = 0; i < boidCount; i++)
	{ 
		boidInfo.Add(new FBoidInfo());

		boidInfo[i]->transform.SetLocation(FVector(FMath::RandRange(-500, 500), FMath::RandRange(-500, 500), FMath::RandRange(-500, 500)));

		boidInfo[i]->meshID = boids->AddInstance(boidInfo[i]->transform);

		boidInfo[i]->velocity = FVector(FMath::RandRange(-minSpeed, maxSpeed));
		boidInfo[i]->direction = boidInfo[i]->velocity / boidInfo[i]->velocity.Size();
	}


	if (IsSpatialPartitioningEnabled)
	{
		FTransform newTransform;
		FActorSpawnParameters spawnParams;

		myOctant = GetWorld()->SpawnActor<AOctant>(AOctant::StaticClass(), newTransform, spawnParams);

		myOctant->InitRoot(maxOctreeLevel, octreeIdealBoidCount, 3000, boidInfo);

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

	for (int i = 0; i < boidCount; i++)
	{
		//create threads or run on main
		if (IsRunningOnMain)
		{
			RunFlockTaskOnMain(i, DeltaTime, boidInfo, viewRadius, avoidRadius);
		}
		else
		{
			RunFlockTask(i, DeltaTime, boidInfo, viewRadius, avoidRadius);
		}
	}

	for (int i = 0; i < boidCount; i++)
	{
		boidInfo[i]->acceleration = FVector(0);

		//target seek force CAUSES MASSIVE FRAME DROPS DUE TO GET ACTOR LOCATION
		//if (boidInfo.target != nullptr) {
		//	FVector offsetToTarget = (boidInfo.target->GetActorLocation() - boids[index]->position); 
		//	boids[index]->acceleration = GetForceToDirection(offsetToTarget) * boidInfo.targetWeight;
		//}

		//flocking forces
		if (boidInfo[i]->numPerceivedFlockmates != 0)
		{
			boidInfo[i]->centroid /= boidInfo[i]->numPerceivedFlockmates;

			boidInfo[i]->acceleration += GetForceToDirection(boidInfo[i]->avgBoidDir, i) * alignmentWeight;
			boidInfo[i]->acceleration += GetForceToDirection(boidInfo[i]->centroid - boidInfo[i]->transform.GetLocation(), i) * cohesionWeight;
			boidInfo[i]->acceleration += GetForceToDirection(boidInfo[i]->avgAvoidDir, i) * seperationWeight;
		}

		//object avoidance
		if (IsCloseToObject(i))
		{
			FVector collisionAvoidForce = GetForceToDirection(GetAvoidDir(i), i) * avoidWeight;
			boidInfo[i]->acceleration += collisionAvoidForce;
		}

		boidInfo[i]->velocity += boidInfo[i]->acceleration * DeltaTime;
		float speed = boidInfo[i]->velocity.Size();
		boidInfo[i]->direction = boidInfo[i]->velocity / speed;
	    speed = FMath::Clamp(speed, minSpeed, maxSpeed);
		boidInfo[i]->velocity = boidInfo[i]->direction * speed;

		boidInfo[i]->transform.SetLocation(boidInfo[i]->transform.GetLocation() + boidInfo[i]->velocity * DeltaTime);
		boidInfo[i]->transform.SetRotation(UKismetMathLibrary::FindLookAtRotation(boidInfo[i]->transform.GetLocation(), boidInfo[i]->transform.GetLocation() + boidInfo[i]->direction).Quaternion());

		boids->UpdateInstanceTransform(boidInfo[i]->meshID, boidInfo[i]->transform);
	}

	//boids->ReleasePerInstanceRenderData();
	boids->MarkRenderStateDirty();
}

bool ABoidManager::IsCloseToObject(int index)
{
	//hit result
	FHitResult hit;

	TArray<AActor*> empty;

	if (UKismetSystemLibrary::SphereTraceSingle(GetWorld(), boidInfo[index]->transform.GetLocation(), boidInfo[index]->transform.GetLocation() + boidInfo[index]->direction * collisionCheckDistance,
		boundsRadius, traceChannel, false, empty, EDrawDebugTrace::None, hit, true))
	{
		return true;
	}

	return false;
}

FVector ABoidManager::GetAvoidDir(int index)
{
	for (int i = 0; i < points.Num(); i++)
	{
		//rotate the point torwards forward
		FVector viewDirection = UKismetMathLibrary::TransformDirection(boidInfo[index]->transform, points[i]);

		FHitResult hit;

		TArray<AActor*> empty;

		if (!UKismetSystemLibrary::SphereTraceSingle(GetWorld(), boidInfo[index]->transform.GetLocation(), boidInfo[index]->transform.GetLocation() + viewDirection * collisionCheckDistance,
			boundsRadius, traceChannel, false, empty, EDrawDebugTrace::None, hit, true))
		{
			return viewDirection;
		}
	}

	return boidInfo[index]->direction;
}

FVector ABoidManager::GetForceToDirection(FVector a_direction, int index)
{
	FVector direction = (a_direction.GetSafeNormal() * maxSpeed) - boidInfo[index]->velocity;
	return direction.GetClampedToMaxSize(maxForce);
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

void ABoidManager::RunFlockTask(int a_boidIndex, float a_deltaTime, TArray<FBoidInfo*> a_boidInfo, float a_viewRadius, float a_avoidRadius)
{
	(new FAutoDeleteAsyncTask<BoidWorker>(a_boidIndex, a_deltaTime, a_boidInfo, a_viewRadius, a_avoidRadius))->StartBackgroundTask();
}

void ABoidManager::RunFlockTaskOnMain(int a_boidIndex, float a_deltaTime, TArray<FBoidInfo*> a_boidInfo, float a_viewRadius, float a_avoidRadius)
{
	BoidWorker* task = new BoidWorker(a_boidIndex, a_deltaTime,  a_boidInfo, a_viewRadius, a_avoidRadius);

	task->DoWorkMain();

	delete task;
}

BoidWorker::BoidWorker(int a_boidIndex, float a_deltaTime, TArray<FBoidInfo*> a_boidInfo, float a_viewRadius, float a_avoidRadius)
{
	index = a_boidIndex;
	deltaTime = a_deltaTime;
	boidsInfo = a_boidInfo;
	viewRadius = a_viewRadius;
	avoidRadius = a_avoidRadius;
}

BoidWorker::~BoidWorker()
{
	//UE_LOG(LogTemp, Warning, TEXT("Task Finished."));
}

void BoidWorker::DoWork()
{
	boidsInfo[index]->numPerceivedFlockmates = 0;
	boidsInfo[index]->avgBoidDir = FVector(0.0f);
	boidsInfo[index]->avgAvoidDir = FVector(0.0f);
	boidsInfo[index]->centroid = FVector(0.0f);
	
	for (int i = 0; i < boidsInfo.Num(); i++)
	{
		FBoidInfo* otherBoid = boidsInfo[i];
	
		if (index != i && boidsInfo[index]->dimensionID == otherBoid->dimensionID)
		{
			FVector offset = otherBoid->transform.GetLocation() - boidsInfo[index]->transform.GetLocation();
			float sqrDst = offset.X * offset.X + offset.Y * offset.Y + offset.Z * offset.Z;
	
			if (sqrDst < viewRadius * viewRadius)
			{
				boidsInfo[index]->numPerceivedFlockmates++;
				boidsInfo[index]->avgBoidDir += otherBoid->direction;
				boidsInfo[index]->centroid += otherBoid->transform.GetLocation();
	
				if (sqrDst < avoidRadius * avoidRadius)
				{
					boidsInfo[index]->avgAvoidDir -= offset / sqrDst;
				}
			}
		}
	}
}

void BoidWorker::DoWorkMain()
{
	DoWork();
}



