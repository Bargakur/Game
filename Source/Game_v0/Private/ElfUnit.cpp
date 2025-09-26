#include "ElfUnit.h"
#include "Engine/AssetManager.h"

AElfUnit::AElfUnit()
{
    // Call parent constructor first
    
    // Explicitly set male elf mesh path (override parent's default)
    UnitMesh = TSoftObjectPtr<USkeletalMesh>(
        FSoftObjectPath(TEXT("/Game/Unit_Mesh_Anim/MaleElf_Mesh.MaleElf_Mesh"))
    );
    
    // Set female elf mesh path
    FemaleElfMesh = TSoftObjectPtr<USkeletalMesh>(
        FSoftObjectPath(TEXT("/Game/Unit_Mesh_Anim/FemaleElf_Mesh.FemaleElf_Mesh"))
    );

    // Set female animation blueprint path
    FemaleAnimBP = TSoftClassPtr<UAnimInstance>(
        FSoftObjectPath(TEXT("/Game/Anim_BPs/ABP_FemaleElf.ABP_FemaleElf_C"))
    );

    // Default to male (this will be used by SelectElfMesh)
    UnitSex = EUnitSex::Male;

    // Explicitly load the appropriate mesh and animation BP based on sex in constructor for clarity
    TSoftObjectPtr<USkeletalMesh> SelectedMesh = SelectElfMesh();
    TSoftClassPtr<UAnimInstance> SelectedAnimBP = SelectElfAnimBP();
    
    UE_LOG(LogTemp, Log, TEXT("ElfUnit Constructor: Selected %s mesh and AnimBP for %s elf"), 
        UnitSex == EUnitSex::Female ? TEXT("Female") : TEXT("Male"),
        *GetName());
    
#if WITH_EDITOR
    // Force-load immediately for editor preview
    if (SelectedMesh.IsValid() || SelectedMesh.ToSoftObjectPath().IsValid())
    {
        USkeletalMesh* LoadedMesh = SelectedMesh.LoadSynchronous();
        if (LoadedMesh && GetMesh())
        {
            GetMesh()->SetSkeletalMesh(LoadedMesh);
            UE_LOG(LogTemp, Log, TEXT("ElfUnit Constructor: Successfully loaded mesh in editor"));
        }
    }
    
    // Load animation blueprint synchronously for editor
    if (SelectedAnimBP.IsValid() || SelectedAnimBP.ToSoftObjectPath().IsValid())
    {
        UClass* LoadedAnimBP = SelectedAnimBP.LoadSynchronous();
        if (LoadedAnimBP && GetMesh())
        {
            GetMesh()->SetAnimInstanceClass(LoadedAnimBP);
            UE_LOG(LogTemp, Log, TEXT("ElfUnit Constructor: Successfully loaded AnimBP in editor"));
        }
    }
#endif
}

void AElfUnit::BeginPlay()
{
    Super::BeginPlay();
    
    // Load the appropriate mesh and animation BP based on sex
    LoadElfMesh();
    LoadElfAnimBP();
}

void AElfUnit::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    
    // Ensure correct mesh is applied when dropped into world
    if (TSoftObjectPtr<USkeletalMesh> SelectedMesh = SelectElfMesh())
    {
        if (USkeletalMesh* LoadedMesh = SelectedMesh.LoadSynchronous())
        {
            GetMesh()->SetSkeletalMesh(LoadedMesh);
        }
    }
}

TSoftObjectPtr<USkeletalMesh> AElfUnit::SelectElfMesh() const
{
    switch (UnitSex)
    {
        case EUnitSex::Female:
            return FemaleElfMesh;
        case EUnitSex::Male:
        default:
            return UnitMesh; // Use parent's male mesh
    }
}

TSoftClassPtr<UAnimInstance> AElfUnit::SelectElfAnimBP() const
{
    switch (UnitSex)
    {
        case EUnitSex::Female:
            return FemaleAnimBP;
        case EUnitSex::Male:
        default:
            // Return null to use the default AnimBP set in parent constructor
            return nullptr;
    }
}

void AElfUnit::LoadElfMesh()
{
    TSoftObjectPtr<USkeletalMesh> SelectedMesh = SelectElfMesh();
    
    if (SelectedMesh.IsValid() || SelectedMesh.ToSoftObjectPath().IsValid())
    {
        FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
        Streamable.RequestAsyncLoad(SelectedMesh.ToSoftObjectPath(),
            [this, SelectedMesh]()
            {
                if (SelectedMesh.IsValid() && GetMesh())
                {
                    USkeletalMesh* LoadedMesh = SelectedMesh.Get();
                    if (LoadedMesh)
                    {
                        GetMesh()->SetSkeletalMesh(LoadedMesh);
                        
                        UE_LOG(LogTemp, Log, TEXT("ElfUnit %s: Loaded %s mesh"), 
                            *GetName(), 
                            UnitSex == EUnitSex::Female ? TEXT("Female") : TEXT("Male"));
                    }
                }
            });
    }
}

void AElfUnit::LoadElfAnimBP()
{
    TSoftClassPtr<UAnimInstance> SelectedAnimBP = SelectElfAnimBP();
    
    // Only load if we have a specific AnimBP for this sex (female)
    if (SelectedAnimBP.IsValid() || SelectedAnimBP.ToSoftObjectPath().IsValid())
    {
        FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
        Streamable.RequestAsyncLoad(SelectedAnimBP.ToSoftObjectPath(),
            [this, SelectedAnimBP]()
            {
                if (SelectedAnimBP.IsValid() && GetMesh())
                {
                    UClass* LoadedAnimBP = SelectedAnimBP.Get();
                    if (LoadedAnimBP)
                    {
                        GetMesh()->SetAnimInstanceClass(LoadedAnimBP);
                        
                        UE_LOG(LogTemp, Log, TEXT("ElfUnit %s: Loaded %s AnimBP"), 
                            *GetName(), 
                            UnitSex == EUnitSex::Female ? TEXT("Female") : TEXT("Male"));
                    }
                }
            });
    }
    else
    {
        // Use default AnimBP from parent (for male)
        UE_LOG(LogTemp, Log, TEXT("ElfUnit %s: Using default AnimBP for male"), *GetName());
    }
}