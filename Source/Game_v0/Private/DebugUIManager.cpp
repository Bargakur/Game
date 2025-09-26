// DebugUIManager.cpp - Fixed version with Blueprint widget support
#include "DebugUIManager.h"
#include "ClassSelectionWidget.h"
#include "ResourceDisplayWidget.h" // Add this include
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/Class.h"
#include "Engine/AssetManager.h"
#include "UObject/ConstructorHelpers.h"

ADebugUIManager::ADebugUIManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Initialize Blueprint widget class references
    // You'll need to set these paths to match your actual Blueprint locations
    static ConstructorHelpers::FClassFinder<UResourceDisplayWidget> ResourceWidgetBP(
        TEXT("/Game/UI/WBP_ResourceDisplay"));
    if (ResourceWidgetBP.Succeeded())
    {
        ResourceDisplayWidgetClass = ResourceWidgetBP.Class;
    }
    
    static ConstructorHelpers::FClassFinder<UClassSelectionWidget> ClassSelectionBP(
        TEXT("/Game/UI/RaceSelectionwidget"));
    if (ClassSelectionBP.Succeeded())
    {
        ClassSelectionWidgetClass = ClassSelectionBP.Class;
    }
}

UUserWidget* ADebugUIManager::ShowDebugWidget(TSubclassOf<UUserWidget> WidgetClass)
{
    if (!WidgetClass)
       return nullptr;
        
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
       return nullptr;
    
    // Check if this is a Blueprint class or pure C++ class
    bool bIsBlueprintClass = WidgetClass->GetName().Contains(TEXT("_C"));
    if (!bIsBlueprintClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempting to create pure C++ widget class '%s' - widget binding may not work!"), 
               *WidgetClass->GetName());
    }
        
    UUserWidget* NewWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
    if (NewWidget)
    {
       NewWidget->AddToViewport();
       
       // Enable mouse cursor for widget interaction
       PC->SetShowMouseCursor(true);
       PC->SetInputMode(FInputModeGameAndUI());
       
       ActiveDebugWidgets.Add(NewWidget);
       
       UE_LOG(LogTemp, Log, TEXT("Widget '%s' created successfully"), *WidgetClass->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create widget of class '%s'"), *WidgetClass->GetName());
    }
    
    return NewWidget;
}

UResourceDisplayWidget* ADebugUIManager::ShowResourceDisplayWidget()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
       return nullptr;
    
    UResourceDisplayWidget* ResourceWidget = nullptr;
    
    // Try to use Blueprint class first (recommended)
    if (ResourceDisplayWidgetClass)
    {
        ResourceWidget = CreateWidget<UResourceDisplayWidget>(PC, ResourceDisplayWidgetClass);
        UE_LOG(LogTemp, Log, TEXT("Created ResourceDisplayWidget from Blueprint class"));
    }
    else
    {
        // Fallback to pure C++ class (widget binding won't work)
        ResourceWidget = CreateWidget<UResourceDisplayWidget>(PC, UResourceDisplayWidget::StaticClass());
        UE_LOG(LogTemp, Warning, TEXT("Created ResourceDisplayWidget from pure C++ class - binding will fail!"));
    }
    
    if (ResourceWidget)
    {
       ResourceWidget->AddToViewport();
       
       // Enable mouse cursor for widget interaction
       PC->SetShowMouseCursor(true);
       PC->SetInputMode(FInputModeGameAndUI());
       
       ActiveDebugWidgets.Add(ResourceWidget);
       
       UE_LOG(LogTemp, Warning, TEXT("ResourceDisplayWidget created and added to viewport"));
    }
    
    return ResourceWidget;
}

UClassSelectionWidget* ADebugUIManager::ShowClassSelectionWidget()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
       return nullptr;
    
    UClassSelectionWidget* ClassWidget = nullptr;
    
    // Try to use Blueprint class first (recommended)
    if (ClassSelectionWidgetClass)
    {
        ClassWidget = CreateWidget<UClassSelectionWidget>(PC, ClassSelectionWidgetClass);
        UE_LOG(LogTemp, Log, TEXT("Created ClassSelectionWidget from Blueprint class"));
    }
    else
    {
        // Fallback to pure C++ class (widget binding won't work)
        ClassWidget = CreateWidget<UClassSelectionWidget>(PC, UClassSelectionWidget::StaticClass());
        UE_LOG(LogTemp, Warning, TEXT("Created ClassSelectionWidget from pure C++ class - binding will fail!"));
    }
    
    if (ClassWidget)
    {
       ClassWidget->AddToViewport();
       
       // Enable mouse cursor for widget interaction
       PC->SetShowMouseCursor(true);
       PC->SetInputMode(FInputModeGameAndUI());
       
       ActiveDebugWidgets.Add(ClassWidget);
       
       UE_LOG(LogTemp, Warning, TEXT("ClassSelectionWidget created and added to viewport"));
    }
    
    return ClassWidget;
}

UUserWidget* ADebugUIManager::ShowWidgetByPath(const FString& BlueprintPath)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
       return nullptr;
    
    // Load the Blueprint class from the provided path
    UClass* WidgetClass = LoadClass<UUserWidget>(nullptr, *BlueprintPath);
    if (!WidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load widget class from path: %s"), *BlueprintPath);
        return nullptr;
    }
    
    return ShowDebugWidget(WidgetClass);
}

