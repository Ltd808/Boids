// Fill out your copyright notice in the Description page of Project Settings.


#include "OctantManager.h"

// Sets default values
AOctantManager::AOctantManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AOctantManager::BeginPlay()
{
	Super::BeginPlay();
	
	FTransform newTransform;
	FVector newPosition = FVector(0,0,0);
	FActorSpawnParameters spawnParams;

	newTransform.SetLocation(newPosition);

	root = GetWorld()->SpawnActor<AOctant>(AOctant::StaticClass(), newTransform, spawnParams);
}

// Called every frame
void AOctantManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	root->Display();
}

