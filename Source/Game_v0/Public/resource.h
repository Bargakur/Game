#pragma once

#include "CoreMinimal.h"
#include "resource.generated.h"
/*
 *Resource names are stored in Resourcenames.h enumclass when creating resource make sure to add name, weight and optionally properties
 *
 *
 */
class APhysicalResourceActor;

UENUM(BlueprintType)
enum class EResourceLocation : uint8
{
	InPlayerInventory, // Abstract/logical state (current default)
	OnGround,         // Physical world location
	CarriedByUnit,    // Carried by a unit
	StoredInBuilding  // Stored in a building
};

USTRUCT(BlueprintType)
struct GAME_V0_API FResource
{
	GENERATED_BODY()
	

	UPROPERTY(VisibleAnywhere, Category="Resource")
	FName ResourceName;

	UPROPERTY()
	TMap<FName, float> ResourceProperties;

	
	FResource()
			: ResourceName(NAME_None), ResourceAmount(0), Weight(0.f)
	{}
	FResource(const FName ResourceName, const int32 initial_amount, const float initial_weight );
	FResource(const FResource& Other)
		: ResourceName(Other.ResourceName), ResourceAmount(Other.ResourceAmount), Weight(Other.Weight)
	{}
	// Helper function to check if this is a valid resource
	bool IsValid() const { return ResourceName != NAME_None; }

		

		UPROPERTY(VisibleInstanceOnly, Category = "Resource")
		int32 ResourceAmount;
		UPROPERTY(VisibleAnywhere, Category = "Resource")//Literally a weight/mass of a recource
		float Weight;
	// Equality operator - compares name and weight only
	bool operator==(const FResource& Other) const
	{
		return ResourceName == Other.ResourceName && 
			   FMath::IsNearlyEqual(Weight, Other.Weight);
	}

	// Addition operator - adds amounts
	FResource operator+(const FResource& Other) const
	{
		FResource Result(*this);
		if (*this == Other)
		{
			Result.ResourceAmount += Other.ResourceAmount;
		}
		return Result;
	}


	FResource operator-(const FResource& Other) const
	{
		FResource Result(*this);
		if (*this == Other)
		{
			Result.ResourceAmount -= Other.ResourceAmount;
		}
		return Result;
	}


	FResource& operator+=(const FResource& Other)
	{
		if (*this == Other)
		{
			ResourceAmount += Other.ResourceAmount;
		}
		return *this;
	}


	FResource& operator-=(const FResource& Other)
	{
		if (*this == Other)
		{
			ResourceAmount -= Other.ResourceAmount;
		}
		return *this;
	}
	void SetProperty(FName PropertyName, float Value)
	{
		ResourceProperties.Add(PropertyName, Value);
	}
	float GetProperty(FName PropertyName, float DefaultValue = 0.0f) const
	{
		if (const float* Value = ResourceProperties.Find(PropertyName))
		{
			return *Value;
		}
		return DefaultValue;
	}
	float GetPropertySafe(FName PropertyName, float DefaultValue = 0.0f) const 
	{
		if (const float* Value = ResourceProperties.Find(PropertyName))
		{
			return *Value;
		}
    
		UE_LOG(LogTemp, Warning, TEXT("Resource '%s': Property '%s' not found, returning default value %.2f"), 
			   *ResourceName.ToString(), *PropertyName.ToString(), DefaultValue);
    
		return DefaultValue;
	}
	bool HasProperty(FName PropertyName) const
	{
		return ResourceProperties.Contains(PropertyName);
	}
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	bool bIsWeapon = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physical")
	EResourceLocation LocationType = EResourceLocation::InPlayerInventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physical")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physical")
	TWeakObjectPtr<AActor> ContainerActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physical")
	TWeakObjectPtr<APhysicalResourceActor> PhysicalActor = nullptr;

	// Physical state management
	void SetOnGround(FVector Location);
	void SetCarriedBy(AActor* CarryingActor);
	void SetInPlayerInventory();
    
	bool IsPhysicallyPresent() const { return LocationType != EResourceLocation::InPlayerInventory; }
	bool CanBePickedUp() const { return LocationType == EResourceLocation::OnGround; }
};
