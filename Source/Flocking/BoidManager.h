// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Containers/EnumAsByte.h"
#include "Engine/World.h"
#include "Boid.h"
#include "DrawDebugHelpers.h"

#include "BoidManager.generated.h"

UCLASS()
class FLOCKING_API ABoidManager : public AActor
{
	GENERATED_BODY()
	
public:	

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		int boidCount = 300;
	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		int numViewDirections = 50;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		float maxSpeed = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		float minSpeed = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		float maxForce =  300.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		float viewRadius = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		float avoidRadius = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		float boundsRadius = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		float collisionCheckDistance = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		float avoidWeight = 100.0f;

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
		TEnumAsByte<ETraceTypeQuery> traceChannel = ETraceTypeQuery::TraceTypeQuery3;

	UPROPERTY(EditAnywhere, Category = "Boid Properties")
		UStaticMesh* bodyRef;

	TArray<ABoid*> boids;

	TArray<FVector> points;

	// Sets default values for this actor's properties
	ABoidManager();

	void CalcPoints();

	bool IsGoingToCollide(int index);

	FVector FindAvoidDirection(int index);

	FVector Move(FVector a_direction, int index);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
