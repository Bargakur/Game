#include "BuildingPlacementComponent.h"

#include "BuildingPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "CustomPlayerState.h"


UBuildingPlacementComponent::UBuildingPlacementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bIsPlacementValid = true;
}

void UBuildingPlacementComponent::BeginPlay()
{
	Super::BeginPlay();
	PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
}

void UBuildingPlacementComponent::StartPlacingBuilding()
{
	if (!BuildingClass)
	{
		UE_LOG(LogTemp, Error, TEXT("No BuildingClass set in BuildingPlacementComponent"));
		return;
	}

	// Spawn a preview building (not solid)
	PreviewBuilding = GetWorld()->SpawnActor<ABuildingBase>(BuildingClass, FVector::ZeroVector, FRotator::ZeroRotator);
	if (PreviewBuilding)
	{
		PreviewBuilding->SetActorEnableCollision(false); // Disable collision for preview

		// Set preview material/appearance if needed
		// This could be done by calling a method on the building itself
		// PreviewBuilding->SetPreviewMode(true);

		UE_LOG(LogTemp, Display, TEXT("Preview building of class %s spawned"), *BuildingClass->GetName());
	}
}


void UBuildingPlacementComponent::ConfirmBuildingPlacement(){
		UE_LOG(LogTemp, Warning, TEXT("ConfirmBuildingPlacement called"));
    
		if (!PreviewBuilding) {
			UE_LOG(LogTemp, Warning, TEXT("No preview building to confirm"));
			return;
		}
    
		if (!bIsPlacementValid){
			UE_LOG(LogTemp, Warning, TEXT("Cannot place building here - invalid location"));
			return;
		}
    
		// Check if player can afford the building
		if (!CanPlaceBuilding(BuildingClass)){
			UE_LOG(LogTemp, Warning, TEXT("Cannot afford to build this structure - canceling placement"));
        
			// Cancel the placement instead of just returning
			CancelBuildingPlacement();
			return;
		}
    
		if (PreviewBuilding){
			PreviewBuilding->SetActorEnableCollision(true);
        
			// Consume the resources after successful placement
			UE_LOG(LogTemp, Warning, TEXT("About to call OnBuildingPlaced"));
			OnBuildingPlaced(BuildingClass);
        
			PreviewBuilding = nullptr;
		}
		UE_LOG(LogTemp, Display, TEXT("Building of class %s placed successfully"), *BuildingClass->GetName());
	}



void UBuildingPlacementComponent::CancelBuildingPlacement(){
	if (PreviewBuilding){
		PreviewBuilding->Destroy();
		PreviewBuilding = nullptr;
	}
}

void UBuildingPlacementComponent::UpdatePreviewBuilding(){
	if (!PreviewBuilding) return;

	FVector MouseLocation = GetMouseWorldLocation();
	PreviewBuilding->SetActorLocation(MouseLocation);
}

FVector UBuildingPlacementComponent::GetMouseWorldLocation(){
	FVector WorldLocation, WorldDirection;
	if (PlayerController && PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection)){
		FVector End = WorldLocation + (WorldDirection * 10000.f);
		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, End, ECC_Visibility)){
			return HitResult.Location;
		}
	}
	return FVector::ZeroVector;
}
void UBuildingPlacementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
	if (PreviewBuilding)
	{
		UpdatePreviewBuilding();
	}
}

void UBuildingPlacementComponent::SetBuildingClass(TSubclassOf<ABuildingBase> NewBuildingClass)
{
	BuildingClass = NewBuildingClass;
}
ACustomPlayerState* UBuildingPlacementComponent::GetOwnerPlayerState() const
{
	if (AActor* Owner = GetOwner())
	{
		if (ABuildingPlayerController* PC = Cast<ABuildingPlayerController>(Owner))
		{
			return PC->GetPlayerState<ACustomPlayerState>();
		}
	}
	return nullptr;
}
bool UBuildingPlacementComponent::CanPlaceBuilding(TSubclassOf<ABuildingBase> BuildingClass)
{
	UE_LOG(LogTemp, Warning, TEXT("CanPlaceBuilding called for class: %s"), BuildingClass ? *BuildingClass->GetName() : TEXT("NULL"));
    
	ACustomPlayerState* PlayerState = GetOwnerPlayerState();
	if (!PlayerState || !PlayerState->GetPlayerRace())
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerState or Race is null"));
		return false;
	}
    
	URace_base* Race = PlayerState->GetPlayerRace().GetDefaultObject();
	bool bCanAfford = Race->CanAffordBuilding(BuildingClass, PlayerState->PlayerResources);
    
	UE_LOG(LogTemp, Warning, TEXT("Can afford building: %s"), bCanAfford ? TEXT("YES") : TEXT("NO"));
    
	return bCanAfford;
}

void UBuildingPlacementComponent::OnBuildingPlaced(TSubclassOf<ABuildingBase> BuildingClass)
{
	UE_LOG(LogTemp, Warning, TEXT("OnBuildingPlaced called for class: %s"), BuildingClass ? *BuildingClass->GetName() : TEXT("NULL"));
    
	ACustomPlayerState* PlayerState = GetOwnerPlayerState();
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerState is null in OnBuildingPlaced"));
		return;
	}
    
	// Only consume resources on server
	if (!PlayerState->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Not server authority, skipping resource consumption"));
		return;
	}
    
	URace_base* Race = PlayerState->GetPlayerRace().GetDefaultObject();
	if (!Race)
	{
		UE_LOG(LogTemp, Error, TEXT("Race is null in OnBuildingPlaced"));
		return;
	}
    
	TArray<FResource> Cost = Race->GetBuildingCost(BuildingClass);
	UE_LOG(LogTemp, Warning, TEXT("Building cost has %d resources"), Cost.Num());
    
	for (const FResource& ResourceCost : Cost)
	{
		UE_LOG(LogTemp, Warning, TEXT("Consuming %d %s"), ResourceCost.ResourceAmount, *ResourceCost.ResourceName.ToString());
		bool bConsumed = PlayerState->ConsumeResource(ResourceCost, ResourceCost.ResourceAmount);
		UE_LOG(LogTemp, Warning, TEXT("Resource consumption result: %s"), bConsumed ? TEXT("SUCCESS") : TEXT("FAILED"));
	}
}
