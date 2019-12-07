// Fill out your copyright notice in the Description page of Project Settings.

#include "Boid.h"

void ABoid::Init(UStaticMesh* bodyRef)
{
	body->SetStaticMesh(bodyRef);
	body->SetWorldRotation(FRotator(0.0f, -90.0f, 90.0f).Quaternion());

	position = GetActorLocation();

	direction = GetActorForwardVector();

}

// Sets default values
ABoid::ABoid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	body = CreateDefaultSubobject<UStaticMeshComponent>("Body");
	RootComponent = body;

	numPerceivedFlockmates = 0;
}


// Called when the game starts or when spawned
void ABoid::BeginPlay()
{
	Super::BeginPlay();
	

}

// Called every frame
void ABoid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


