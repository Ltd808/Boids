// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

#include "Boid.generated.h"

UCLASS()
class FLOCKING_API ABoid : public AActor
{
	GENERATED_BODY()

public:	
	//Sets default values for this actor's properties
	ABoid();

	//body (cone) ref
	UStaticMeshComponent* body;

	//counts
	int numPerceivedFlockmates;
	int numViewDirections;

	//movement vectors
	FVector position = FVector(0.0f);
	FVector acceleration = FVector(0.0f);
	FVector velocity = FVector(0.0f);
	FVector direction = FVector(0.0f);

	//flock vectors
	FVector centroid = FVector(0.0f);
	FVector avgBoidDir = FVector(0.0f);
	FVector avgAvoidDir = FVector(0.0f);

	//view direction points
	TArray<FVector> points;

	//target actor
	AActor* target;

	//all boids
	TArray<AActor*> boids;

	//spheretrace channel
	TEnumAsByte<ETraceTypeQuery> traceChannel = ETraceTypeQuery::TraceTypeQuery3;

	//weights
	float avoidWeight = 10;
	float seperationWeight = 1;
	float cohesionWeight = 1;
	float alignmentWeight = 1;
	float targetWeight = 1;

	//caps
	float maxSpeed = 500.0f;
	float minSpeed = 200.0f;
	float maxForce = 300.0f;

	//awareness
	float boundsRadius = 25.0f;
	float collisionCheckDistance = 500.0f; 

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	virtual void Tick(float DeltaTime) override;

	//using this so we can init the static mesh at the right time
	void Init(UStaticMesh* bodyRef, TArray<AActor*> a_boids, AActor* a_target, float a_avoidWeight, float a_seperationWeight,
		float a_cohesionWeight, float a_alignmentWeight, float a_targetWeight, float a_maxSpeed, float a_minSpeed,
		float a_maxForce, float a_boundsRadius, float a_collisionCheckDistance, int a_numViewDirections, TEnumAsByte<ETraceTypeQuery> traceChannel, TArray<FVector> points);

	//spherecast ahead of object to check for collisions
	bool IsCloseToObject();

	//sphere cast along point array until safe direction is found
	FVector GetAvoidDir();

	//get force required to move in the specified direction
	FVector GetForceToDirection(FVector a_direction);

	void CalcPoints();
};
