#include "BuildingSelectionWidget.h"

#include "buildings/BigPotionWorkshop_Elves.h"
#include "buildings/chata.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "CustomPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Race_base.h"

#include "Components/TextBlock.h"

UBuildingSelectionWidget::UBuildingSelectionWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
}

void UBuildingSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Bind button click events
	if (Button1)
	{
		Button1->OnClicked.AddDynamic(this, &UBuildingSelectionWidget::OnButton1Clicked);
	}
	
	if (Button2)
	{
		Button2->OnClicked.AddDynamic(this, &UBuildingSelectionWidget::OnButton2Clicked);
	}
	
	if (Button3)
	{
		Button3->OnClicked.AddDynamic(this, &UBuildingSelectionWidget::OnButton3Clicked);
	}
}

void UBuildingSelectionWidget::OnButton1Clicked()
{
	if (URace_base* RaceCDO = GetLocalPlayerRaceCDO())
	{
		const TArray<TSubclassOf<ABuildingBase>>& Buildings = RaceCDO->GetAvailableBuildings();
		if (Buildings.Num() > 0)
		{
			buildingclass = Buildings[0];
			UE_LOG(LogTemp, Display, TEXT("Button 1 selected building: %s"), *buildingclass->GetName());
			OnBuildingSelected.Broadcast(buildingclass);
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("No available building found for Button 1!"));
}

void UBuildingSelectionWidget::OnButton2Clicked()
{
	if (URace_base* RaceCDO = GetLocalPlayerRaceCDO())
	{
		const TArray<TSubclassOf<ABuildingBase>>& Buildings = RaceCDO->GetAvailableBuildings();
		if (Buildings.Num() > 1)
		{
			buildingclass = Buildings[1];
			UE_LOG(LogTemp, Display, TEXT("Button 2 selected building: %s"), *buildingclass->GetName());
			OnBuildingSelected.Broadcast(buildingclass);
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("No available building found for Button 2!"));
}
void UBuildingSelectionWidget::OnButton3Clicked()
{
	UE_LOG(LogTemp, Display, TEXT("Button 3 was clicked!"));

	if (URace_base* RaceCDO = GetLocalPlayerRaceCDO())
	{
		const TArray<TSubclassOf<ABuildingBase>>& Buildings = RaceCDO->GetAvailableBuildings();
		if (Buildings.Num() > 2)
		{
			buildingclass = Buildings[2];
			UE_LOG(LogTemp, Display, TEXT("Button 3 selected building: %s"), *buildingclass->GetName());
			OnBuildingSelected.Broadcast(buildingclass);
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("No available building found for Button 3!"));
}
URace_base* UBuildingSelectionWidget::GetLocalPlayerRaceCDO() const
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (ACustomPlayerState* PS = PC->GetPlayerState<ACustomPlayerState>())
		{
			if (TSubclassOf<URace_base> RaceClass = PS->GetPlayerRace())
			{
				return RaceClass->GetDefaultObject<URace_base>();
			}
		}
	}
	return nullptr;
}
