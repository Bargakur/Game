// Fill out your copyright notice in the Description page of Project Settings.


#include "Race_base.h"

void URace_base::InitBuildings(TArray<TSubclassOf<ABuildingBase>> &BuildingClasses)
{
	AvailableBuildings = BuildingClasses;
}
void URace_base::InitializeAllResources()
{
	AllResources.Empty();
    
	// Create FResource instances for each resource type
	for (const auto& ResourcePair : InitialResourceAmounts)
	{
		EResourceKind ResourceKind = ResourcePair.Key;
		int32 Amount = ResourcePair.Value;
        
		// Get weight for this resource type
		float Weight = 1.0f;
		if (const float* WeightPtr = InitialResourceWeights.Find(ResourceKind))
		{
			Weight = *WeightPtr;
		}
        
		// Convert enum to name
		FName ResourceName = UGameResources::ResourceKindToName(ResourceKind);
        
		// Create and add the resource
		FResource NewResource(ResourceName, Amount, Weight);
        
		// Add properties if they exist for this resource type
		if (const FPropertyMapWrap* PropertiesWrapPtr = InitialResourceProperties.Find(ResourceKind))
		{
			NewResource.ResourceProperties = PropertiesWrapPtr->Properties;
			UE_LOG(LogTemp, Log, TEXT("Initialized starting resource %s with %d properties"), 
				   *ResourceName.ToString(), PropertiesWrapPtr->Properties.Num());
		}
        
		AllResources.Add(NewResource);
	}
}

   TArray<FResource> URace_base::GetInitialResourcesArray() const
{
    TArray<FResource> ResourceArray;
    
    UE_LOG(LogTemp, Log, TEXT("GetInitialResourcesArray called for race: %s"), *RaceName.ToString());
    UE_LOG(LogTemp, Log, TEXT("InitialResourceAmounts has %d entries"), InitialResourceAmounts.Num());
    
    // Create FResource instances for each resource type
    for (const auto& ResourcePair : InitialResourceAmounts)
    {
        EResourceKind ResourceKind = ResourcePair.Key;
        int32 Amount = ResourcePair.Value;
        
        // Get weight for this resource type
        float Weight = 1.0f; // Default weight
        if (const float* WeightPtr = InitialResourceWeights.Find(ResourceKind))
        {
            Weight = *WeightPtr;
        }
        
        // Convert enum to name using your utility function
        FName ResourceName = UGameResources::ResourceKindToName(ResourceKind);
        UE_LOG(LogTemp, Log, TEXT("Converting resource kind %d to name: %s"), (int32)ResourceKind, *ResourceName.ToString());
        
        // Create the resource
        FResource NewResource(ResourceName, Amount, Weight);
        
        // Add properties if they exist for this resource type
        if (const FPropertyMapWrap* PropertiesWrapPtr = InitialResourceProperties.Find(ResourceKind))
        {
            NewResource.ResourceProperties = PropertiesWrapPtr->Properties;
            UE_LOG(LogTemp, Log, TEXT("Added %d properties to resource: %s"), PropertiesWrapPtr->Properties.Num(), *ResourceName.ToString());
            
            // Log each property for debugging
            for (const auto& PropertyPair : PropertiesWrapPtr->Properties)
            {
                UE_LOG(LogTemp, Log, TEXT(" Property: %s = %.2f"), *PropertyPair.Key.ToString(), PropertyPair.Value);
            }
        }
        
        ResourceArray.Add(NewResource);
        
        UE_LOG(LogTemp, Log, TEXT("Added initial resource: %s, Amount: %d, Weight: %f, Properties: %d"), 
               *NewResource.ResourceName.ToString(), NewResource.ResourceAmount, NewResource.Weight, NewResource.ResourceProperties.Num());
    }
    
    UE_LOG(LogTemp, Log, TEXT("GetInitialResourcesArray completed. Total resources: %d"), ResourceArray.Num());
    return ResourceArray;

}

int32 URace_base::GetInitialResourceAmount(EResourceKind ResourceKind) const
{
    if (const int32* Amount = InitialResourceAmounts.Find(ResourceKind))
    {
        return *Amount;
    }
    return 0;
}

