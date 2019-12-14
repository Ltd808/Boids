//Fill out your copyright notice in the Description page of Project Settings
#include "BoidManager.h"
#include "Octant.h"
//Sets default values
ABoidManager::ABoidManager(const FObjectInitializer& ObjectInitializer)
{
	//Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoidInstancedMesh = ObjectInitializer.CreateDefaultSubobject<UInstancedStaticMeshComponent>(this, TEXT("Boifeefed"));
	RootComponent = BoidInstancedMesh;
}

//called when the game starts or when spawned
void ABoidManager::BeginPlay()
{
	Super::BeginPlay();

	CalcPoints();



	for (size_t i = 0; i < BoidCount; i++)
	{ 
		BoidInfo.Add(new FBoidInfo());

		BoidInfo[i]->Transform.SetLocation(FVector(FMath::RandRange(-500, 500), FMath::RandRange(-500, 500), FMath::RandRange(-500, 500)));

		BoidInfo[i]->meshID = BoidInstancedMesh->AddInstance(BoidInfo[i]->Transform);

		BoidInfo[i]->Velocity = FVector(FMath::RandRange(-minSpeed, MaxSpeed));
		BoidInfo[i]->direction = BoidInfo[i]->Velocity / BoidInfo[i]->Velocity.Size();
	}


	if (IsSpatialPartitioningEnabled)
	{
		FTransform newTransform;
		FActorSpawnParameters spawnParams;

		RootOctant = GetWorld()->SpawnActor<AOctant>(AOctant::StaticClass(), newTransform, spawnParams);

		RootOctant->InitRoot(OctreeMaxLevel, OctantIdealBoidCount, 3000, BoidInfo);

		GetWorld()->GetTimerManager().SetTimer(OctreeTimerHandle, this, &ABoidManager::CalcOctree, OctreeTimerInterval, true, 0.0f);
	}
}

void ABoidManager::CalcOctree()
{
	RootOctant->KillBranches();
	RootOctant->ConstructTree();
	RootOctant->AssignIDtoEntity();

	if (IsSpatialPartitioningDisplayOn) {
		RootOctant->Display(OctreeDisplayColor);
	}

}

//Called every frame
void ABoidManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (int i = 0; i < BoidCount; i++)
	{
		//create threads or run on main
		if (IsRunningOnMain)
		{
			RunFlockTaskOnMain(i, DeltaTime, BoidInfo, ViewRadius, AvoidRadius);
		}
		else
		{
			RunFlockTask(i, DeltaTime, BoidInfo, ViewRadius, AvoidRadius);
		}
	}

	for (int i = 0; i < BoidCount; i++)
	{
		BoidInfo[i]->Acceleration = FVector(0);

		//target seek force CAUSES MASSIVE FRAME DROPS DUE TO GET ACTOR LOCATION
		//if (boidInfo.target != nullptr) {
		//	FVector offsetToTarget = (boidInfo.target->GetActorLocation() - boids[index]->position); 
		//	boids[index]->acceleration = GetForceToDirection(offsetToTarget) * boidInfo.targetWeight;
		//}

		//flocking forces
		if (BoidInfo[i]->numPerceivedFlockmates != 0)
		{
			BoidInfo[i]->Centroid /= BoidInfo[i]->numPerceivedFlockmates;

			BoidInfo[i]->Acceleration += GetForceToDirection(BoidInfo[i]->AvgBoidDir, i) * alignmentWeight;
			BoidInfo[i]->Acceleration += GetForceToDirection(BoidInfo[i]->Centroid - BoidInfo[i]->Transform.GetLocation(), i) * cohesionWeight;
			BoidInfo[i]->Acceleration += GetForceToDirection(BoidInfo[i]->AvgAvoidDir, i) * seperationWeight;
		}

		//object avoidance
		if (IsCloseToObject(i))
		{
			FVector collisionAvoidForce = GetForceToDirection(GetAvoidDir(i), i) * AvoidWeight;
			BoidInfo[i]->Acceleration += collisionAvoidForce;
		}

		BoidInfo[i]->Velocity += BoidInfo[i]->Acceleration * DeltaTime;
		float speed = BoidInfo[i]->Velocity.Size();
		BoidInfo[i]->direction = BoidInfo[i]->Velocity / speed;
	    speed = FMath::Clamp(speed, minSpeed, MaxSpeed);
		BoidInfo[i]->Velocity = BoidInfo[i]->direction * speed;

		BoidInfo[i]->Transform.SetLocation(BoidInfo[i]->Transform.GetLocation() + BoidInfo[i]->Velocity * DeltaTime);
		BoidInfo[i]->Transform.SetRotation(UKismetMathLibrary::FindLookAtRotation(BoidInfo[i]->Transform.GetLocation(), BoidInfo[i]->Transform.GetLocation() + BoidInfo[i]->direction).Quaternion());

		BoidInstancedMesh->UpdateInstanceTransform(BoidInfo[i]->meshID, BoidInfo[i]->Transform);
	}

	//boids->ReleasePerInstanceRenderData();
	BoidInstancedMesh->MarkRenderStateDirty();
}

