// Fill out your copyright notice in the Description page of Project Settings.
#include "Boid.h"

//using this so we can init the static mesh at the right time
void ABoid::Init(UStaticMesh* bodyRef, TArray<AActor*> a_boids)
{
	//set body
	body->SetStaticMesh(bodyRef);

	boids = a_boids;

	//store ac
	position = GetActorLocation();

	FQuat newRotation;
	
	SetActorRotation(newRotation.MakeFromEuler(FVector(FMath::RandRange(-180, 180))));

	//set forward with new rotation
	direction = GetActorForwardVector();

	velocity = direction * maxSpeed;

	CalcPoints();
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

	//starts at 0
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

	acceleration = FVector(0);

	//target seek force
	if (target != nullptr) {
		FVector offsetToTarget = (target->GetActorLocation() - position);
		acceleration = GetForceToDirection(offsetToTarget) * targetWeight;
	}

	//flocking forces
	if (numPerceivedFlockmates != 0)
	{
		centroid /= numPerceivedFlockmates;

		acceleration += GetForceToDirection(avgBoidDir) * alignmentWeight;
		acceleration += GetForceToDirection(centroid - position) * cohesionWeight;
		acceleration += GetForceToDirection(avgAvoidDir) * seperationWeight;
	}

	//Draw debug lines for first 50 pts
	//for (size_t x = 0; x < 50; x++)
	//{
	//	//rotate torwards forward
	//	FTransform newTransform = GetTransform();
	//	FVector viewDirection = UKismetMathLibrary::TransformDirection(newTransform, points[x]);

	//	DrawDebugLine(GetWorld(), position, position + (viewDirection * collisionCheckDistance), FColor::Magenta, false, .1f);
	//}

	//object avoidance
	if (IsCloseToObject())
	{
		FVector collisionAvoidForce = GetForceToDirection(GetAvoidDir()) * avoidWeight;
		acceleration += collisionAvoidForce;
	}
	
	velocity += acceleration * DeltaTime;
	float speed = velocity.Size();
	direction = velocity / speed;
	speed = FMath::Clamp(speed, minSpeed, maxSpeed);
	velocity = direction * speed;
	position += velocity * DeltaTime;

	SetActorLocation(position);
	SetActorRotation(UKismetMathLibrary::FindLookAtRotation(position, position + direction));
}

bool ABoid::IsCloseToObject()
{
	//hit result
	FHitResult hit;

	TArray<AActor*> empty;

	if (UKismetSystemLibrary::SphereTraceSingle((UObject*)GetWorld(), position, position + direction * collisionCheckDistance, boundsRadius, traceChannel, false, boids, EDrawDebugTrace::None, hit, true))
	{
		return true;
	}

	return false;
}

FVector ABoid::GetAvoidDir()
{
	for (int i = 0; i < points.Num(); i++)
	{
		//rotate the point torwards forward
		FTransform newTransform = GetTransform();
		FVector viewDirection = UKismetMathLibrary::TransformDirection(newTransform, points[i]);

		FHitResult hit;

		TArray<AActor*> empty; 

		if (!UKismetSystemLibrary::SphereTraceSingle((UObject*)GetWorld(), position, position + viewDirection * collisionCheckDistance, boundsRadius, traceChannel, false, boids, EDrawDebugTrace::None, hit, true))
		{
			return viewDirection;
		}
	}

	return direction;
}

FVector ABoid::GetForceToDirection(FVector a_direction)
{
	FVector direction = (a_direction.GetSafeNormal() * maxSpeed) - velocity;
	return direction.GetClampedToMaxSize(maxForce);
}

//golden spiral method
void ABoid::CalcPoints()
{
	float goldenRatio = (1 + FMath::Sqrt(5)) / 2;
	float angleIncrement = PI * 2 * goldenRatio;

	for (int i = 0; i < numViewDirections; i++)
	{
		float t = (float)i / numViewDirections;
		float inclination = FMath::Acos(1 - 2 * t);
		float azimuth = angleIncrement * i;

		float x = FMath::Sin(inclination) * FMath::Cos(azimuth);
		float y = FMath::Sin(inclination) * FMath::Sin(azimuth);
		float z = FMath::Cos(inclination);

		//rotate torward forward like boid cone
		FTransform newTransform;
		FQuat deltaRotate;
		newTransform.SetRotation(newTransform.GetRotation() * deltaRotate.MakeFromEuler(FVector(0, -90, 0)));
		FVector viewDirection = UKismetMathLibrary::TransformDirection(newTransform, FVector(x, y, z));

		points.Add(viewDirection);
	}
}




