// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/StaticMeshComponent.h"

#include "Boid.generated.h"

UCLASS()
class FLOCKING_API ABoid : public AActor
{
	GENERATED_BODY()

public:	

	UStaticMeshComponent* body;

	int numPerceivedFlockmates;

	FVector position;
	FVector acceleration;
	FVector velocity;
	FVector direction;

	FVector centroid;
	FVector avgBoidDir;
	FVector avgAvoidDir;

	// Sets default values for this actor's properties
	ABoid();

	//using this so we can init the static mesh at the right time
	void Init(UStaticMesh* bodyRef);

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
