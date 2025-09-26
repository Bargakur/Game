#include "DwarfHouse.h"

void ADwarfHouse::BeginPlay()
{
	Super::BeginPlay();
}

void ADwarfHouse::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

ADwarfHouse::ADwarfHouse()
{
	PrimaryActorTick.bCanEverTick = true;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DwarfHouse_mesh(TEXT("/Game/Building_Meshes/DwarfHouse_Mesh.DwarfHouse_Mesh"));
	BuildingMesh->SetStaticMesh(DwarfHouse_mesh.Object);
	BuildingName = TEXT("DwarfHouse");
	
}
