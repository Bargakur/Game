#include "ClassSelectionWidget.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UObject/ConstructorHelpers.h"
#include "MainMenuGameMode.h"

void UClassSelectionWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    CreateClassButtons();
}

void UClassSelectionWidget::CreateClassButtons()
{
    if (!ClassListContainer)
    {
        UE_LOG(LogTemp, Warning, TEXT("ClassListContainer not found! Make sure it's bound in the widget blueprint."));
        return;
    }

    // Clear any existing buttons
    ClassListContainer->ClearChildren();
    ButtonRaceMap.Empty();

    // Iterate through all race names (excluding MAX)
    for (int32 i = 1; i < static_cast<int32>(ERaceNames::MAX); ++i)
    {
        ERaceNames CurrentRace = static_cast<ERaceNames>(i);
        
        // Create button for this class
        UButton* ClassButton = NewObject<UButton>(this);
        if (ClassButton)
        {
            // Create text block for button label
            UTextBlock* ButtonText = NewObject<UTextBlock>(this);
            if (ButtonText)
            {
                // Get the display name from the enum meta data
                FString DisplayName;
                const UEnum* EnumPtr = StaticEnum<ERaceNames>();
                if (EnumPtr)
                {
                    DisplayName = EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(CurrentRace)).ToString();
                }
                else
                {
                    // Fallback to enum name if meta data not found
                    DisplayName = StaticEnum<ERaceNames>()->GetNameStringByValue(static_cast<int64>(CurrentRace));
                }
                
                ButtonText->SetText(FText::FromString(DisplayName));
                ClassButton->AddChild(ButtonText);
            }

            // Store which race this button represents
            ButtonRaceMap.Add(ClassButton, CurrentRace);
            
            // All buttons use the same click handler
            ClassButton->OnClicked.AddDynamic(this, &UClassSelectionWidget::OnClassButtonClicked);
            
            // Add button to container
            ClassListContainer->AddChild(ClassButton);
        }
    }
}

void UClassSelectionWidget::OnClassButtonClicked()
{
    // Find which button was clicked by checking which one has focus
    UButton* ClickedButton = nullptr;
    
    // Check all buttons in our map to find the focused one
    for (auto& ButtonPair : ButtonRaceMap)
    {
        if (ButtonPair.Key && ButtonPair.Key->HasKeyboardFocus())
        {
            ClickedButton = ButtonPair.Key;
            break;
        }
    }
    
    if (ClickedButton)
    {
        // Store the selected race locally
        SelectedRace = ButtonRaceMap[ClickedButton];
        bRaceSelected = true;
        
        // Get display name for logging
        const UEnum* EnumPtr = StaticEnum<ERaceNames>();
        FString DisplayName = EnumPtr ? 
            EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(SelectedRace)).ToString() : 
            TEXT("Unknown");
        
        UE_LOG(LogTemp, Warning, TEXT("Race selected: %s"), *DisplayName);

        if (AMainMenuGameMode* GameMode = Cast<AMainMenuGameMode>(GetWorld()->GetAuthGameMode()))
        {
            UE_LOG(LogTemp, Warning, TEXT("Calling GameMode OnClassSelectionComplete"));
            GameMode->OnClassSelectionComplete(SelectedRace);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get MainMenuGameMode!"));
        }
        
        // Notify that selection is complete - PlayerState can now call GetSelectedRace()
        // You could also broadcast a delegate here if needed
        

        SetVisibility(ESlateVisibility::Hidden); 
    }
}