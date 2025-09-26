#include "UnitBase.h"
#include "UnitController.h"
#include "UnitSelectionManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

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



