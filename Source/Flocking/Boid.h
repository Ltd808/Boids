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

	UPROPERTY(VisibleAnywhere, Category = "Boid Properties")
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

	void Init(UStaticMesh* bodyRef);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
