#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ResourceMeshConfig.h"
#include "ResourceMeshSubsystem.generated.h"

UCLASS()
class GAME_V0_API UResourceMeshSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UResourceMeshManager* GetMeshManager() const { return MeshManager; }

private:
	UPROPERTY()
	UResourceMeshManager* MeshManager;
};