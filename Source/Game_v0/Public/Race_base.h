// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "resource.h"
#include "Resourcenames.h"
#include "buildings/BuildingBase.h"
#include "Race_base.generated.h"


/**
 * 
 */
USTRUCT()
struct FPropertyMapWrap // wraper 
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FName, float> Properties; //property name -> prop value
};
USTRUCT(BlueprintType)
struct GAME_V0_API FBuildingCostEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	TSubclassOf<ABuildingBase> BuildingClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	TArray<FResource> ResourceCosts;

	FBuildingCostEntry()
	{
		BuildingClass = nullptr;
	}

	FBuildingCostEntry(TSubclassOf<ABuildingBase> InBuildingClass, const TArray<FResource>& InCosts)
		: BuildingClass(InBuildingClass), ResourceCosts(InCosts)
	{
	}
};
UCLASS()
class GAME_V0_API URace_base : public UObject
{
	GENERATED_BODY()
	public:
	URace_base(){};
	void InitBuildings(TArray<TSubclassOf<ABuildingBase>> &BuildingClasses);

	TArray<TSubclassOf<ABuildingBase>> GetAvailableBuildings(){return AvailableBuildings;};

	UFUNCTION(BlueprintCallable, Category = "Race")
	FName GetRaceName(){return RaceName;};
	
	UPROPERTY()
	TMap<EResourceKind, FPropertyMapWrap> InitialResourceProperties;

	UFUNCTION()
	void InitializeAllResources();

	UFUNCTION()
	TArray<FResource> GetAllResources() const {return AllResources;};

	// Get resources as FResource array from the maps (for PlayerState initialization)
	UFUNCTION()
	TArray<FResource> GetInitialResourcesArray() const;

	// Get a specific resource amount
	UFUNCTION(BlueprintCallable, Category = "Race|Resources")
	int32 GetInitialResourceAmount(EResourceKind ResourceKind) const;

	// Get a specific resource weight
	UFUNCTION(BlueprintCallable, Category = "Race|Resources")
	float GetInitialResourceWeight(EResourceKind ResourceKind) const;

	// Check if race has a specific resource type
	UFUNCTION(BlueprintCallable, Category = "Race|Resources")
	bool HasInitialResource(EResourceKind ResourceKind) const;

	// Get all resource types that this race starts with
	UFUNCTION(BlueprintCallable, Category = "Race|Resources")
	TArray<EResourceKind> GetInitialResourceTypes() const;

	UFUNCTION(BlueprintCallable, Category = "Race|Resources")
	TMap<FName, float> GetInitialResourceProperties(EResourceKind ResourceKind) const;

	UFUNCTION(BlueprintCallable, Category = "Race|Resources")
	void SetInitialResourceProperty(EResourceKind ResourceKind, FName PropertyName, float Value);

	UFUNCTION(BlueprintCallable, Category = "Race|Resources")
	float GetInitialResourceProperty(EResourceKind ResourceKind, FName PropertyName, float DefaultValue = 0.0f) const;

	UFUNCTION(BlueprintCallable, Category = "Race|Resources")
	bool HasInitialResourceProperty(EResourceKind ResourceKind, FName PropertyName) const;

	TArray<FResource> GetBuildingCost(const TSubclassOf<ABuildingBase>& BuildingClass) const;
	bool CanAffordBuilding(const TSubclassOf<ABuildingBase>& BuildingClass, const TArray<FResource>& PlayerResources) const;
	void SetBuildingCost(TSubclassOf<ABuildingBase> BuildingClass, const TArray<FResource>& Cost);
	


	protected:

	UPROPERTY(EditDefaultsOnly, Category="Race")
	FName RaceName;

	UPROPERTY(EditDefaultsOnly, Category="Race")
	TArray<TSubclassOf<ABuildingBase>> AvailableBuildings;

	UPROPERTY(EditDefaultsOnly, Category="Recource")
	TArray<FResource> AllResources;//Reserved don't modify
	
	static const TArray<EResourceKind> ResourcesList;

	// Race-specific initial amounts
	UPROPERTY(EditDefaultsOnly, Category="Race|Resources")
	TMap<EResourceKind, int32> InitialResourceAmounts;

	UPROPERTY(EditDefaultsOnly, Category="Race|Resources")
	TMap<EResourceKind, float> InitialResourceWeights;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Costs")
	TArray<FBuildingCostEntry> BuildingCosts;


};
