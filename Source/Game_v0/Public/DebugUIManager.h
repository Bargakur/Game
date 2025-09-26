#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "DebugUIManager.generated.h"

class UClassSelectionWidget;
class UResourceDisplayWidget;

UCLASS()
class GAME_V0_API ADebugUIManager : public AActor
{
    GENERATED_BODY()
    
public:    
    ADebugUIManager();

    // Show a debug widget by class
    UFUNCTION(BlueprintCallable, Category = "Debug UI")
    UUserWidget* ShowDebugWidget(TSubclassOf<UUserWidget> WidgetClass);
    
    // Show ResourceDisplayWidget with Blueprint support
    UFUNCTION(BlueprintCallable, Category = "Debug UI")
    UResourceDisplayWidget* ShowResourceDisplayWidget();
    
    // Show ClassSelectionWidget with Blueprint support  
    UFUNCTION(BlueprintCallable, Category = "Debug UI")
    UClassSelectionWidget* ShowClassSelectionWidget();
    
    // Show widget by Blueprint path
    UFUNCTION(BlueprintCallable, Category = "Debug UI")
    UUserWidget* ShowWidgetByPath(const FString& BlueprintPath);
    
    // Clear all debug widgets
    UFUNCTION(BlueprintCallable, Category = "Debug UI")
    void ClearDebugWidgets();

protected:
    // Blueprint class references - set these in Blueprint or constructor
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget Classes")
    TSubclassOf<UResourceDisplayWidget> ResourceDisplayWidgetClass;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget Classes")
    TSubclassOf<UClassSelectionWidget> ClassSelectionWidgetClass;
    
    // Track active widgets for cleanup
    UPROPERTY()
    TArray<UUserWidget*> ActiveDebugWidgets;
};