#pragma once
#include "UObject/NameTypes.h"
#include "Resourcenames.generated.h"



UENUM(BlueprintType)
enum class EResourceKind : uint8
{
	wood UMETA(DisplayName = "wood"),
	water_small UMETA(DisplayName = "small water barrel"),
	Elvish_bow UMETA(DisplayName = "Elvish bow"),
	Sword UMETA(DisplayName = "Sword"),
	
	MAX UMETA(Hidden)
};
// Static utility class for resource management
UCLASS(BlueprintType)
class GAME_V0_API UGameResources : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Convert enum to FName using reflection
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Resources")
	static FName ResourceKindToName(EResourceKind ResourceKind);
    
	// Convert FName to enum using reflection
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Resources")
	static EResourceKind NameToResourceKind(const FName& ResourceName);
    
	// Get all resource kinds as array (automatically includes new ones)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Resources")
	static TArray<EResourceKind> GetAllResourceKinds();
    
	// Get all resource names as array (automatically includes new ones)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Resources")
	static TArray<FName> GetAllResourceNames();
    
	// Check if resource kind is valid
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Resources")
	static bool IsValidResourceKind(EResourceKind ResourceKind);
    
	// Check if resource name is valid
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Resources")
	static bool IsValidResourceName(const FName& ResourceName);

private:
	// Helper to get the enum's UEnum pointer
	static const UEnum* GetResourceKindEnum();
};