bool ABoidManager::IsCloseToObject(int index)
{
	//hit result
	FHitResult hit;

	TArray<AActor*> empty;

	if (UKismetSystemLibrary::SphereTraceSingle(GetWorld(), BoidInfo[index]->Transform.GetLocation(), BoidInfo[index]->Transform.GetLocation() + BoidInfo[index]->direction * collisionCheckDistance,
		boundsRadius, traceChannel, false, empty, EDrawDebugTrace::None, hit, true))
	{
		return true;
	}

	return false;
}

FVector ABoidManager::GetAvoidDir(int index)
{
	for (int i = 0; i < Points.Num(); i++)
	{
		//rotate the point torwards forward
		FVector viewDirection = UKismetMathLibrary::TransformDirection(BoidInfo[index]->Transform, Points[i]);

		FHitResult hit;

		TArray<AActor*> empty;

		if (!UKismetSystemLibrary::SphereTraceSingle(GetWorld(), BoidInfo[index]->Transform.GetLocation(), BoidInfo[index]->Transform.GetLocation() + viewDirection * collisionCheckDistance,
			boundsRadius, traceChannel, false, empty, EDrawDebugTrace::None, hit, true))
		{
			return viewDirection;
		}
	}

	return BoidInfo[index]->direction;
}

FVector ABoidManager::GetForceToDirection(FVector a_direction, int index)
{
	FVector direction = (a_direction.GetSafeNormal() * MaxSpeed) - BoidInfo[index]->Velocity;
	return direction.GetClampedToMaxSize(MaxForce);
}


//golden spiral method
void ABoidManager::CalcPoints()
{
	float goldenRatio = (1 + FMath::Sqrt(5)) / 2;
	float angleIncrement = PI * 2 * goldenRatio;

	for (int i = 0; i < ViewDirectionCount; i++)
	{
		float t = (float)i / ViewDirectionCount;
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

		Points.Add(viewDirection);
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
	Index = a_boidIndex;
	DeltaTime = a_deltaTime;
	BoidInfo = a_boidInfo;
	ViewRadius = a_viewRadius;
	AvoidRadius = a_avoidRadius;
}

BoidWorker::~BoidWorker()
{
	//UE_LOG(LogTemp, Warning, TEXT("Task Finished."));
}

void BoidWorker::DoWork()
{
	BoidInfo[Index]->numPerceivedFlockmates = 0;
	BoidInfo[Index]->AvgBoidDir = FVector(0.0f);
	BoidInfo[Index]->AvgAvoidDir = FVector(0.0f);
	BoidInfo[Index]->Centroid = FVector(0.0f);
	
	for (int i = 0; i < BoidInfo.Num(); i++)
	{
		FBoidInfo* otherBoid = BoidInfo[i];
	
		if (Index != i && BoidInfo[Index]->dimensionID == otherBoid->dimensionID)
		{
			FVector offset = otherBoid->Transform.GetLocation() - BoidInfo[Index]->Transform.GetLocation();
			float sqrDst = offset.X * offset.X + offset.Y * offset.Y + offset.Z * offset.Z;
	
			if (sqrDst < ViewRadius * ViewRadius)
			{
				BoidInfo[Index]->numPerceivedFlockmates++;
				BoidInfo[Index]->AvgBoidDir += otherBoid->direction;
				BoidInfo[Index]->Centroid += otherBoid->Transform.GetLocation();
	
				if (sqrDst < AvoidRadius * AvoidRadius)
				{
					BoidInfo[Index]->AvgAvoidDir -= offset / sqrDst;
				}
			}
		}
	}
}

void BoidWorker::DoWorkMain()
{
	DoWork();
}



