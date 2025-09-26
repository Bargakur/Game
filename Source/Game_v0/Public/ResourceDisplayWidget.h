#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "resource.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"
#include "ResourceDisplayWidget.generated.h"

UCLASS()
class GAME_V0_API UResourceDisplayWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Update the displayed resources - removed BlueprintCallable since TArray<FResource> isn't Blueprint compatible
    UFUNCTION(Category = "Resources")
    void UpdateResources(const TArray<FResource>& Resources);

    // Group resources by name and sum their amounts
    UFUNCTION(Category = "Resources")
    TMap<FName, int32> GroupResourcesByName(const TArray<FResource>& Resources);

    // Blueprint-compatible version using individual parameters
    UFUNCTION(BlueprintCallable, Category = "Resources")
    void UpdateSingleResource(const FName& ResourceName, int32 Amount);

    // Blueprint-compatible function to clear all resources
    UFUNCTION(BlueprintCallable, Category = "Resources")
    void ClearAllResources();

protected:
    virtual void NativeConstruct() override;
    virtual void NativePreConstruct() override;
    
    // Override to ensure proper initialization
    virtual void NativeOnInitialized() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor PanelBackgroundColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.7f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor TextColor = FLinearColor::White; 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float PanelPadding = 10.0f;

    // CRITICAL: These must match EXACTLY the widget names in your Blueprint
    // Remove the optional binding meta tag and make them required
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UBorder* ResourcePanel;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UVerticalBox* ResourceContainer;

private:
    // Map to store resource text widgets for efficient updates
    UPROPERTY()
    TMap<FName, class UTextBlock*> ResourceTextWidgets;

    // Cached grouped resources to avoid recalculating
    TMap<FName, int32> CachedGroupedResources;

    // Create a new resource entry widget
    class UHorizontalBox* CreateResourceEntry(const FResource& Resource);
    
    // Update existing resource entry
    void UpdateResourceEntry(const FName& ResourceName, int32 NewAmount);
    
    // Update a single resource display efficiently
    void UpdateSingleResourceDisplay(const FName& ResourceName, int32 NewAmount);
    
    // Initialize the widget structure (only when widgets aren't bound)
    void InitializeWidgetStructure();
    
    // Ensure widget tree is properly initialized
    void EnsureWidgetTreeExists();
};