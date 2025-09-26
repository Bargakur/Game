// Fill out your copyright notice in the Description page of Project Settings.


#include "Chata.h"

// Sets default values
AChata::AChata()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	BuildingName = FString("Chata");

}

// Called when the game starts or when spawned
void AChata::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AChata::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

