// ResourceMeshConfig.cpp
#include "ResourceMeshConfig.h"
#include "Resourcenames.h"
#include "resource.h"

// Implementation of FResourceMeshConfig::GetMeshForProperties
TSoftObjectPtr<UStaticMesh> FResourceMeshConfig::GetMeshForProperties(const TMap<FName, float>& ResourceProperties) const
{
    if (Meshes.Num() == 0)
    {
        return nullptr;
    }

    // Get manager instance to access calculator functions
    // Note: In real implementation, you might want to pass manager as parameter or make this a member function
    // For now, we'll implement the logic directly here or get it through a singleton/subsystem
    
    int32 Index = 0; // Default to first mesh
    
    // This would typically call the calculator function
    // For now, return the first mesh as default
    Index = FMath::Clamp(Index, 0, Meshes.Num() - 1);
    return Meshes[Index];
}

// Manager Implementation
UResourceMeshManager::UResourceMeshManager()
{
    InitializeCalculatorFunctions();
    InitializeDefaultConfigs();
}

TSoftObjectPtr<UStaticMesh> UResourceMeshManager::GetResourceMesh(const FResource& Resource) const
{
    return GetMeshForResource(Resource.ResourceName, Resource.ResourceProperties);
}
/*
TSoftObjectPtr<UStaticMesh> UResourceMeshManager::GetMeshForResource(FName ResourceName, const TMap<FName, float>& Properties) const
{
    const FResourceMeshConfig* Config = MeshConfigs.Find(ResourceName);
    if (!Config)
    {
        UE_LOG(LogTemp, Warning, TEXT("No mesh config found for resource: %s"), *ResourceName.ToString());
        return nullptr;
    }

    if (Config->Meshes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No meshes configured for resource: %s"), *ResourceName.ToString());
        return nullptr;
    }

    // Get the calculator function for this resource
    const FResourceCalculatorFunction* CalculatorFunc = CalculatorFunctions.Find(ResourceName);
    
    int32 Index = 0; // Default to first mesh
    
    if (CalculatorFunc && *CalculatorFunc)
    {
        // Call the calculator function with properties and config
        Index = (*CalculatorFunc)(Properties, Config->CalculatorConfig);
        UE_LOG(LogTemp, VeryVerbose, TEXT("Calculator for %s returned index %d"), *ResourceName.ToString(), Index);
    }
    else
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("No calculator found for %s, using default mesh"), *ResourceName.ToString());
    }

    // Clamp index to valid range
    Index = FMath::Clamp(Index, 0, Config->Meshes.Num() - 1);
    
    return Config->Meshes[Index];
}
*/
TSoftObjectPtr<UStaticMesh> UResourceMeshManager::GetMeshForResource(FName ResourceName, const TMap<FName, float>& Properties) const
{
    UE_LOG(LogTemp, Warning, TEXT("=== GetMeshForResource: %s ==="), *ResourceName.ToString());
    UE_LOG(LogTemp, Warning, TEXT("MeshConfigs count: %d"), MeshConfigs.Num());
    
    // Log all available configs
    for (const auto& ConfigPair : MeshConfigs)
    {
        UE_LOG(LogTemp, Warning, TEXT("Available config: %s"), *ConfigPair.Key.ToString());
    }
    
    const FResourceMeshConfig* Config = MeshConfigs.Find(ResourceName);
    if (!Config)
    {
        UE_LOG(LogTemp, Error, TEXT("No mesh config found for resource: %s"), *ResourceName.ToString());
        return nullptr;
    }

    UE_LOG(LogTemp, Warning, TEXT("Found config with %d meshes"), Config->Meshes.Num());
    
    if (Config->Meshes.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("No meshes configured for resource: %s"), *ResourceName.ToString());
        return nullptr;
    }

    // Get the calculator function for this resource
    const FResourceCalculatorFunction* CalculatorFunc = CalculatorFunctions.Find(ResourceName);
    
    int32 Index = 0; // Default to first mesh
    
    if (CalculatorFunc && *CalculatorFunc)
    {
        UE_LOG(LogTemp, Warning, TEXT("Using calculator function for %s"), *ResourceName.ToString());
        Index = (*CalculatorFunc)(Properties, Config->CalculatorConfig);
        UE_LOG(LogTemp, Warning, TEXT("Calculator returned index %d"), Index);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No calculator found for %s, using index 0"), *ResourceName.ToString());
    }

    // Clamp index to valid range
    Index = FMath::Clamp(Index, 0, Config->Meshes.Num() - 1);
    UE_LOG(LogTemp, Warning, TEXT("Final clamped index: %d"), Index);
    
    TSoftObjectPtr<UStaticMesh> ResultMesh = Config->Meshes[Index];
    UE_LOG(LogTemp, Warning, TEXT("Returning mesh path: %s"), *ResultMesh.ToSoftObjectPath().ToString());
    
    return ResultMesh;
}
void UResourceMeshManager::SetResourceMeshConfig(const FResourceMeshConfig& Config)
{
    MeshConfigs.Add(Config.ResourceName, Config);
    UE_LOG(LogTemp, Log, TEXT("Updated mesh config for resource: %s with %d meshes"), 
        *Config.ResourceName.ToString(), Config.Meshes.Num());
}

