// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ToroidGameMode.generated.h"

/**
 * Similar to BuildingGameModeDemo but with toroidal geometry
 */
USTRUCT(BlueprintType)
struct FCoreSpawnPoint_T
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Location;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FRotator Rotation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 TeamID;

	FCoreSpawnPoint_T()
		: Location(FVector::ZeroVector), Rotation(FRotator::ZeroRotator), TeamID(0)
	{
	}

	FCoreSpawnPoint_T(FVector InLocation, FRotator InRotation, int32 InTeamID)
		: Location(InLocation), Rotation(InRotation), TeamID(InTeamID)
	{
	}
};

UCLASS()
class GAME_V0_API AToroidGameMode : public AGameMode
{
	GENERATED_BODY()
};
