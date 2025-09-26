#include "BigPotionWorkshop_Elves.h"






void ABigPotionWorkshop_Elves::Tick(float DeltaTime){;}


void ABigPotionWorkshop_Elves::BeginPlay(){}

ABigPotionWorkshop_Elves::ABigPotionWorkshop_Elves()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BigPotionMesh(TEXT("/Game/domek_elfow_mesh.domek_elfow_mesh"));
	BuildingMesh->SetStaticMesh(BigPotionMesh.Object);

	BuildingName = TEXT("ElfHouse");
}
