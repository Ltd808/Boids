#include "BoidWorker.h"
//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
FBoidWorker* FBoidWorker::Runnable = NULL;
//***********************************************************

FBoidWorker::FBoidWorker(TArray<ABoid*>& a_boids, float a_ViewRadius, float a_AvoidRadius)
{
	//Link to where data should be stored
	boids = a_boids;
	viewRadius = a_ViewRadius;
	avoidRadius = a_AvoidRadius;
	isDone = false;

	Thread = FRunnableThread::Create(this, TEXT("FBoidWorker"), 0, TPri_BelowNormal); //windows default = 8mb for thread, could specify more
}

FBoidWorker::~FBoidWorker()
{
	delete Thread;
	Thread = NULL;
}

//Init
bool FBoidWorker::Init()
{
	return true;
}

//Run
uint32 FBoidWorker::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03);

	//compare to flock
	for (int index = 0; index < boids.Num(); index++)
	{
		boids[index]->numPerceivedFlockmates = 0;
		boids[index]->avgBoidDir = FVector(0);
		boids[index]->avgAvoidDir = FVector(0);
		boids[index]->centroid = FVector(0);

		for (int i = 0; i < boids.Num(); i++)
		{
			ABoid* currentBoid = boids[i];

			if (index != i)
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
	}
	//Run FPrimeNumberWorker::Shutdown() from the timer in Game Thread that is watching
		//to see when FPrimeNumberWorker::IsThreadFinished()

	return 0;
}

//stop
void FBoidWorker::Stop()
{
	StopTaskCounter.Increment();
}

FBoidWorker* FBoidWorker::JoyInit(TArray<ABoid*>& a_boids, float a_ViewRadius, float a_AvoidRadius)
{
	//Create new instance of thread if it does not exist
	//		and the platform supports multi threading!
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FBoidWorker(a_boids, a_ViewRadius, a_AvoidRadius);
	}
	return Runnable;
}

void FBoidWorker::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

void FBoidWorker::Shutdown()
{
	if (Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}

bool FBoidWorker::IsThreadFinished()
{
	if (Runnable) return Runnable->isDone;
	return true;
}
