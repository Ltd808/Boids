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
	UPROPERTY(EditAnywhere, Category = "Boid")
		int numPerceivedFlockmates = 0;
	UPROPERTY(EditAnywhere, Category = "Boid")
		int dimensionID = 0;

	UPROPERTY(EditAnywhere, Category = "Boid")
		int meshID;

	//movement vectors
	UPROPERTY(EditAnywhere, Category = "Boid")
		FTransform transform;
	UPROPERTY(EditAnywhere, Category = "Boid")
		FVector acceleration = FVector(0.0f);
	UPROPERTY(EditAnywhere, Category = "Boid")
		FVector velocity = FVector(0.0f);
	UPROPERTY(EditAnywhere, Category = "Boid")
		FVector direction = FVector(0.0f);


	//flock vectors
	UPROPERTY(EditAnywhere, Category = "Boid")
		FVector centroid = FVector(0.0f);
	UPROPERTY(EditAnywhere, Category = "Boid")
		FVector avgBoidDir = FVector(0.0f);
	UPROPERTY(EditAnywhere, Category = "Boid")
		FVector avgAvoidDir = FVector(0.0f);
};

class AOctant;
UCLASS()
class FLOCKING_API ABoidManager : public AActor
{
	GENERATED_BODY()
	
public:	

	//Sets default values for this actor's properties
	ABoidManager(const FObjectInitializer& ObjectInitializer);
	
	//classes

	TArray<FBoidInfo*> boidInfo;

	TArray<FVector> points;

	AOctant* myOctant;

	//start variables
	UPROPERTY(EditAnywhere, Category = "Boid Start Properties")
		bool IsRunningOnMain = false;

	UPROPERTY(EditAnywhere, Category = "Boid Start Properties")
		bool IsSpatialPartitioningEnabled = true;

	UPROPERTY(EditAnywhere, Category = "Boid Start Properties")
		bool IsSpatialPartitioningDisplayOn = true;

	UPROPERTY(EditAnywhere, Category = "Boid Start Properties")
		int boidCount = 600;

	UPROPERTY(EditAnywhere, Category = "Boid Start Properties")
		int numViewDirections = 100;

	UPROPERTY(EditAnywhere, Category = "Boid Start Properties")
		int maxOctreeLevel = 3;

	UPROPERTY(EditAnywhere, Category = "Boid Start Properties")
		int octreeIdealBoidCount = 20;

	UPROPERTY(EditAnywhere, Category = "Boid Start Properties")
		float octreeTimerInterval = 1.0f;
	
	UPROPERTY(EditAnywhere, Category = "Boid Movement")
		float maxSpeed = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Movement")
		float minSpeed = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Movement")
		float maxForce = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Perception")
		float viewRadius = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Perception")
		float avoidRadius = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Perception")
		float boundsRadius = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Perception")
		float collisionCheckDistance = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Perception")
		AActor* target = nullptr;

	UPROPERTY(EditAnywhere, Category = "Boid Perception")
		TEnumAsByte<ETraceTypeQuery> traceChannel = ETraceTypeQuery::TraceTypeQuery3;

	UPROPERTY(EditAnywhere, Category = "Boid Weights")
		float seperationWeight = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Weights")
		float cohesionWeight = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Weights")
		float alignmentWeight = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Weights")
		float avoidWeight = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Weights")
		float targetWeight = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Start Properties")
		FBoidInfo boidInfoRef;

	UPROPERTY(EditAnywhere, Category = "Boid Body")
		class UInstancedStaticMeshComponent* boids;

	FTimerHandle timerHandle;

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	//Called every frame
	virtual void Tick(float DeltaTime) override;

	void CalcOctree();
	
	bool IsCloseToObject(int index);

	FVector GetAvoidDir(int index);

	FVector GetForceToDirection(FVector a_direction, int index);

	void CalcPoints();

	//create a new thread to calculate flock variables
	void RunFlockTask(int boidIndex, float deltaTime, TArray<FBoidInfo*> boidsInfo, float viewRadius, float avoidRadius);

	//calculate flock variables in game thread
	void RunFlockTaskOnMain(int boidIndex, float deltaTime, TArray<FBoidInfo*> boidsInfo, float viewRadius, float avoidRadius);
};

class BoidWorker : public FNonAbandonableTask
{
public:

	BoidWorker(int boidIndex, float deltaTime, TArray<FBoidInfo*> boidsInfo, float viewRadius, float avoidRadius);

	~BoidWorker();

	//requied by UE4
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(BoidWorker, STATGROUP_ThreadPoolAsyncTasks);
	}

	int index;

	float deltaTime;

	float viewRadius;

	float avoidRadius;

	TArray<FBoidInfo*> boidsInfo;

	void DoWork();

	void DoWorkMain();
};

