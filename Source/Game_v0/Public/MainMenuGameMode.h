#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

class UClassSelectionWidget;

UCLASS()
class GAME_V0_API AMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMainMenuGameMode();

protected:
	virtual void BeginPlay() override;

	// Widget class reference
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UClassSelectionWidget> ClassSelectionWidgetClass;

	// Instance of the widget
	UPROPERTY()
	UClassSelectionWidget* ClassSelectionWidget;

	// Main game level to load after selection
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Levels")
	FString MainGameLevelName;

public:
	// Called when class selection is complete
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void OnClassSelectionComplete(ERaceNames SelectedRace);
};