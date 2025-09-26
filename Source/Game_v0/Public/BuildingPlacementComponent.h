#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SimpleBuildingSystem.h"
#include "buildings/BuildingBase.h"
#include "BuildingPlacementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAME_V0_API UBuildingPlacementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuildingPlacementComponent();
	// Add to your UBuildingPlacementComponent.h
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Building Placement")
	void StartPlacingBuilding();

	UFUNCTION(BlueprintCallable, Category = "Building Placement")
	void ConfirmBuildingPlacement();

	UFUNCTION(BlueprintCallable, Category = "Building Placement")
	void CancelBuildingPlacement();

	UFUNCTION(BlueprintCallable, Category = "Building")
	void SetBuildingClass(TSubclassOf<ABuildingBase> NewBuildingClass);

	UFUNCTION(BlueprintCallable, Category = "Building Placement")
	bool IsPlacingBuilding() const { return PreviewBuilding != nullptr; }

	UFUNCTION(BlueprintCallable, Category = "Building")
	TSubclassOf<ABuildingBase> GetCurrentBuildingClass() const { return BuildingClass; }

	// For UI/HUD to get building info
	UFUNCTION(BlueprintCallable, Category = "Building")
	ABuildingBase* GetPreviewBuilding() const { return PreviewBuilding; }

	bool CanPlaceBuilding(TSubclassOf<ABuildingBase> BuildingClass);
	void OnBuildingPlaced(TSubclassOf<ABuildingBase> BuildingClass);
    


private:
	void UpdatePreviewBuilding();

	FTimerHandle InitializationTimerHandle;
	
	
	FVector GetMouseWorldLocation();

	UPROPERTY()
	ABuildingBase* PreviewBuilding;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABuildingBase> BuildingClass;

	UPROPERTY()
	APlayerController* PlayerController;

	UPROPERTY()
	bool bIsPlacementValid;
	
	ACustomPlayerState* GetOwnerPlayerState() const;

	bool  CheckPlacementValidity();
};
