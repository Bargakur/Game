#include "UnitBase.h"
#include "UnitController.h"
#include "UnitSelectionManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AUnitBase::AUnitBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize defaults
    TeamId = 0;
    Age = 0;
    Strength = 10.0f;
    SeeingRange = 800.0f;
    UnitMovementSpeed = 200.0f;
    bIsSelected = false;
    bIsMoving = false;
    SelectionManager = nullptr;

    SetupCollision();
    SetupMovement();
    SetupSelectionIndicator();

    AIControllerClass = AUnitController::StaticClass();

    UnitMesh = TSoftObjectPtr<USkeletalMesh>(
       FSoftObjectPath(TEXT("/Game/Unit_Mesh_Anim/MaleElf_Mesh.MaleElf_Mesh"))
   );

    SelectionMesh = TSoftObjectPtr<UStaticMesh>(
        FSoftObjectPath(TEXT("/Game/Unit_Mesh_Anim/Walking__Mesh1_0.Walking__Mesh1_0"))
    );
    static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPClass(
    TEXT("/Game/Anim_BPs/ABP_Unit.ABP_Unit_C"));
    if (AnimBPClass.Succeeded())
    {
        GetMesh()->SetAnimInstanceClass(AnimBPClass.Class);
    }
#if WITH_EDITOR
    // Force-load immediately for editor preview
    if (UnitMesh.IsValid() || UnitMesh.ToSoftObjectPath().IsValid())
    {
        USkeletalMesh* LoadedMesh = UnitMesh.LoadSynchronous();
        if (LoadedMesh && GetMesh())
        {
            GetMesh()->SetSkeletalMesh(LoadedMesh);
        }
    }

    if (SelectionMesh.IsValid() || SelectionMesh.ToSoftObjectPath().IsValid())
    {
        UStaticMesh* LoadedMesh = SelectionMesh.LoadSynchronous();
        if (LoadedMesh && SelectionIndicator)
        {
            SelectionIndicator->SetStaticMesh(LoadedMesh);
        }
    }
#endif
   
    
}

void AUnitBase::BeginPlay()
{
    Super::BeginPlay();

    LoadUnitMesh();
    LoadSelectionMesh();

    UpdateSelectionVisual();
}

void AUnitBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AUnitBase, CarriedResources);
}

void AUnitBase::SetupCollision()
{
    UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
    if (CapsuleComp)
    {
        // Adjust capsule size to better match your unit
        CapsuleComp->SetCapsuleSize(40.0f, 88.0f); // Radius, Half-Height
        CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        CapsuleComp->SetCollisionObjectType(ECC_Pawn);
        CapsuleComp->SetCollisionResponseToAllChannels(ECR_Block);
        CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        CapsuleComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
        
    }
    
    // Adjust mesh position to align with capsule
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (MeshComp)
    {
        // Move mesh down so feet are at capsule bottom
        MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
        MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f)); // Adjust if needed
    }
}

void AUnitBase::SetupMovement()
{
    UCharacterMovementComponent* MovementComp = GetCharacterMovement();
    if (MovementComp)
    {
        MovementComp->MaxWalkSpeed = UnitMovementSpeed;
        MovementComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
        MovementComp->bOrientRotationToMovement = true;
        MovementComp->bUseControllerDesiredRotation = false;
        MovementComp->GetNavAgentPropertiesRef().bCanCrouch = false;
        MovementComp->GetNavAgentPropertiesRef().bCanJump = false;
        MovementComp->GetNavAgentPropertiesRef().bCanFly = false;
    }
}

void AUnitBase::SetupSelectionIndicator()
{
    SelectionIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectionIndicator"));
    SelectionIndicator->SetupAttachment(RootComponent);

    SelectionIndicator->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
    SelectionIndicator->SetRelativeScale3D(FVector(2.0f, 2.0f, 0.1f));
    SelectionIndicator->SetVisibility(false);
    SelectionIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AUnitBase::LoadUnitMesh()
{
    if (UnitMesh.IsValid() || UnitMesh.ToSoftObjectPath().IsValid())
    {
        FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
        Streamable.RequestAsyncLoad(UnitMesh.ToSoftObjectPath(),
            [this]()
            {
                if (UnitMesh.IsValid() && GetMesh())
                {
                    GetMesh()->SetSkeletalMesh(UnitMesh.Get());
                }
            });
    }
}

void AUnitBase::LoadSelectionMesh()
{
    if (SelectionMesh.IsValid() || SelectionMesh.ToSoftObjectPath().IsValid())
    {
        FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
        Streamable.RequestAsyncLoad(SelectionMesh.ToSoftObjectPath(),
            [this]()
            {
                if (SelectionMesh.IsValid() && SelectionIndicator)
                {
                    SelectionIndicator->SetStaticMesh(SelectionMesh.Get());
                }
            });
    }
}

void AUnitBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateAnimationState();
}

void AUnitBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AUnitBase::SetIsSelected(bool bSelected)
{
    if (bIsSelected != bSelected)
    {
        bIsSelected = bSelected;
        UpdateSelectionVisual();

        UE_LOG(LogTemp, Log, TEXT("Unit %s selection state changed to: %s"),
            *GetName(), bSelected ? TEXT("Selected") : TEXT("Deselected"));
    }
}

void AUnitBase::SetIsMoving(bool bMoving)
{
    if (bIsMoving != bMoving)
    {
        bIsMoving = bMoving;
        bShouldMove = bMoving;
        UpdateAnimationState();

        UE_LOG(LogTemp, VeryVerbose, TEXT("Unit %s movement state changed to: %s"),
            *GetName(), bMoving ? TEXT("Moving") : TEXT("Idle"));
    }
}

void AUnitBase::InitializeUnit(int32 InTeamId, AUnitSelectionManager* InSelectionManager)
{
    TeamId = InTeamId;
    SelectionManager = InSelectionManager;

    UE_LOG(LogTemp, Log, TEXT("Unit %s initialized with TeamId: %d"), *GetName(), TeamId);
}

void AUnitBase::UpdateSelectionVisual()
{
    if (SelectionIndicator)
    {
        SelectionIndicator->SetVisibility(bIsSelected);

        if (bIsSelected)
        {
            UMaterialInterface* BaseMaterial = SelectionIndicator->GetMaterial(0);
            if (BaseMaterial)
            {
                UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
                if (DynamicMaterial)
                {
                    DynamicMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor::Green);
                    SelectionIndicator->SetMaterial(0, DynamicMaterial);
                }
            }
        }
    }
}

void AUnitBase::UpdateAnimationState()
{
    UCharacterMovementComponent* MovementComp = GetCharacterMovement();
    if (MovementComp)
    {
        FVector Velocity = MovementComp->Velocity;
        MovementSpeed = Velocity.Size2D();
        
        float MovementThreshold = 10.0f;
        bShouldMove = MovementSpeed > MovementThreshold && bIsMoving;
        
        // FORCE ANIMATION UPDATE - Add this temporarily
        USkeletalMeshComponent* MeshComp = GetMesh();
        if (MeshComp && MeshComp->GetAnimInstance())
        {
            // Force the animation instance to refresh
            MeshComp->GetAnimInstance()->UpdateAnimation(GetWorld()->GetDeltaSeconds(), false);
        }
        /*
        UE_LOG(LogTemp, Warning, TEXT("Unit %s: Speed=%.2f, bIsMoving=%s, bShouldMove=%s"),
            *GetName(), 
            MovementSpeed, 
            bIsMoving ? TEXT("true") : TEXT("false"),
            bShouldMove ? TEXT("true") : TEXT("false"));
            */
    }
}

void AUnitBase::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // Ensure meshes are applied when dropped into world at runtime
    if (UnitMesh.IsValid())
    {
        if (USkeletalMesh* LoadedMesh = UnitMesh.LoadSynchronous())
        {
            GetMesh()->SetSkeletalMesh(LoadedMesh);
        }
    }

    if (SelectionMesh.IsValid())
    {
        if (UStaticMesh* LoadedMesh = SelectionMesh.LoadSynchronous())
        {
            SelectionIndicator->SetStaticMesh(LoadedMesh);
        }
    }
}

bool AUnitBase::CanPickupResource(const FResource& Resource) const
{
    // Check slot capacity
    if (CarriedResources.Num() >= MaxCarrySlots)
    {
        return false;
    }
    
    // Check weight capacity
    float CurrentWeight = GetCurrentCarryWeight();
    if (CurrentWeight + Resource.Weight > MaxCarryWeight)
    {
        return false;
    }
    
    return true;
}