void UResourceMeshManager::RegisterCalculatorFunction(FName ResourceName, const FResourceCalculatorFunction& Function)
{
    CalculatorFunctions.Add(ResourceName, Function);
    UE_LOG(LogTemp, Log, TEXT("Registered calculator function for resource: %s"), *ResourceName.ToString());
}

const FResourceCalculatorFunction* UResourceMeshManager::GetCalculatorFunction(FName ResourceName) const
{
    return CalculatorFunctions.Find(ResourceName);
}

// ===========================================
// CALCULATOR FUNCTION DEFINITIONS
// ===========================================

namespace ResourceCalculators
{
    // Sword Calculator: Returns 0-3 based on damage and combat effectiveness
    static int32 CalculateSwordMesh(const TMap<FName, float>& Props, const FResourceCalculatorConfig& Config)
    {
        const float* Damage = Props.Find(FName("Damage"));
        if (!Damage) return 0;
        
        const float* Speed = Props.Find(FName("Speed"));
        
        float Tier1 = Config.Parameters.FindRef(FName("Tier1DamageThreshold"));
        float Tier2 = Config.Parameters.FindRef(FName("Tier2DamageThreshold"));
        float Combat = Config.Parameters.FindRef(FName("CombatThreshold"));
        
        if (Tier1 <= 0.0f) Tier1 = 7.0f;
        if (Tier2 <= 0.0f) Tier2 = 10.0f;
        if (Combat <= 0.0f) Combat = 35.0f;
        
        if (Speed && (*Damage * *Speed) >= Combat) return 3; // Elite
        if (*Damage >= Tier2) return 2; // Masterwork
        if (*Damage >= Tier1) return 1; // Quality
        return 0; // Basic
    }
    
    // Bow Calculator: Returns 0-1 based on range
    static int32 CalculateBowMesh(const TMap<FName, float>& Props, const FResourceCalculatorConfig& Config)
    {
        const float* Range = Props.Find(FName("Range"));
        if (!Range) return 0;
        
        float Threshold = Config.Parameters.FindRef(FName("LongRangeThreshold"));
        if (Threshold <= 0.0f) Threshold = 15.0f;
        
        return (*Range >= Threshold) ? 1 : 0;
    }
    
    // Armor Calculator: Returns 0-2 based on defense
    static int32 CalculateArmorMesh(const TMap<FName, float>& Props, const FResourceCalculatorConfig& Config)
    {
        const float* Defense = Props.Find(FName("Defense"));
        if (!Defense) return 0;
        
        float Heavy = Config.Parameters.FindRef(FName("HeavyThreshold"));
        float Medium = Config.Parameters.FindRef(FName("MediumThreshold"));
        
        if (Heavy <= 0.0f) Heavy = 50.0f;
        if (Medium <= 0.0f) Medium = 25.0f;
        
        if (*Defense >= Heavy) return 2;
        if (*Defense >= Medium) return 1;
        return 0;
    }
    
