#include "ResourceMeshSubsystem.h"

void UResourceMeshSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	MeshManager = NewObject<UResourceMeshManager>(this);
	UE_LOG(LogTemp, Log, TEXT("ResourceMeshSubsystem initialized"));
}