#include "ResourceDisplayWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/BorderSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/Engine.h"

void UResourceDisplayWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    
    // Don't create widget tree here - let UMG handle it
    UE_LOG(LogTemp, Log, TEXT("ResourceDisplayWidget initialized"));
}

void UResourceDisplayWidget::EnsureWidgetTreeExists()
{
    if (!WidgetTree)
    {
        WidgetTree = NewObject<UWidgetTree>(this);
        UE_LOG(LogTemp, Warning, TEXT("Had to create WidgetTree manually - this might indicate a Blueprint issue"));
    }
}

void UResourceDisplayWidget::NativePreConstruct()
{
    Super::NativePreConstruct();
    
    // Clear any existing cached data
    CachedGroupedResources.Empty();
    ResourceTextWidgets.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("NativePreConstruct called"));
}

void UResourceDisplayWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // CRITICAL: Check binding AFTER Super::NativeConstruct()
    UE_LOG(LogTemp, Warning, TEXT("=== Widget Binding Check ==="));
    UE_LOG(LogTemp, Warning, TEXT("ResourcePanel: %s (Type: %s)"), 
           ResourcePanel ? TEXT("BOUND") : TEXT("NULL"),
           ResourcePanel ? *ResourcePanel->GetClass()->GetName() : TEXT("N/A"));
    UE_LOG(LogTemp, Warning, TEXT("ResourceContainer: %s (Type: %s)"), 
           ResourceContainer ? TEXT("BOUND") : TEXT("NULL"),
           ResourceContainer ? *ResourceContainer->GetClass()->GetName() : TEXT("N/A"));
    
    // Check if this widget was created from a Blueprint
    if (GetClass() != UResourceDisplayWidget::StaticClass())
    {
        UE_LOG(LogTemp, Log, TEXT("Widget is Blueprint-derived: %s"), *GetClass()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Widget is pure C++ class - Blueprint binding won't work!"));
    }
    
    // Only initialize programmatically if binding failed
    if (!ResourcePanel || !ResourceContainer)
    {
        UE_LOG(LogTemp, Warning, TEXT("Blueprint binding failed - falling back to programmatic creation"));
        
        // Ensure WidgetTree exists before doing anything
        EnsureWidgetTreeExists();
        
        // Initialize the widget structure programmatically
        InitializeWidgetStructure();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Blueprint widgets successfully bound!"));
        
        // Verify the bound widgets are in the correct hierarchy
        if (ResourceContainer->GetParent() != ResourcePanel)
        {
            UE_LOG(LogTemp, Error, TEXT("Widget hierarchy mismatch! ResourceContainer parent is not ResourcePanel"));
        }
    }
    
    // Position the widget in bottom right corner if it's on a canvas
    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
    {
        CanvasSlot->SetAnchors(FAnchors(1.0f, 1.0f, 1.0f, 1.0f));
        CanvasSlot->SetAlignment(FVector2D(1.0f, 1.0f));
        CanvasSlot->SetPosition(FVector2D(-20.0f, -20.0f)); // Offset from bottom right
        CanvasSlot->SetAutoSize(true);
    }
}

