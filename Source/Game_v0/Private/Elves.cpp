#include "Elves.h"

#include "buildings/BigPotionWorkshop_Elves.h"
#include"buildings/BlacksmithWorkshop_Elves.h"


UElves::UElves()
{
	RaceName = FName("Elves");

	// init available buildings
	AvailableBuildings.Add(AChata::StaticClass());
	AvailableBuildings.Add(ABigPotionWorkshop_Elves::StaticClass());
	AvailableBuildings.Add(ABlacksmithWorkshop_Elves::StaticClass());

	// init resources
	InitialResourceAmounts.Add(EResourceKind::water_small, 25);
	InitialResourceAmounts.Add(EResourceKind::wood, 25);
	InitialResourceAmounts.Add(EResourceKind::Elvish_bow, 10);
	InitialResourceAmounts.Add(EResourceKind::Sword, 10);

	InitialResourceWeights.Add(EResourceKind::water_small, 1.5);
	InitialResourceWeights.Add(EResourceKind::wood, 1.5);
	InitialResourceWeights.Add(EResourceKind::Elvish_bow, 0.5);
	InitialResourceWeights.Add(EResourceKind::Sword, 3.5);
    
	FPropertyMapWrap SwordProperties;
	SwordProperties.Properties.Add(FName("Damage"), 5.0f);
	SwordProperties.Properties.Add(FName("Range"), 1.0f);
	InitialResourceProperties.Add(EResourceKind::Sword, SwordProperties);
	
	// Initialize building costs
	TArray<FResource> PotionWorkshopCost;
	PotionWorkshopCost.Add(FResource(UGameResources::ResourceKindToName(EResourceKind::wood), 10, 2.5f));//weight doesn't matter
    
	BuildingCosts.Add(FBuildingCostEntry(ABigPotionWorkshop_Elves::StaticClass(), PotionWorkshopCost));

	UE_LOG(LogTemp, Log, TEXT("Elves race initialized with %d resource types"), 
		  InitialResourceAmounts.Num());
}

	
	

