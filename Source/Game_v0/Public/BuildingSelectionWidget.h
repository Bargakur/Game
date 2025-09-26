#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "buildings/BuildingBase.h"
#include "Race_base.h"
#include "BuildingSelectionWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildingSelected, TSubclassOf<ABuildingBase>, SelectedBuildingClass);


UCLASS()
class GAME_V0_API UBuildingSelectionWidget : public UUserWidget
{	
	GENERATED_BODY()
	
public:
	UBuildingSelectionWidget(const FObjectInitializer& ObjectInitializer);
	
	virtual void NativeConstruct() override;
	UFUNCTION(BlueprintCallable)
	TSubclassOf<AActor> GetBuildingClass() const { return buildingclass; }

	UPROPERTY(BlueprintAssignable)
	FOnBuildingSelected OnBuildingSelected;

	URace_base* GetLocalPlayerRaceCDO() const;
	
	
protected:


	UPROPERTY()
	TSubclassOf<ABuildingBase> buildingclass;
	
	// Button references
	UPROPERTY(meta = (BindWidget))
	UButton* Button1;
	
	UPROPERTY(meta = (BindWidget))
	UButton* Button2;
	
	UPROPERTY(meta = (BindWidget))
	UButton* Button3;
	
	// Optional: Text labels for buttons
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Button1Text;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Button2Text;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Button3Text;
	
	// Button click handlers
	UFUNCTION()
	void OnButton1Clicked();
	
	UFUNCTION()
	void OnButton2Clicked();
	
	UFUNCTION()
	void OnButton3Clicked();
};