float URace_base::GetInitialResourceWeight(EResourceKind ResourceKind) const
{
    if (const float* Weight = InitialResourceWeights.Find(ResourceKind))
    {
        return *Weight;
    }
    return 1.0f; // Default weight
}

bool URace_base::HasInitialResource(EResourceKind ResourceKind) const
{
    return InitialResourceAmounts.Contains(ResourceKind);
}

TArray<EResourceKind> URace_base::GetInitialResourceTypes() const
{
    TArray<EResourceKind> ResourceTypes;
    InitialResourceAmounts.GetKeys(ResourceTypes);
    return ResourceTypes;
}

TMap<FName, float> URace_base::GetInitialResourceProperties(EResourceKind ResourceKind) const
{
	if (const FPropertyMapWrap* PropertiesWrapPtr = InitialResourceProperties.Find(ResourceKind))
	{
		return PropertiesWrapPtr->Properties;
	}
	return TMap<FName, float>();
}
void URace_base::SetInitialResourceProperty(EResourceKind ResourceKind, FName PropertyName, float Value)
{
	FPropertyMapWrap& PropertiesWrap = InitialResourceProperties.FindOrAdd(ResourceKind);
	PropertiesWrap.Properties.Add(PropertyName, Value);
}
bool URace_base::HasInitialResourceProperty(EResourceKind ResourceKind, FName PropertyName) const
{
	if (const FPropertyMapWrap* PropertiesWrapPtr = InitialResourceProperties.Find(ResourceKind))
	{
		return PropertiesWrapPtr->Properties.Contains(PropertyName);
	}
	return false;
}
float URace_base::GetInitialResourceProperty(EResourceKind ResourceKind, FName PropertyName, float DefaultValue) const
{
	if (const FPropertyMapWrap* PropertiesWrapPtr = InitialResourceProperties.Find(ResourceKind))
	{
		if (const float* ValuePtr = PropertiesWrapPtr->Properties.Find(PropertyName))
		{
			return *ValuePtr;
		}
	}
	return DefaultValue;
}
TArray<FResource> URace_base::GetBuildingCost(const TSubclassOf<ABuildingBase>& BuildingClass) const
{
	UE_LOG(LogTemp, Warning, TEXT("GetBuildingCost called for: %s"), BuildingClass ? *BuildingClass->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("BuildingCosts array has %d entries"), BuildingCosts.Num());
    
	for (const FBuildingCostEntry& Entry : BuildingCosts)
	{
		UE_LOG(LogTemp, Warning, TEXT("Checking entry for class: %s"), Entry.BuildingClass ? *Entry.BuildingClass->GetName() : TEXT("NULL"));
		if (Entry.BuildingClass == BuildingClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("Found cost entry with %d resources"), Entry.ResourceCosts.Num());
			return Entry.ResourceCosts;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("No cost entry found for building class"));
	return TArray<FResource>();
}

bool URace_base::CanAffordBuilding(const TSubclassOf<ABuildingBase>& BuildingClass, const TArray<FResource>& PlayerResources) const
{
	TArray<FResource> Cost = GetBuildingCost(BuildingClass);
    
	for (const FResource& RequiredResource : Cost)
	{
		int32 PlayerAmount = 0;
		for (const FResource& PlayerResource : PlayerResources)
		{
			if (PlayerResource.ResourceName == RequiredResource.ResourceName)
			{
				PlayerAmount += PlayerResource.ResourceAmount;
			}
		}
        
		if (PlayerAmount < RequiredResource.ResourceAmount)
		{
			return false;
		}
	}
	return true;
}

void URace_base::SetBuildingCost(TSubclassOf<ABuildingBase> BuildingClass, const TArray<FResource>& Cost)
{
	// Find existing entry or add new one
	for (FBuildingCostEntry& Entry : BuildingCosts)
	{
		if (Entry.BuildingClass == BuildingClass)
		{
			Entry.ResourceCosts = Cost;
			return;
		}
	}
    
	// Add new entry if not found
	BuildingCosts.Add(FBuildingCostEntry(BuildingClass, Cost));
}