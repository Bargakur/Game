#pragma once

#include "BuildingBase.h"
#include "BigPotionWorkshop_Elves.generated.h"
UCLASS()
class GAME_V0_API ABigPotionWorkshop_Elves: public ABuildingBase
{
	GENERATED_BODY()
public:
	ABigPotionWorkshop_Elves();

	virtual void BeginPlay() override;


	virtual void Tick(float DeltaTime) override;
	
};
