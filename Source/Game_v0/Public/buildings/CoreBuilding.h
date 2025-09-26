#pragma once

#include "BuildingBase.h"
#include "CoreBuilding.generated.h"


UCLASS()

class GAME_V0_API ACoreBuilding : public ABuildingBase
{
	GENERATED_BODY()

	public:
	ACoreBuilding();


	protected:
	virtual void HandleBuildingDestroyed();
	
	virtual void BeginPlay() override;
};