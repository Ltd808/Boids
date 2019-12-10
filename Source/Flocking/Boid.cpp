// Fill out your copyright notice in the Description page of Project Settings.
#include "Boid.h"

//using this so we can init the static mesh at the right time
void ABoid::Init(UStaticMesh* bodyRef)
{
	//set body
	body->SetStaticMesh(bodyRef);

	//setup
	position = GetActorLocation();

	FQuat newRotation;

	SetActorRotation(newRotation.MakeFromEuler(FVector(FMath::RandRange(0, 360), FMath::RandRange(0, 360), FMath::RandRange(0, 360))));

	direction = GetActorForwardVector();

	//temp should change almost immediately
	velocity = direction;
}

// Sets default values
ABoid::ABoid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//setup body
	body = CreateDefaultSubobject<UStaticMeshComponent>("Body");

	//set body as root
	RootComponent = body;

	numPerceivedFlockmates = 0;
}

// Called when the game starts or when spawned
void ABoid::BeginPlay()
{
	Super::BeginPlay();
}

void ABoid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetActorLocation(position);
	SetActorRotation(UKismetMathLibrary::FindLookAtRotation(position, position + direction));
}

