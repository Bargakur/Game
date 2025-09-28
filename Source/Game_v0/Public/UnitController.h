#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PhysicalResourceActor.h"
#include "Navigation/PathFollowingComponent.h"
#include "UnitCommand.h"

#include "UnitController.generated.h"


class AUnitBase;



UCLASS()
class GAME_V0_API AUnitController : public AAIController
{
	GENERATED_BODY()

public:
	AUnitController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	

	UPROPERTY()
	AUnitBase* ControlledUnit;

	UPROPERTY()
	FVector CurrentDestination;

	UPROPERTY()
	bool bHasDestination;

	UPROPERTY()
	bool bIsMoving;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AcceptanceRadius = 100.0f;
	
	UPROPERTY()
	bool bUseDirectMovement = false;

public:
	// Movement commands

	void MoveToLocation(FVector Destination);


	virtual void StopMovement() override;

	// Command execution
	void ExecuteCommand(const FUnitCommand& Command);

	// State queries
	UFUNCTION(BlueprintPure)
	bool IsMoving() const { return bIsMoving; }

	UFUNCTION(BlueprintPure)
	FVector GetCurrentDestination() const { return CurrentDestination; }

	UFUNCTION(BlueprintCallable)
	void ExecuteResourceCommand(const FUnitCommand& Command);
    
	// Current resource-related command state
	UPROPERTY()
	TWeakObjectPtr<APhysicalResourceActor> TargetResourceActor;
    
	UPROPERTY()
	FUnitCommand PendingResourceCommand;
    
	UPROPERTY()
	bool bHasPendingResourceAction = false;

	void CommandPickupNearestResource(FName ResourceName, float SearchRadius);


protected:
	void UpdateMovement(float DeltaTime);
	void OnReachedDestination();


	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	void HandlePickupResourceCommand(const FUnitCommand& Command);
	void HandleDropResourceCommand(const FUnitCommand& Command);
    
	// Check if we've reached the target for resource pickup
	void CheckResourcePickupCompletion();
	
    
	// Resource interaction distance
	UPROPERTY(EditAnywhere, Category = "Resources")
	float ResourceInteractionDistance = 150.0f;


	void CommandDropAllResources();
	void CommandDropResourceSlot(int32 SlotIndex);



private:
	void MoveDirectly(float DeltaTime);	
};