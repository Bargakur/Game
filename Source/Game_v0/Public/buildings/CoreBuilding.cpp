


#include "CoreBuilding.h"
#include "BGameState.h"


ACoreBuilding::ACoreBuilding()
{
	BuildingName = TEXT("Core Building");


	static ConstructorHelpers::FObjectFinder<UStaticMesh> CoreMesh(TEXT("/Game/Building_Meshes/PlaceholderPortalMesh.PlaceholderPortalMesh"));
	BuildingMesh->SetStaticMesh(CoreMesh.Object);
}

void ACoreBuilding::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (ABGameState* GS = GetWorld()->GetGameState<ABGameState>())
		{
			GS->RegisterCoreBuilding(this, TeamID);
		}
	}
}

void ACoreBuilding::HandleBuildingDestroyed()
{
	Super::HandleBuildingDestroyed();

	if (HasAuthority())
	{
		if (ABGameState* GS = GetWorld()->GetGameState<ABGameState>())
		{
			GS->NotifyCoreDestroyed(this, TeamID);
		}
	}
}