void AUnitBase::ServerPickupResource_Implementation(APhysicalResourceActor* ResourceActor)
{
    if (!ResourceActor || !HasAuthority())
    {
        return;
    }
    
    // Get resource data from the physical actor
    FName ResourceName;
    int32 ResourceAmount;
    float ResourceWeight;
    TMap<FName, float> ResourceProperties;
    ResourceActor->GetResourceData(ResourceName, ResourceAmount, ResourceWeight, ResourceProperties);
    
    // Create FResource object
    FResource ResourceData(ResourceName, ResourceAmount, ResourceWeight);
    ResourceData.ResourceProperties = ResourceProperties;
    
    // Validate pickup
    if (!CanPickupResource(ResourceData) || !ResourceActor->CanBePickedUpBy(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot pickup resource: capacity or state issue"));
        return;
    }
    
    // Add to carried resources
    ResourceData.SetCarriedBy(this);
    CarriedResources.Add(ResourceData);
    
    // Destroy the physical actor
    ResourceActor->Destroy();
    
    UE_LOG(LogTemp, Log, TEXT("Unit %s picked up resource: %s"), 
           *GetName(), *ResourceData.ResourceName.ToString());
    
    UpdateCarriedResourceVisuals();
}

void AUnitBase::ServerDropResource_Implementation(int32 ResourceIndex)
{
    if (!HasAuthority() || ResourceIndex < 0 || ResourceIndex >= CarriedResources.Num())
    {
        return;
    }
    
    FResource ResourceToDrop = CarriedResources[ResourceIndex];
    CarriedResources.RemoveAt(ResourceIndex);
    
    // Spawn physical resource in world
    FVector DropLocation = GetActorLocation() + GetActorForwardVector() * 100.0f;
    
    if (UWorld* World = GetWorld())
    {
        APhysicalResourceActor* PhysicalResource = World->SpawnActor<APhysicalResourceActor>(
            APhysicalResourceActor::StaticClass(),
            DropLocation,
            FRotator::ZeroRotator
        );
        
        if (PhysicalResource)
        {
            ResourceToDrop.SetOnGround(DropLocation);
            PhysicalResource->Initialize(ResourceToDrop);
            
            UE_LOG(LogTemp, Log, TEXT("Unit %s dropped resource: %s"), 
                   *GetName(), *ResourceToDrop.ResourceName.ToString());
        }
    }
    
    UpdateCarriedResourceVisuals();
}

void AUnitBase::ServerDropAllResources_Implementation()
{
    if (!HasAuthority())
    {
        return;
    }
    
    for (int32 i = CarriedResources.Num() - 1; i >= 0; i--)
    {
        ServerDropResource(i);
    }
}

APhysicalResourceActor* AUnitBase::FindNearestResource(FName ResourceName, float SearchRadius)
{
    if (!GetWorld())
    {
        return nullptr;
    }
    
    TArray<AActor*> FoundActors;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetActorLocation(),
        SearchRadius,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        APhysicalResourceActor::StaticClass(),
        TArray<AActor*>(),
        FoundActors
    );
    
    APhysicalResourceActor* NearestResource = nullptr;
    float NearestDistance = SearchRadius;
    
    for (AActor* Actor : FoundActors)
    {
        if (APhysicalResourceActor* ResourceActor = Cast<APhysicalResourceActor>(Actor))
        {
            // Get resource data to check the name
            if (ResourceName != NAME_None)
            {
                FName ActorResourceName;
                int32 ActorResourceAmount;
                float ActorResourceWeight;
                TMap<FName, float> ActorResourceProperties;
                ResourceActor->GetResourceData(ActorResourceName, ActorResourceAmount, ActorResourceWeight, ActorResourceProperties);
                
                // Check if this is the resource we're looking for
                if (ActorResourceName != ResourceName)
                {
                    continue;
                }
            }
            
            // Check if it can be picked up
            if (!ResourceActor->CanBePickedUpBy(this))
            {
                continue;
            }
            
            float Distance = FVector::Dist(GetActorLocation(), ResourceActor->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestResource = ResourceActor;
            }
        }
    }
    
    return NearestResource;
}

int32 AUnitBase::GetCarriedAmount(FName ResourceName) const
{
    int32 TotalAmount = 0;
    for (const FResource& Resource : CarriedResources)
    {
        if (Resource.ResourceName == ResourceName)
        {
            TotalAmount += Resource.ResourceAmount;
        }
    }
    return TotalAmount;
}

bool AUnitBase::HasResourceType(FName ResourceName) const
{
    for (const FResource& Resource : CarriedResources)
    {
        if (Resource.ResourceName == ResourceName)
        {
            return true;
        }
    }
    return false;
}

float AUnitBase::GetCurrentCarryWeight() const
{
    float TotalWeight = 0.0f;
    for (const FResource& Resource : CarriedResources)
    {
        TotalWeight += Resource.Weight * Resource.ResourceAmount;
    }
    return TotalWeight;
}

void AUnitBase::OnRep_CarriedResources()
{
    UpdateCarriedResourceVisuals();
}

void AUnitBase::UpdateCarriedResourceVisuals()
{
    // TODO: Update visual representation of carried resources
    // Could attach small mesh components to the unit to show what they're carrying
    UE_LOG(LogTemp, VeryVerbose, TEXT("Unit %s carrying %d resources (Weight: %.1f/%.1f)"), 
           *GetName(), CarriedResources.Num(), GetCurrentCarryWeight(), MaxCarryWeight);
}



