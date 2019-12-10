// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/StaticMeshComponent.h"
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

	//movement vectors
	FVector position = FVector(0.0f);
	FVector acceleration = FVector(0.0f);
	FVector velocity = FVector(0.0f);
	FVector direction = FVector(0.0f);

	//flock vectors
	FVector centroid = FVector(0.0f);
	FVector avgBoidDir = FVector(0.0f);
	FVector avgAvoidDir = FVector(0.0f);

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	virtual void Tick(float DeltaTime) override;

	//using this so we can init the static mesh at the right time
	void Init(UStaticMesh* bodyRef);
};
