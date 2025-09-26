#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "RaceNames.h"
#include "ClassSelectionWidget.generated.h"

/**
 *
 Takes all races from race enum creates buttons for all of them, stores and broadcasts selected race
 *
 *
 *
 *
 * 
 */
UCLASS()
class GAME_V0_API UClassSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* ClassListContainer;

private:
	void CreateClassButtons();
    
	UFUNCTION()
	void OnClassButtonClicked();
    
	// Store which race each button represents
	UPROPERTY()
	TMap<UButton*, ERaceNames> ButtonRaceMap;

public:
	// Function to get the selected race - called by PlayerState or GameMode
	ERaceNames GetSelectedRace() const { return SelectedRace; }
    
	// Check if a race has been selected
	bool HasRaceBeenSelected() const { return bRaceSelected; }

private:
	// Store the selected race locally
	ERaceNames SelectedRace;
	bool bRaceSelected = false;
};