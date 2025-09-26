#pragma once
#include "BuildingBase.h"
#include "BlacksmithWorkshop_Elves.generated.h"

UCLASS()
class GAME_V0_API ABlacksmithWorkshop_Elves : public ABuildingBase
{
	GENERATED_BODY()
public:
	ABlacksmithWorkshop_Elves();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
};
