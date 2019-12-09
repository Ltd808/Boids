// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

#include "Boid.generated.h"

UCLASS()
class FLOCKING_API ABoid : public AActor
{
	GENERATED_BODY()

public:	

	UStaticMeshComponent* body;

	int numPerceivedFlockmates = 0;

	FVector position = FVector(0.0f);
	FVector acceleration = FVector(0.0f);
	FVector velocity = FVector(0.0f);
	FVector direction = FVector(0.0f);

	FVector centroid = FVector(0.0f);
	FVector avgBoidDir = FVector(0.0f);
	FVector avgAvoidDir = FVector(0.0f);

	TArray<FVector> points;

	AActor* target;

	TEnumAsByte<ETraceTypeQuery> traceChannel = ETraceTypeQuery::TraceTypeQuery3;

	float avoidWeight = 10.0f;

	float seperationWeight = 1.0f;

	float cohesionWeight = 1.0f;

	float alignmentWeight = 1.0f;

	float targetWeight = 1.0f;

	float maxSpeed = 500.0f;

	float minSpeed = 200.0f;

	float maxForce = 300.0f;

	int numViewDirections = 300;

	float boundsRadius = 25.0f;

	float collisionCheckDistance = 500.0f;

	// Sets default values for this actor's properties
	ABoid();

	TArray<AActor*> boids;

	//using this so we can init the static mesh at the right time
	void Init(UStaticMesh* bodyRef, TArray<AActor*> a_boids);

	bool IsCloseToObject();

	FVector GetAvoidDir();

	FVector GetForceToDirection(FVector a_direction);

	void CalcPoints();

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:
	virtual void Tick(float DeltaTime) override;
};
