// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Kismet/KismetMathLibrary.h"
#include "Containers/EnumAsByte.h"
#include "Engine/World.h"
#include "Boid.h"

#include "BoidManager.generated.h"

UCLASS()
class FLOCKING_API ABoidManager : public AActor
{
	GENERATED_BODY()
	
public:	

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		int boidCount = 300;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		int numViewDirections = 300;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		float maxSpeed = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		float minSpeed = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		float maxForce =  300.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		float boundsRadius = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		float collisionCheckDistance = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		float avoidWeight = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		float seperationWeight = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		float cohesionWeight = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		float alignmentWeight = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		float targetWeight = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		AActor* target;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		UStaticMesh* bodyRef;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		TEnumAsByte<ETraceTypeQuery> traceChannel = ETraceTypeQuery::TraceTypeQuery3;

	TArray<ABoid*> boids;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		float viewRadius = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		float avoidRadius = 200.0f;

	bool IsRunningOnMain = false;

	// Sets default values for this actor's properties
	ABoidManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void RunFlockTask(TArray<ABoid*> a_boids, int a_boidIndex, float a_viewRadius, float a_avoidRadius);

	UFUNCTION(BlueprintCallable)
	void RunFlockTaskOnMain(TArray<ABoid*> a_boids, int a_boidIndex, float a_viewRadius, float a_avoidRadius);
};

class CalcFlockTask : public FNonAbandonableTask
{
public:

	CalcFlockTask(TArray<ABoid*> boids, int boidIndex, float viewRadius, float avoidRadius);

	~CalcFlockTask();

	//requied by UE4
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(CalcFlockTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	TArray<ABoid*> boids;

	float viewRadius;

	float avoidRadius;

	int index;

	void DoWork();

	void DoWorkMain();
};