void ADebugUIManager::ClearDebugWidgets()
{
    // Restore input mode when clearing widgets
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        PC->SetShowMouseCursor(false);
        PC->SetInputMode(FInputModeGameOnly());
    }
    
    for (UUserWidget* Widget : ActiveDebugWidgets)
    {
       if (Widget)
       {
          Widget->RemoveFromParent();
       }
    }
    
    ActiveDebugWidgets.Empty();
}

// Enhanced command function with Blueprint class support
void ShowDebugWidgetCommand(const TArray<FString>& Args, UWorld* InWorld)
{
    if (!InWorld)
        return;
        
    ADebugUIManager* DebugManager = Cast<ADebugUIManager>(UGameplayStatics::GetActorOfClass(InWorld, ADebugUIManager::StaticClass()));
    if (!DebugManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("No DebugUIManager found in world. Spawning one..."));
        DebugManager = InWorld->SpawnActor<ADebugUIManager>();
    }
    
    if (Args.Num() > 0)
    {
        FString WidgetName = Args[0];
        
        // Special cases for known widgets with Blueprint support
        if (WidgetName == "ResourceDisplay" || WidgetName == "ResourceDisplayWidget")
        {
            DebugManager->ShowResourceDisplayWidget();
            return;
        }
        
        if (WidgetName == "ClassSelection" || WidgetName == "ClassSelectionWidget")
        {
            DebugManager->ShowClassSelectionWidget();
            return;
        }
        
        // Try to load as Blueprint path directly
        if (WidgetName.Contains(TEXT("/Game/")))
        {
            DebugManager->ShowWidgetByPath(WidgetName);
            return;
        }
        
        // Search for widget classes, prioritizing Blueprint classes
        TArray<UClass*> FoundClasses;
        
        for (TObjectIterator<UClass> It; It; ++It)
        {
            UClass* Class = *It;
            if (Class->IsChildOf(UUserWidget::StaticClass()) && 
                !Class->HasAnyClassFlags(CLASS_Abstract))
            {
                // Check name matching
                if (Class->GetName().Contains(WidgetName) || 
                    Class->GetName() == WidgetName ||
                    Class->GetName() == (WidgetName + "_C"))
                {
                    FoundClasses.Add(Class);
                }
            }
        }
        
        // Sort classes - Blueprint classes (ending with _C) first
        FoundClasses.Sort([](const UClass& A, const UClass& B) {
            bool AIsBlueprint = A.GetName().EndsWith(TEXT("_C"));
            bool BIsBlueprint = B.GetName().EndsWith(TEXT("_C"));
            if (AIsBlueprint && !BIsBlueprint) return true;
            if (!AIsBlueprint && BIsBlueprint) return false;
            return A.GetName() < B.GetName();
        });
        
        if (FoundClasses.Num() > 0)
        {
            UClass* SelectedClass = FoundClasses[0];
            UE_LOG(LogTemp, Log, TEXT("Found %d matching classes, using: %s"), 
                   FoundClasses.Num(), *SelectedClass->GetName());
            
            // List all found classes for reference
            for (int32 i = 0; i < FoundClasses.Num(); ++i)
            {
                UE_LOG(LogTemp, Log, TEXT("  [%d] %s"), i, *FoundClasses[i]->GetName());
            }
            
            DebugManager->ShowDebugWidget(SelectedClass);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Widget class '%s' not found"), *WidgetName);
            UE_LOG(LogTemp, Warning, TEXT("Try using full Blueprint path like: /Game/UI/BP_YourWidget.BP_YourWidget_C"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Usage: ShowDebugWidget <WidgetClassName|BlueprintPath>"));
        UE_LOG(LogTemp, Warning, TEXT("Available shortcuts: ResourceDisplay, ClassSelection"));
        UE_LOG(LogTemp, Warning, TEXT("Example paths:"));
        UE_LOG(LogTemp, Warning, TEXT("  ShowDebugWidget /Game/UI/BP_ResourceDisplayWidget.BP_ResourceDisplayWidget_C"));
    }
}

// Clear widgets command (unchanged)
void ClearDebugWidgetsCommand(const TArray<FString>& Args, UWorld* InWorld)
{
    if (!InWorld)
        return;
        
    ADebugUIManager* DebugManager = Cast<ADebugUIManager>(UGameplayStatics::GetActorOfClass(InWorld, ADebugUIManager::StaticClass()));
    if (DebugManager)
    {
        DebugManager->ClearDebugWidgets();
        UE_LOG(LogTemp, Warning, TEXT("Debug widgets cleared"));
    }
}

// Register the commands
FAutoConsoleCommandWithWorldAndArgs ShowWidgetCmd(
    TEXT("ShowDebugWidget"),
    TEXT("Shows a debug widget on screen. Usage: ShowDebugWidget <WidgetName|BlueprintPath>"),
    FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&ShowDebugWidgetCommand)
);

FAutoConsoleCommandWithWorldAndArgs ClearWidgetsCmd(
    TEXT("ClearDebugWidgets"),
    TEXT("Clears all debug widgets from screen"),
    FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&ClearDebugWidgetsCommand)
);