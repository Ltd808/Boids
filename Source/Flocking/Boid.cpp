// Fill out your copyright notice in the Description page of Project Settings.
#include "Boid.h"

//using this so we can init the static mesh at the right time
void ABoid::Init(UStaticMesh* bodyRef)
{
	//set body
	body->SetStaticMesh(bodyRef);

	//set rotation to random
	//body->SetWorldRotation(FRotator(FMath::RandRange(-180, 180), FMath::RandRange(-180, 180), FMath::RandRange(-180, 180)).Quaternion());

	//store ac
	position = GetActorLocation();

	//set forward with new rotation
	direction = GetActorForwardVector();
}

// Sets default values
ABoid::ABoid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//setup body
	body = CreateDefaultSubobject<UStaticMeshComponent>("Body");

	//set body as root
	RootComponent = body;

	//starts at 0
	numPerceivedFlockmates = 0;
}

// Called when the game starts or when spawned
void ABoid::BeginPlay()
{
	Super::BeginPlay();
}




