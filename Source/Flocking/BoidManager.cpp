// Fill out your copyright notice in the Description page of Project Settings
#include "BoidManager.h"

// Sets default values
ABoidManager::ABoidManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABoidManager::BeginPlay()
{
	Super::BeginPlay();

	CalcPoints();

	for (int i = 0; i < boidCount; i++)
	{
		FTransform newTransform;
		FVector newPosition = FVector(FMath::RandRange(-1000, 1000), FMath::RandRange(-1000, 1000), FMath::RandRange(-1000, 1000));
		FActorSpawnParameters spawnParams;

		newTransform.SetLocation(newPosition);

		boids.Add(GetWorld()->SpawnActor<ABoid>(ABoid::StaticClass(), newTransform, spawnParams));

		boids[i]->Init(bodyRef);

		boids[i]->velocity = boids[i]->direction * maxSpeed;
	}
}

// Called every frame
void ABoidManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//do this in parallel
	ParallelFor(boids.Num(), [&](int index)
	{
		//compare to flock
		boids[index]->numPerceivedFlockmates = 0;
		boids[index]->avgBoidDir = FVector(0);
		boids[index]->avgAvoidDir = FVector(0);
		boids[index]->centroid = FVector(0);

		for (int i = 0; i < boids.Num(); i++)
		{
			ABoid* currentBoid = boids[i];

			if (boids[index] != currentBoid)
			{
				FVector offset = currentBoid->position - boids[index]->position;
				float sqrDst = offset.X * offset.X + offset.Y * offset.Y + offset.Z * offset.Z;

				if (sqrDst < viewRadius * viewRadius)
				{
					boids[index]->numPerceivedFlockmates++;
					boids[index]->avgBoidDir += currentBoid->direction;
					boids[index]->centroid += currentBoid->position;

					if (sqrDst < avoidRadius * avoidRadius)
					{
						boids[index]->avgAvoidDir -= offset / sqrDst;
					}
				}
			}
		}
	}, false);

	for (int i = 0; i < boids.Num(); i++)
	{
		//individual boids 
		boids[i]->acceleration = FVector(0);

		if (target != nullptr) {
			FVector offsetToTarget = (target->GetActorLocation() - boids[i]->position);
			boids[i]->acceleration = Move(offsetToTarget, i) * targetWeight;
		}

		if (boids[i]->numPerceivedFlockmates != 0)
		{
			boids[i]->centroid /= boids[i]->numPerceivedFlockmates;

			FVector offsetToFlockmatesCentre = (boids[i]->centroid - boids[i]->position);

			boids[i]->acceleration += Move(boids[i]->avgBoidDir, i) * alignmentWeight;
			boids[i]->acceleration += Move(offsetToFlockmatesCentre, i) * cohesionWeight;
			boids[i]->acceleration += Move(boids[i]->avgAvoidDir, i) * seperationWeight;
		}

		//Draw debug lines for first 50 pts
		//for (size_t x = 0; x < 50; x++)
		//{
		//	//rotate torwards forward
		//	FTransform newTransform = boids[i]->GetTransform();
		//	FVector viewDirection = UKismetMathLibrary::TransformDirection(newTransform, points[x]);

		//	DrawDebugLine(GetWorld(), boids[i]->position, boids[i]->position + (viewDirection * collisionCheckDistance), FColor::Magenta, false, .1f);
		//}

		if (IsCloseToObject(i))
		{
			FVector collisionAvoidForce = Move(GetAvoidDir(i), i) * avoidWeight;
			boids[i]->acceleration += collisionAvoidForce;
		}

		boids[i]->velocity += boids[i]->acceleration * DeltaTime;
		float speed = boids[i]->velocity.Size();
		boids[i]->direction = boids[i]->velocity / speed;
		speed = FMath::Clamp(speed, minSpeed, maxSpeed);
		boids[i]->velocity = boids[i]->direction * speed;
		boids[i]->SetActorLocation(boids[i]->GetActorLocation() + boids[i]->velocity * DeltaTime);

		boids[i]->SetActorRotation(UKismetMathLibrary::RInterpTo(boids[i]->GetActorRotation(), UKismetMathLibrary::FindLookAtRotation(boids[i]->position, boids[i]->position + boids[i]->direction), DeltaTime, 5).Quaternion());
		boids[i]->position = boids[i]->GetActorLocation();
	}
}

bool ABoidManager::IsCloseToObject(int index)
{
	//hit result
	FHitResult hit;

	if (UKismetSystemLibrary::SphereTraceSingle((UObject*)GetWorld(), boids[index]->position, boids[index]->position + boids[index]->direction * collisionCheckDistance, boundsRadius, traceChannel, false, (TArray<AActor*>)boids, EDrawDebugTrace::None, hit, true))
	{
		return true;
	}

	return false;
}

FVector ABoidManager::GetAvoidDir(int index)
{
	for (int i = 0; i < points.Num(); i++) 
	{
		//rotate the point torwards forward
		FTransform newTransform = boids[index]->GetTransform();
		FVector viewDirection = UKismetMathLibrary::TransformDirection(newTransform, points[i]);

		FHitResult hit;

		if (!UKismetSystemLibrary::SphereTraceSingle((UObject*)GetWorld(), boids[index]->position, boids[index]->position + (viewDirection * collisionCheckDistance), boundsRadius, traceChannel, false, (TArray<AActor*>)boids, EDrawDebugTrace::None, hit, true))
		{
			return viewDirection;
		}
	}

	return boids[index]->direction;
}

FVector ABoidManager::Move(FVector a_direction, int index)
{
	FVector direction = (a_direction.GetSafeNormal() * maxSpeed) - boids[index]->velocity;
	return direction.GetClampedToMaxSize(maxForce);
}

//golden spiral method
void ABoidManager::CalcPoints()
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
		FVector viewDirection = UKismetMathLibrary::TransformDirection(newTransform, FVector(x,y,z));

		points.Add(viewDirection);
	}
}




