// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "Octant.h"
#include "Boid.h"	
#include "DrawDebugHelpers.h"

#include "BoidManager.generated.h"

USTRUCT()
struct FBoidInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Boid Movement")
		float maxSpeed = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Movement")
		float minSpeed = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Movement")
		float maxForce = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Perception")
		float boundsRadius = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Perception")
		float viewRadius = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Perception")
		float avoidRadius = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Perception")
		float collisionCheckDistance = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Perception")
		AActor* target = nullptr;

	UPROPERTY(EditAnywhere, Category = "Boid Perception")
		TEnumAsByte<ETraceTypeQuery> traceChannel = ETraceTypeQuery::TraceTypeQuery3;

	UPROPERTY(EditAnywhere, Category = "Boid Weights")
		float seperationWeight = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Weights")
		float cohesionWeight = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Weights")
		float alignmentWeight = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Weights")
		float avoidWeight = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Weights")
		float targetWeight = 1.0f;

	//Constructor
	//FBoidInfo()
	//{
	//}
};

UCLASS()
class FLOCKING_API ABoidManager : public AActor
{
	GENERATED_BODY()
	
public:	

	//Sets default values for this actor's properties
	ABoidManager();

	//generated vars
	TArray<ABoid*> boids;
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
		int boidCount = 300;

	UPROPERTY(EditAnywhere, Category = "Boid Start Properties")
		int numViewDirections = 100;

	UPROPERTY(EditAnywhere, Category = "Boid Start Properties")
		int maxOctreeLevel = 3;

	UPROPERTY(EditAnywhere, Category = "Boid Start Properties")
		int octreeIdealBoidCount = 20;

	UPROPERTY(EditAnywhere, Category = "Boid Start Properties")
		float octreeTimerInterval = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Start Properties")
		UStaticMesh* bodyRef;

	UPROPERTY(EditAnywhere, Category = "Boid Start Properties")
		FBoidInfo boidInfo;

	FTimerHandle timerHandle;

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	//Called every frame
	virtual void Tick(float DeltaTime) override;

	void CalcOctree();
	
	void CalcPoints();

	//create a new thread to calculate flock variables
	void RunFlockTask(int boidIndex, float deltaTime, TArray<ABoid*> boids, TArray<FVector> points, FBoidInfo boidsInfo);

	//calculate flock variables in game thread
	void RunFlockTaskOnMain(int boidIndex, float deltaTime, TArray<ABoid*> boids, TArray<FVector> points, FBoidInfo boidsInfo);
};

class BoidWorker : public FNonAbandonableTask
{
public:

	BoidWorker(int boidIndex, float deltaTime, TArray<ABoid*> boids, TArray<FVector> points, FBoidInfo boidsInfo);

	~BoidWorker();

	//requied by UE4
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(BoidWorker, STATGROUP_ThreadPoolAsyncTasks);
	}

	int index;

	float deltaTime;

	TArray<ABoid*> boids;

	TArray<FVector> points;

	FBoidInfo boidInfo;

	void DoWork();

	void DoWorkMain();

	//spherecast ahead of object to check for collisions
	bool IsCloseToObject();

	//sphere cast along point array until safe direction is found
	FVector GetAvoidDir();

	//get force required to move in the specified direction
	FVector GetForceToDirection(FVector direction);
};

