// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnitBase.h"
#include "ElfUnit.generated.h"

/**
 * Specialized elf unit that selects mesh based on sex
 */
UCLASS()
class GAME_V0_API AElfUnit : public AUnitBase
{
	GENERATED_BODY()

public:
	AElfUnit();


	// Female mesh override
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elf Appearance", meta = (AllowedClasses = "SkeletalMesh"))
	TSoftObjectPtr<USkeletalMesh> FemaleElfMesh;

	// Female animation blueprint override
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elf Appearance", meta = (AllowedClasses = "AnimBlueprint"))
	TSoftClassPtr<UAnimInstance> FemaleAnimBP;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	// Select appropriate mesh based on sex
	TSoftObjectPtr<USkeletalMesh> SelectElfMesh() const;

	// Select appropriate animation blueprint based on sex
	TSoftClassPtr<UAnimInstance> SelectElfAnimBP() const;

	// Override mesh loading to use sex-based selection
	void LoadElfMesh();

	// Load appropriate animation blueprint
	void LoadElfAnimBP();



};