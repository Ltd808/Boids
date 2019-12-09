// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Boid.h"

//~~~~~ Multi Threading ~~~
class FLOCKING_API FBoidWorker : public FRunnable
{
	/** Singleton instance, can access the thread any time via static accessor, if it is active! */
	static  FBoidWorker* Runnable;

	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;

	/** The Data Ptr */
	TArray<ABoid*> boids;

	float viewRadius;

	float avoidRadius;

	bool isDone;

	/** Stop this thread? Uses Thread Safe Counter */
	FThreadSafeCounter StopTaskCounter;

private:

public:
	//~~~ Thread Core Functions ~~~

	//Constructor / Destructor
	FBoidWorker(TArray<ABoid*>& boidsInfo, float a_ViewRadius, float a_AvoidRadius);
	virtual ~FBoidWorker();

	// Begin FRunnable interface.
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	// End FRunnable interface

	/** Makes sure this thread has stopped properly */
	void EnsureCompletion();

	//~~~ Starting and Stopping Thread ~~~

	/*
		Start the thread and the worker from static (easy access)!
		This code ensures only 1 Prime Number thread will be able to run at a time.
		This function returns a handle to the newly started instance.
	*/
	static FBoidWorker* JoyInit(TArray<ABoid*>& boids, float a_ViewRadius, float a_AvoidRadius);

	/** Shuts down the thread. Static so it can easily be called from outside the thread context */
	static void Shutdown();

	static bool IsThreadFinished();

};