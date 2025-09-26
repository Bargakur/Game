#include "BlacksmithWorkshop_Elves.h"




ABlacksmithWorkshop_Elves::ABlacksmithWorkshop_Elves()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BlacksmithMesh(TEXT("/Game/Building_Meshes/ElfBlacksmith_Mesh.ElfBlacksmith_Mesh"));
	BuildingMesh->SetStaticMesh(BlacksmithMesh.Object);

	BuildingName = "ElfBlacksmith";
	
}
void ABlacksmithWorkshop_Elves::BeginPlay()
{
	;
}
void ABlacksmithWorkshop_Elves::Tick(float DeltaTime)
{
	;
}

