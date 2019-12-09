// Fill out your copyright notice in the Description page of Project Settings.
#include "Boid.h"

//using this so we can init the static mesh at the right time
void ABoid::Init(UStaticMesh* bodyRef, TArray<AActor*> a_boids, AActor* a_target, float a_avoidWeight, float a_seperationWeight, 
					float a_cohesionWeight, float a_alignmentWeight, float a_targetWeight, float a_maxSpeed, float a_minSpeed, 
					float a_maxForce, float a_boundsRadius, float a_collisionCheckDistance, int a_numViewDirections, TEnumAsByte<ETraceTypeQuery> a_traceChannel, TArray<FVector> a_points)
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

	AActor* target = a_target;

	TEnumAsByte<ETraceTypeQuery> traceChannel = a_traceChannel;

	target = a_target;

	avoidWeight = a_avoidWeight;

	seperationWeight = a_seperationWeight;

	cohesionWeight = a_cohesionWeight;

	alignmentWeight = a_alignmentWeight;

	targetWeight = a_targetWeight;

	maxSpeed = a_maxSpeed;

	minSpeed = a_minSpeed;

	maxForce = a_maxForce;

	boundsRadius = a_boundsRadius;

	collisionCheckDistance = a_collisionCheckDistance;

	numViewDirections = a_numViewDirections;

	points = a_points;
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




