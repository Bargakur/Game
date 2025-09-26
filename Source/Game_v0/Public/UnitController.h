#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

#include "UnitController.generated.h"


class AUnitBase;
struct FUnitCommand;


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


protected:
	void UpdateMovement(float DeltaTime);
	void OnReachedDestination();


	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;



private:
	void MoveDirectly(float DeltaTime);	
};