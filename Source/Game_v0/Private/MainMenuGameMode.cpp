#include "MainMenuGameMode.h"
#include "ClassSelectionWidget.h"
#include "MyGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AMainMenuGameMode::AMainMenuGameMode()
{
    // Set a simple pawn or no pawn for main menu
    DefaultPawnClass = nullptr;
    
    // Set the main game level name (adjust to your actual level name)
    MainGameLevelName = TEXT("MainGameLevel");

    // Set the class selection widget class
    static ConstructorHelpers::FClassFinder<UClassSelectionWidget> WidgetClassFinder(TEXT("/Game/UI/RaceSelectionwidget"));
    if (WidgetClassFinder.Succeeded())
    {
        ClassSelectionWidgetClass = WidgetClassFinder.Class;
        UE_LOG(LogTemp, Warning, TEXT("Main Menu: Successfully loaded ClassSelectionWidgetClass"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Main Menu: Failed to load ClassSelectionWidgetClass from /Game/UI/RaceSelectionwidget"));
    }
}

void AMainMenuGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Show class selection widget immediately
    if (ClassSelectionWidgetClass)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
        if (PC)
        {
            ClassSelectionWidget = CreateWidget<UClassSelectionWidget>(PC, ClassSelectionWidgetClass);
            if (ClassSelectionWidget)
            {
                ClassSelectionWidget->AddToViewport();
                ClassSelectionWidget->SetVisibility(ESlateVisibility::Visible);
                
                // Set input mode to UI only
                PC->SetInputMode(FInputModeUIOnly());
                PC->SetShowMouseCursor(true);
                
                UE_LOG(LogTemp, Warning, TEXT("Main Menu: Class selection widget shown"));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Main Menu: ClassSelectionWidgetClass not set!"));
    }
}

void AMainMenuGameMode::OnClassSelectionComplete(ERaceNames SelectedRace)
{
    UE_LOG(LogTemp, Log, TEXT("Main Menu: Class selected, transitioning to main game"));
    
    // Store selected race in Game Instance with automatic class mapping
    if (UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(GetGameInstance()))
    {
        MyGameInstance->SetSelectedRace(SelectedRace);  // Use this instead
        UE_LOG(LogTemp, Log, TEXT("Main Menu: Selected race with mapping stored in Game Instance"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Main Menu: Failed to get MyGameInstance!"));
    }
    
    // Load the main game level directly
    if (!MainGameLevelName.IsEmpty())
    {
        UGameplayStatics::OpenLevel(this, FName(*MainGameLevelName));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Main Menu: MainGameLevelName not set!"));
    }
}