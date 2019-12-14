// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "DrawDebugHelpers.h"

#include "BoidManager.generated.h"

USTRUCT()
struct FBoidInfo
{
	GENERATED_BODY()

	//counts
	UPROPERTY()
		int numPerceivedFlockmates = 0;

	UPROPERTY()
		int dimensionID = 0;

	UPROPERTY()
		int meshID;

	//movement vectors
	UPROPERTY()
		FTransform Transform;

	UPROPERTY()
		FVector Acceleration = FVector(0.0f);

	UPROPERTY()
		FVector Velocity = FVector(0.0f);

	UPROPERTY()
		FVector direction = FVector(0.0f);

	//flock vectors
	UPROPERTY()
		FVector Centroid = FVector(0.0f);

	UPROPERTY()
		FVector AvgBoidDir = FVector(0.0f);

	UPROPERTY()
		FVector AvgAvoidDir = FVector(0.0f);
};

class AOctant;
UCLASS()
class FLOCKING_API ABoidManager : public AActor
{
	GENERATED_BODY()
	
public:	

	//start
	UPROPERTY(EditAnywhere, Category = "Boid Start Properties")
		bool IsRunningOnMain = false;

	UPROPERTY(EditAnywhere, Category = "Boid Start Properties")
		bool IsSpatialPartitioningEnabled = true;

	UPROPERTY(EditAnywhere, Category = "Boid Start Properties")
		bool IsSpatialPartitioningDisplayOn = true;

	UPROPERTY(EditAnywhere, Category = "Boid Start Properties")
		int BoidCount = 600;

	UPROPERTY(EditAnywhere, Category = "Boid Start Properties")
		int ViewDirectionCount = 100;

	UPROPERTY(EditAnywhere, Category = "Boid Octree")
		int OctreeMaxLevel = 3;

	UPROPERTY(EditAnywhere, Category = "Boid Octree")
		int OctantIdealBoidCount = 20;

	UPROPERTY(EditAnywhere, Category = "Boid Octree")
		float OctreeTimerInterval = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Octree")
		FColor OctreeDisplayColor = FColor::Cyan;

	//movement
	UPROPERTY(EditAnywhere, Category = "Boid Movement")
		float MaxSpeed = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Movement")
		float minSpeed = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Movement")
		float MaxForce = 300.0f;

	//perception
	UPROPERTY(EditAnywhere, Category = "Boid Perception")
		float ViewRadius = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Perception")
		float AvoidRadius = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Perception")
		float boundsRadius = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Perception")
		float collisionCheckDistance = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Perception")
		AActor* target = nullptr;

	UPROPERTY(EditAnywhere, Category = "Boid Perception")
		TEnumAsByte<ETraceTypeQuery> traceChannel = ETraceTypeQuery::TraceTypeQuery3;

	//weights
	UPROPERTY(EditAnywhere, Category = "Boid Weights")
		float seperationWeight = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Weights")
		float cohesionWeight = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Weights")
		float alignmentWeight = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Weights")
		float AvoidWeight = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Weights")
		float targetWeight = 1.0f;

	//instance mesh
	UPROPERTY(EditAnywhere, Category = "Boid Mesh Instance")
		class UInstancedStaticMeshComponent* BoidInstancedMesh;

	//make privat
	FTimerHandle OctreeTimerHandle;

	TArray<FBoidInfo*> BoidInfo;

	TArray<FVector> Points;
	
	AOctant* RootOctant;

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	//set default values
	ABoidManager(const FObjectInitializer& ObjectInitializer);

	//Called every frame
	virtual void Tick(float DeltaTime) override;

	void CalcOctree();
	
	bool IsCloseToObject(int Index);

	FVector GetAvoidDir(int Index);

	//get force needed to move in the direction vecors direction 
	FVector GetForceToDirection(FVector Direction, int Index);

	//calculate view points using 3D golden spiral
	void CalcPoints();

	//create a new thread to calculate flock variables
	void RunFlockTask(int Index, float DeltaTime, TArray<FBoidInfo*> BoidInfo, float ViewRadius, float AvoidRadius);

	//calculate flock variables in game thread
	void RunFlockTaskOnMain(int Index, float DeltaTime, TArray<FBoidInfo*> BoidInfo, float ViewRadius, float AvoidRadius);
};

class BoidWorker : public FNonAbandonableTask
{
public:

	BoidWorker(int Index, float DeltaTime, TArray<FBoidInfo*> BoidInfo, float ViewRadius, float AvoidRadius);

	~BoidWorker();

	//requied by UE4
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(BoidWorker, STATGROUP_ThreadPoolAsyncTasks);
	}

	int Index;

	float DeltaTime;

	float ViewRadius;

	float AvoidRadius;

	TArray<FBoidInfo*> BoidInfo;

	void DoWork();

	void DoWorkMain();
};

