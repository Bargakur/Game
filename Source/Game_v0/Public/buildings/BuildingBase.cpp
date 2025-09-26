// Fill out your copyright notice in the Description page of Project Settings.

#include "BuildingBase.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ABuildingBase::ABuildingBase()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;
    
    // Create and set up the static mesh component
    BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
    RootComponent = BuildingMesh;

    
    static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/temp_1.temp_1")); // 
    if (DefaultMesh.Succeeded())
    {
        BuildingMesh->SetStaticMesh(DefaultMesh.Object);
    }
    
    // Set default values
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    TeamID = 0;
    BuildingName = TEXT("Base Building");
    BuildingDescription = FText::FromString(TEXT("Building base class."));
    ConstructionTime = 5.0f;
    bIsConstructed = false;
    
    // Enable replication
    bReplicates = true;
    BuildingMesh->SetIsReplicated(true);
}

// Called when the game starts or when spawned
void ABuildingBase::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize health
    CurrentHealth = MaxHealth;
}

// Called every frame
void ABuildingBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABuildingBase::SetBuildingMesh(UStaticMesh* NewMesh)
{
    if (NewMesh)
    {
        BuildingMesh->SetStaticMesh(NewMesh);
    }
}



float ABuildingBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // Only process damage on the server
    if (!HasAuthority())
    {
        return 0.0f;
    }
    
    // Don't take damage if already destroyed or not constructed
    if (bIsDestroyed || !bIsConstructed)
    {
        return 0.0f;
    }
    
    // Clamp damage to valid range
    float ActualDamage = FMath::Max(0.0f, DamageAmount);
    
    // Apply damage
    float OldHealth = CurrentHealth;
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);
    
    // Calculate actual damage dealt
    float DamageDealt = OldHealth - CurrentHealth;
    
    UE_LOG(LogTemp, Log, TEXT("BuildingBase: %s took %.1f damage (%.1f -> %.1f health)"), 
           *BuildingName, DamageDealt, OldHealth, CurrentHealth);
    
    // Check if building is destroyed
    if (CurrentHealth <= 0.0f && !bIsDestroyed)
    {
        bIsDestroyed = true;
        HandleBuildingDestroyed();
    }
    
    // Call parent implementation for any additional processing
    Super::TakeDamage(DamageDealt, DamageEvent, EventInstigator, DamageCauser);
    
    return DamageDealt;
}

void ABuildingBase::CompleteConstruction()
{
    bIsConstructed = true;
    
    // You might want to change the appearance or enable certain functionalities here
}

float ABuildingBase::GetHealthPercentage() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}
void ABuildingBase::HandleBuildingDestroyed()
{
    if (!HasAuthority())
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildingBase: %s has been destroyed!"), *BuildingName);
    
    // Broadcast destruction event
    OnBuildingDestroyed.Broadcast(this);
    
    
    // Note: We don't destroy the actor here - let the game mode decide what to do
}
void ABuildingBase::HealBuilding(float HealAmount)
{
    if (!HasAuthority() || bIsDestroyed)
    {
        return;
    }
    
    float OldHealth = CurrentHealth;
    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + FMath::Max(0.0f, HealAmount));
    
    float HealingDone = CurrentHealth - OldHealth;
    if (HealingDone > 0.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("BuildingBase: %s healed for %.1f (%.1f -> %.1f health)"), 
               *BuildingName, HealingDone, OldHealth, CurrentHealth);
    }
}

void ABuildingBase::OnRep_IsDestroyed()
{
    if (bIsDestroyed)
    {
        UE_LOG(LogTemp, Log, TEXT("BuildingBase: %s destruction replicated"), *BuildingName);
        
    }
}
void ABuildingBase::OnRep_IsConstructed()
{
    if (bIsConstructed)
    {
        UE_LOG(LogTemp, Log, TEXT("BuildingBase: %s construction replicated"), *BuildingName);
        // TODO: Play construction completion effects
    }
}
void ABuildingBase::OnRep_CurrentHealth()
{
    
}

void ABuildingBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // Replicate health to all clients
    DOREPLIFETIME(ABuildingBase, CurrentHealth);
    
    // Replicate construction state to all clients
    DOREPLIFETIME(ABuildingBase, bIsConstructed);
    
    // Replicate destroyed state to all clients
    DOREPLIFETIME(ABuildingBase, bIsDestroyed);
    
    // Replicate team ID to all clients
    DOREPLIFETIME(ABuildingBase, TeamID);

    DOREPLIFETIME(ABuildingBase, OwningPlayer);
}
void ABuildingBase::SetOwningPlayer(ACustomPlayerState* InOwningPlayer)
{
    if (HasAuthority())
    {
        OwningPlayer = InOwningPlayer;
    }
}



