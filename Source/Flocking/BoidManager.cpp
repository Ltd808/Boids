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

	for (int i = 0; i < boids.Num(); i++)
	{
		//compare to flock
		boids[i]->numPerceivedFlockmates = 0;
		boids[i]->avgBoidDir = FVector(0);
		boids[i]->avgAvoidDir = FVector(0);
		boids[i]->centroid = FVector(0);

		for (ABoid* boid : boids)
		{
			ABoid* currentBoid = boid;

			if (boids[i] != currentBoid)
			{
				FVector offset = currentBoid->position - boids[i]->position;
				float sqrDst = offset.X * offset.X + offset.Y * offset.Y + offset.Z * offset.Z;

				if (sqrDst < viewRadius * viewRadius)
				{
					boids[i]->numPerceivedFlockmates++;
					boids[i]->avgBoidDir += currentBoid->direction;
					boids[i]->centroid += currentBoid->position;

					if (sqrDst < avoidRadius * avoidRadius)
					{
						boids[i]->avgAvoidDir -= offset / sqrDst;
					}
				}
			}
		}

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
		//	//rotate points
		//	FTransform newTransform = boids[i]->GetTransform();

		//	//offset rotation of the cone
		//	FQuat deltaRotate;

		//	//rotate current rotation and set
		//	FQuat finalRotation = newTransform.GetRotation() * deltaRotate.MakeFromEuler(FVector(0, -90, 0));
		//	newTransform.SetRotation(finalRotation);

		//	DrawDebugLine(GetWorld(), boids[i]->position, boids[i]->position + (UKismetMathLibrary::TransformDirection(newTransform, points[x]) * collisionCheckDistance), FColor::Magenta, false, .1f);
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
		//rotate points
		FTransform newTransform = boids[i]->GetTransform();

		//offset rotation of the cone
		FQuat deltaRotate;

		//rotate current rotation and set
		FQuat finalRotation = newTransform.GetRotation() * deltaRotate.MakeFromEuler(FVector(0, -90, 0));
		newTransform.SetRotation(finalRotation);

		FVector viewDirection = UKismetMathLibrary::TransformDirection(newTransform, points[i]);
		
		//hit result
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
		points.Add(FVector(x, y, z));
	}
}