void UResourceDisplayWidget::InitializeWidgetStructure()
{
    // This should only be called if Blueprint binding failed
    UE_LOG(LogTemp, Warning, TEXT("Creating widgets programmatically - Blueprint binding failed!"));
    
    // Ensure WidgetTree exists
    if (!WidgetTree)
    {
        UE_LOG(LogTemp, Error, TEXT("WidgetTree is null in InitializeWidgetStructure!"));
        return;
    }

    // Create main border panel only if not bound
    if (!ResourcePanel)
    {
        ResourcePanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
        if (IsValid(ResourcePanel))
        {
            ResourcePanel->SetBrushColor(PanelBackgroundColor);
            
            // Set as root widget if we don't have one
            if (!WidgetTree->RootWidget)
            {
                WidgetTree->RootWidget = ResourcePanel;
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create ResourcePanel!"));
            return;
        }
    }
    
    // Create vertical box container only if not bound
    if (!ResourceContainer && IsValid(ResourcePanel))
    {
        ResourceContainer = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
        
        if (IsValid(ResourceContainer))
        {
            // Add container to panel
            UBorderSlot* BorderSlot = Cast<UBorderSlot>(ResourcePanel->AddChild(ResourceContainer));
            if (BorderSlot)
            {
                BorderSlot->SetPadding(FMargin(PanelPadding));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create ResourceContainer!"));
        }
    }
}

void UResourceDisplayWidget::UpdateResources(const TArray<FResource>& Resources)
{
    if (!IsValid(ResourceContainer))
    {
        UE_LOG(LogTemp, Error, TEXT("ResourceContainer is not valid in UpdateResources - cannot display resources!"));
        return;
    }

    // Group resources by name and sum their amounts
    TMap<FName, int32> NewGroupedResources = GroupResourcesByName(Resources);

    // Get all unique resource names
    TSet<FName> AllResourceNames;
    NewGroupedResources.GetKeys(AllResourceNames);
    
    TArray<FName> CachedKeys;
    CachedGroupedResources.GetKeys(CachedKeys);
    AllResourceNames.Append(CachedKeys);

    // Process each resource name
    for (const FName& ResourceName : AllResourceNames)
    {
        int32* NewAmount = NewGroupedResources.Find(ResourceName);
        int32* CachedAmount = CachedGroupedResources.Find(ResourceName);

        if (NewAmount && CachedAmount)
        {
            // Resource exists in both - check if amount changed
            if (*NewAmount != *CachedAmount)
            {
                UpdateSingleResourceDisplay(ResourceName, *NewAmount);
            }
        }
        else if (NewAmount && !CachedAmount)
        {
            // New resource - create display
            FResource DisplayResource;
            DisplayResource.ResourceName = ResourceName;
            DisplayResource.ResourceAmount = *NewAmount;
            
            UHorizontalBox* NewEntry = CreateResourceEntry(DisplayResource);
            if (IsValid(NewEntry))
            {
                ResourceContainer->AddChild(NewEntry);
            }
        }
        else if (!NewAmount && CachedAmount)
        {
            // Resource removed - remove display
            if (UTextBlock** FoundWidget = ResourceTextWidgets.Find(ResourceName))
            {
                if (UTextBlock* TextWidget = *FoundWidget)
                {
                    if (IsValid(TextWidget))
                    {
                        if (UWidget* ParentWidget = TextWidget->GetParent())
                        {
                            ParentWidget->RemoveFromParent();
                        }
                    }
                }
                ResourceTextWidgets.Remove(ResourceName);
            }
        }
    }

    // Update cached resources
    CachedGroupedResources = NewGroupedResources;
}

void UResourceDisplayWidget::UpdateSingleResourceDisplay(const FName& ResourceName, int32 NewAmount)
{
    if (UTextBlock** FoundWidget = ResourceTextWidgets.Find(ResourceName))
    {
        if (UTextBlock* AmountText = *FoundWidget)
        {
            if (IsValid(AmountText))
            {
                AmountText->SetText(FText::AsNumber(NewAmount));
            }
        }
    }
}

TMap<FName, int32> UResourceDisplayWidget::GroupResourcesByName(const TArray<FResource>& Resources)
{
    TMap<FName, int32> GroupedResources;
    
    for (const FResource& Resource : Resources)
    {
        if (Resource.IsValid())
        {
            if (int32* ExistingAmount = GroupedResources.Find(Resource.ResourceName))
            {
                // Add to existing total
                *ExistingAmount += Resource.ResourceAmount;
            }
            else
            {
                // Create new entry
                GroupedResources.Add(Resource.ResourceName, Resource.ResourceAmount);
            }
        }
    }
    
    return GroupedResources;
}

UHorizontalBox* UResourceDisplayWidget::CreateResourceEntry(const FResource& Resource)
{
    if (!IsValid(ResourceContainer))
    {
        UE_LOG(LogTemp, Warning, TEXT("ResourceContainer is not valid in CreateResourceEntry"));
        return nullptr;
    }
    
    if (!WidgetTree)
    {
        UE_LOG(LogTemp, Error, TEXT("WidgetTree is null in CreateResourceEntry!"));
        return nullptr;
    }

    // Create horizontal box for this resource entry
    UHorizontalBox* ResourceEntry = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
    if (!IsValid(ResourceEntry))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create ResourceEntry HorizontalBox"));
        return nullptr;
    }
    
    // Create resource name text
    UTextBlock* ResourceNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    if (IsValid(ResourceNameText))
    {
        ResourceNameText->SetText(FText::FromName(Resource.ResourceName));
        ResourceNameText->SetColorAndOpacity(FSlateColor(TextColor));
    }
    
    // Create resource amount text
    UTextBlock* ResourceAmountText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    if (IsValid(ResourceAmountText))
    {
        ResourceAmountText->SetText(FText::AsNumber(Resource.ResourceAmount));
        ResourceAmountText->SetColorAndOpacity(FSlateColor(TextColor));
        
        // Store reference to amount text for updates
        ResourceTextWidgets.Add(Resource.ResourceName, ResourceAmountText);
    }
    
    // Add widgets to horizontal box
    if (IsValid(ResourceNameText))
    {
        ResourceEntry->AddChild(ResourceNameText);
        
        // Set up slot for proper spacing
        if (UHorizontalBoxSlot* NameSlot = Cast<UHorizontalBoxSlot>(ResourceNameText->Slot))
        {
            NameSlot->SetHorizontalAlignment(HAlign_Left);
            NameSlot->SetPadding(FMargin(0, 2, 10, 2));
            NameSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        }
    }
    
    if (IsValid(ResourceAmountText))
    {
        ResourceEntry->AddChild(ResourceAmountText);
        
        // Set up slot for proper spacing
        if (UHorizontalBoxSlot* AmountSlot = Cast<UHorizontalBoxSlot>(ResourceAmountText->Slot))
        {
            AmountSlot->SetHorizontalAlignment(HAlign_Right);
            AmountSlot->SetPadding(FMargin(0, 2, 0, 2));
            AmountSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
        }
    }
    
    return ResourceEntry;
}

void UResourceDisplayWidget::UpdateResourceEntry(const FName& ResourceName, int32 NewAmount)
{
    UpdateSingleResourceDisplay(ResourceName, NewAmount);
}

void UResourceDisplayWidget::UpdateSingleResource(const FName& ResourceName, int32 Amount)
{
    // Create a single resource and update display
    FResource SingleResource;
    SingleResource.ResourceName = ResourceName;
    SingleResource.ResourceAmount = Amount;
    
    TArray<FResource> SingleResourceArray;
    SingleResourceArray.Add(SingleResource);
    
    // Use the main update function
    UpdateResources(SingleResourceArray);
}

void UResourceDisplayWidget::ClearAllResources()
{
    if (!IsValid(ResourceContainer))
    {
        return;
    }
    
    // Clear all children from the container
    ResourceContainer->ClearChildren();
    
    // Clear cached data
    CachedGroupedResources.Empty();
    ResourceTextWidgets.Empty();
}