    // Potion Calculator: Returns 0-2 based on complex formula
    static int32 CalculatePotionMesh(const TMap<FName, float>& Props, const FResourceCalculatorConfig& Config)
    {
        const float* Potency = Props.Find(FName("Potency"));
        const float* Purity = Props.Find(FName("Purity"));
        const float* Rarity = Props.Find(FName("Rarity"));
        
        if (!Potency || !Purity) return 0;
        
        float Score = (*Potency * 0.5f) + (*Purity * 0.3f);
        if (Rarity) Score += (*Rarity * 0.2f);
        
        float Legendary = Config.Parameters.FindRef(FName("LegendaryThreshold"));
        float Superior = Config.Parameters.FindRef(FName("SuperiorThreshold"));
        
        if (Legendary <= 0.0f) Legendary = 80.0f;
        if (Superior <= 0.0f) Superior = 50.0f;
        
        if (Score >= Legendary) return 2;
        if (Score >= Superior) return 1;
        return 0;
    }
    
    // Generic threshold calculator
    static int32 CalculateThresholdMesh(const TMap<FName, float>& Props, const FResourceCalculatorConfig& Config)
    {
        // Get property name from string parameters
        FString PropertyNameStr = Config.StringParameters.FindRef(FName("PropertyName"));
        if (PropertyNameStr.IsEmpty()) return 0;
        
        FName PropertyName = FName(*PropertyNameStr);
        const float* Value = Props.Find(PropertyName);
        if (!Value) return 0;
        
        TArray<float> Thresholds;
        for (int32 i = 0; i < 10; i++) // Support up to 10 thresholds
        {
            FName ThresholdKey = FName(*FString::Printf(TEXT("Threshold%d"), i));
            float Threshold = Config.Parameters.FindRef(ThresholdKey);
            if (Threshold > 0.0f) Thresholds.Add(Threshold);
            else break;
        }
        
        for (int32 i = Thresholds.Num() - 1; i >= 0; i--)
        {
            if (*Value >= Thresholds[i]) return i + 1;
        }
        
        return 0;
    }
}

void UResourceMeshManager::InitializeCalculatorFunctions()
{
    // ===========================================
    // CALCULATOR REGISTRATION TABLE
    // ===========================================
    
    struct FCalculatorRegistration
    {
        EResourceKind ResourceType;
        FResourceCalculatorFunction Function;
        FString Name;
    };
    
    TArray<FCalculatorRegistration> Calculators = {
        { EResourceKind::Sword,      ResourceCalculators::CalculateSwordMesh,     TEXT("Sword Mesh Calculator") },
        { EResourceKind::Elvish_bow, ResourceCalculators::CalculateBowMesh,       TEXT("Bow Mesh Calculator") },
        // { EResourceKind::Armor,      ResourceCalculators::CalculateArmorMesh,     TEXT("Armor Mesh Calculator") },
        // { EResourceKind::Potion,     ResourceCalculators::CalculatePotionMesh,    TEXT("Potion Mesh Calculator") },
    };
    
    for (const auto& Calc : Calculators)
    {
        FName ResourceName = UGameResources::ResourceKindToName(Calc.ResourceType);
        RegisterCalculatorFunction(ResourceName, Calc.Function);
        
        UE_LOG(LogTemp, Log, TEXT("Registered %s for resource %s"), 
            *Calc.Name, *ResourceName.ToString());
    }
    
    UE_LOG(LogTemp, Log, TEXT("ResourceMeshManager: Initialized %d calculator functions"), Calculators.Num());
}

