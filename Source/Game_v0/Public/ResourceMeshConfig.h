// ResourceMeshConfig.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "UObject/SoftObjectPtr.h"
#include "ResourceMeshConfig.generated.h"

// Base class for resource mesh calculators
UCLASS(BlueprintType, Abstract)
class GAME_V0_API UResourceMeshCalculator : public UObject
{
    GENERATED_BODY()

public:
    // Pure virtual function to calculate mesh index
    virtual int32 CalculateMeshIndex(const TMap<FName, float>& Properties) const PURE_VIRTUAL(UResourceMeshCalculator::CalculateMeshIndex, return 0;);
    
    // Get the maximum possible index this calculator can return
    virtual int32 GetMaxIndex() const PURE_VIRTUAL(UResourceMeshCalculator::GetMaxIndex, return 0;);
    
    // Blueprint implementable version
    UFUNCTION(BlueprintImplementableEvent, Category = "Resource Mesh")
    int32 CalculateMeshIndexBP(const TMap<FName, float>& Properties) const;
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Resource Mesh")
    int32 GetMaxIndexBP() const;
};

// Template base class for type-safe calculators with metaprogramming
template<typename TEnum>
class GAME_V0_API TResourceMeshCalculator : public UResourceMeshCalculator
{
public:
    // Template specialization will be defined for each resource type
    virtual int32 CalculateIndex(const TMap<FName, float>& Properties) const = 0;
    
    // Implement base class method
    virtual int32 CalculateMeshIndex(const TMap<FName, float>& Properties) const override final
    {
        int32 Index = CalculateIndex(Properties);
        return FMath::Clamp(Index, 0, GetMaxIndex());
    }
};

// Structure to define calculator functions and their configuration
USTRUCT(BlueprintType)
struct GAME_V0_API FResourceCalculatorConfig
{
    GENERATED_BODY()

    // The resource this calculator applies to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    FName ResourceName;
    
    // Maximum index this calculator can return
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    int32 MaxIndex = 0;
    
    // Display name for this calculator
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    FString CalculatorName;
    
    // Description of what this calculator does
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    FString Description;
    
    // Configurable parameters (threshold values, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    TMap<FName, float> Parameters;
    
    // String parameters for more complex configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    TMap<FName, FString> StringParameters;
    
    FResourceCalculatorConfig()
        : ResourceName(NAME_None)
        , MaxIndex(0)
        , CalculatorName(TEXT("Default"))
        , Description(TEXT("Default calculator"))
    {}
    
    FResourceCalculatorConfig(FName InResourceName, int32 InMaxIndex, const FString& InName, const FString& InDesc)
        : ResourceName(InResourceName)
        , MaxIndex(InMaxIndex)  
        , CalculatorName(InName)
        , Description(InDesc)
    {}
};

// Generic threshold calculator for simple resources
UCLASS(BlueprintType)
class GAME_V0_API UThresholdMeshCalculator : public UResourceMeshCalculator
{
    GENERATED_BODY()

public:

    
    virtual int32 GetMaxIndex() const override { return Thresholds.Num(); }
    
    // Property to check
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threshold")
    FName PropertyName;
    
    // Threshold values - returns index based on which threshold is exceeded
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threshold")
    TArray<float> Thresholds;
};

// Function pointer calculator for maximum flexibility
UCLASS(BlueprintType)
class GAME_V0_API UFunctionPtrMeshCalculator : public UResourceMeshCalculator
{
    GENERATED_BODY()

public:
    
    virtual int32 GetMaxIndex() const override { return MaxIndex; }
    
    // Function pointer for custom calculation
    TFunction<int32(const TMap<FName, float>&)> CalculationFunction;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
    int32 MaxIndex = 0;
    
    // Set custom function
    void SetCalculationFunction(TFunction<int32(const TMap<FName, float>&)> Function, int32 InMaxIndex)
    {
        CalculationFunction = Function;
        MaxIndex = InMaxIndex;
    }
};

USTRUCT(BlueprintType)
struct GAME_V0_API FResourceMeshConfig
{
    GENERATED_BODY()

    // Resource name this config applies to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FName ResourceName;

    // Ordered array of meshes - calculator returns index into this array
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TArray<TSoftObjectPtr<UStaticMesh>> Meshes;

    // Calculator configuration and parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calculator")
    FResourceCalculatorConfig CalculatorConfig;

    FResourceMeshConfig()
        : ResourceName(NAME_None)
    {}

    // Get the appropriate mesh for given properties
    TSoftObjectPtr<UStaticMesh> GetMeshForProperties(const TMap<FName, float>& ResourceProperties) const;
};

// Metaprogramming helper for registering calculators
template<typename TCalculator>
struct TCalculatorRegistry
{
    static TCalculator* GetCalculator()
    {
        static TCalculator* Instance = nullptr;
        if (!Instance)
        {
            Instance = NewObject<TCalculator>();
        }
        return Instance;
    }
};

// Macro for easy calculator registration
#define REGISTER_MESH_CALCULATOR(ResourceType, CalculatorClass) \
    template<> \
    struct TCalculatorRegistry<CalculatorClass> \
    { \
        static CalculatorClass* GetCalculator() \
        { \
            static CalculatorClass* Instance = NewObject<CalculatorClass>(); \
            return Instance; \
        } \
    };

// Type alias for calculator function
using FResourceCalculatorFunction = TFunction<int32(const TMap<FName, float>&, const FResourceCalculatorConfig&)>;

// Manager class for resource meshes with function-based calculators
UCLASS(BlueprintType, Blueprintable)
class GAME_V0_API UResourceMeshManager : public UObject
{
    GENERATED_BODY()

public:
    UResourceMeshManager();

    // Get mesh for a resource
    UFUNCTION(BlueprintCallable, Category = "Resource Mesh")
    TSoftObjectPtr<UStaticMesh> GetResourceMesh(const FResource& Resource) const;

    // Get mesh by name and properties
    UFUNCTION(BlueprintCallable, Category = "Resource Mesh")
    TSoftObjectPtr<UStaticMesh> GetMeshForResource(FName ResourceName, const TMap<FName, float>& Properties) const;

    // Add or update mesh config
    UFUNCTION(BlueprintCallable, Category = "Resource Mesh")
    void SetResourceMeshConfig(const FResourceMeshConfig& Config);

    // Register a calculator function for a resource type
    void RegisterCalculatorFunction(FName ResourceName, const FResourceCalculatorFunction& Function);

    // Get the calculator function for a resource
    const FResourceCalculatorFunction* GetCalculatorFunction(FName ResourceName) const;

protected:
    // Mesh configurations for each resource type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TMap<FName, FResourceMeshConfig> MeshConfigs;

    // Calculator functions stored by resource name
    TMap<FName, FResourceCalculatorFunction> CalculatorFunctions;

private:
    // Initialize default configurations and calculators
    void InitializeDefaultConfigs();
    void InitializeCalculatorFunctions();
};
