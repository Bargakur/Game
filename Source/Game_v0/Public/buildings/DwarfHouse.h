#pragma once
#include "BuildingBase.h"
#include "DwarfHouse.generated.h"

UCLASS()
class GAME_V0_API ADwarfHouse : public ABuildingBase
{
	GENERATED_BODY()
public:
	ADwarfHouse();
	virtual void Tick(float DeltaTime) override;

	protected:
	virtual void BeginPlay() override;
};