void UResourceMeshManager::InitializeDefaultConfigs()
{
    // ===========================================
    // SWORD CONFIGURATION
    // ===========================================
    FResourceMeshConfig SwordConfig;
    SwordConfig.ResourceName = UGameResources::ResourceKindToName(EResourceKind::Sword);
    
    // Configure sword calculator parameters
    SwordConfig.CalculatorConfig = FResourceCalculatorConfig(
        SwordConfig.ResourceName,
        3, // MaxIndex (0-3)
        TEXT("Sword Tier Calculator"),
        TEXT("Calculates sword mesh based on damage and combat effectiveness (damage*speed)")
    );
    
    // Set configurable parameters
    SwordConfig.CalculatorConfig.Parameters.Add(FName("Tier1DamageThreshold"), 7.0f);  // Quality sword
    SwordConfig.CalculatorConfig.Parameters.Add(FName("Tier2DamageThreshold"), 10.0f); // Masterwork sword
    SwordConfig.CalculatorConfig.Parameters.Add(FName("CombatThreshold"), 35.0f);      // Elite combat sword (damage*speed)
    
    // Add sword meshes in order: Basic(0), Quality(1), Masterwork(2), Elite(3)
    SwordConfig.Meshes.Add(TSoftObjectPtr<UStaticMesh>(
        FSoftObjectPath(TEXT("/Game/Resources/WeaponMeshes/ElfSword_Mesh.ElfSword_Mesh"))
    ));
    SwordConfig.Meshes.Add(TSoftObjectPtr<UStaticMesh>(
        FSoftObjectPath(TEXT("/Game/Resources/Meshes/Sword_Quality.Sword_Quality"))
    ));
    SwordConfig.Meshes.Add(TSoftObjectPtr<UStaticMesh>(
        FSoftObjectPath(TEXT("/Game/Resources/Meshes/Sword_Masterwork.Sword_Masterwork"))
    ));
    SwordConfig.Meshes.Add(TSoftObjectPtr<UStaticMesh>(
        FSoftObjectPath(TEXT("/Game/Resources/Meshes/Sword_Elite.Sword_Elite"))
    ));
    
    MeshConfigs.Add(SwordConfig.ResourceName, SwordConfig);
    
    // ===========================================
    // BOW CONFIGURATION
    // ===========================================
    FResourceMeshConfig BowConfig;
    BowConfig.ResourceName = UGameResources::ResourceKindToName(EResourceKind::Elvish_bow);
    
    BowConfig.CalculatorConfig = FResourceCalculatorConfig(
        BowConfig.ResourceName,
        1, // MaxIndex (0-1)
        TEXT("Bow Range Calculator"),
        TEXT("Calculates bow mesh based on range property")
    );
    
    BowConfig.CalculatorConfig.Parameters.Add(FName("LongRangeThreshold"), 15.0f);
    
    // Add bow meshes: Standard(0), Longbow(1)
    BowConfig.Meshes.Add(TSoftObjectPtr<UStaticMesh>(
        FSoftObjectPath(TEXT("/Game/Resources/Meshes/ElvishBow_Standard.ElvishBow_Standard"))
    ));
    BowConfig.Meshes.Add(TSoftObjectPtr<UStaticMesh>(
        FSoftObjectPath(TEXT("/Game/Resources/Meshes/ElvishBow_Longbow.ElvishBow_Longbow"))
    ));
    
    MeshConfigs.Add(BowConfig.ResourceName, BowConfig);
    
    // ===========================================
    // SIMPLE RESOURCE CONFIGURATIONS
    // ===========================================
    
    // Wood (no variants)
    FResourceMeshConfig WoodConfig;
    WoodConfig.ResourceName = UGameResources::ResourceKindToName(EResourceKind::wood);
    WoodConfig.Meshes.Add(TSoftObjectPtr<UStaticMesh>(
        FSoftObjectPath(TEXT("/Game/Resources/Wood_Mesh.Wood_Mesh"))
    ));
    MeshConfigs.Add(WoodConfig.ResourceName, WoodConfig);

    // Water (no variants)
    FResourceMeshConfig WaterConfig;
    WaterConfig.ResourceName = UGameResources::ResourceKindToName(EResourceKind::water_small);
    WaterConfig.Meshes.Add(TSoftObjectPtr<UStaticMesh>(
        FSoftObjectPath(TEXT("/Game/Resources/Meshes/WaterBarrel_Small.WaterBarrel_Small"))
    ));
    MeshConfigs.Add(WaterConfig.ResourceName, WaterConfig);

    UE_LOG(LogTemp, Log, TEXT("ResourceMeshManager: Initialized %d mesh configurations"), MeshConfigs.Num());
}