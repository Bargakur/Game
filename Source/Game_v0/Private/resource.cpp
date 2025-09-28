#include "resource.h"


FResource::FResource(const FName ResourceName, const int32 initial_amount = 0, const float initial_weight = 1.0 )

{
	this->ResourceName = ResourceName;
	ResourceAmount = initial_amount;
	Weight = initial_weight;
}

void FResource::SetOnGround(FVector Location)
{
	LocationType = EResourceLocation::OnGround;
	WorldLocation = Location;
	ContainerActor = nullptr;
}

void FResource::SetCarriedBy(AActor* CarryingActor)
{
	LocationType = EResourceLocation::CarriedByUnit;
	ContainerActor = CarryingActor;
	WorldLocation = CarryingActor ? CarryingActor->GetActorLocation() : FVector::ZeroVector;
}

void FResource::SetInPlayerInventory()
{
	LocationType = EResourceLocation::InPlayerInventory;
	ContainerActor = nullptr;
	WorldLocation = FVector::ZeroVector;
	PhysicalActor = nullptr;
}