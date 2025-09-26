#pragma once
#include "Resourcenames.h"

#include "BuildingBase.generated.h"



// Delegate for building destruction
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildingDestroyed, class ABuildingBase*, DestroyedBuilding);
UCLASS()
class GAME_V0_API ABuildingBase : public AActor
{
	GENERATED_BODY()
public:
	
	
	// Sets default values for this actor's properties
	ABuildingBase();

	UPROPERTY(BlueprintAssignable)
	FOnBuildingDestroyed OnBuildingDestroyed;
	

	// Static mesh component for the building's visual representation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* BuildingMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building Properties", Replicated)
	class ACustomPlayerState* OwningPlayer;
	// Health properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Properties", meta = (ClampMin = "0.0"))
	float MaxHealth;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building Properties")
	float CurrentHealth;
    
	// Team ID to identify which player owns this building
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Properties")
	int32 TeamID;
    
	// Building properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Properties")
	FString BuildingName;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Properties")
	FText BuildingDescription;
    
	// Construction properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
	float ConstructionTime;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Construction")
	bool bIsConstructed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Dimensions")
	float Width;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Dimensions")
	float Length;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Dimensions")
	float Height;

	UPROPERTY(ReplicatedUsing = OnRep_IsDestroyed, VisibleAnywhere, BlueprintReadOnly, Category = "Building Properties")
	bool bIsDestroyed;

	UFUNCTION(BlueprintCallable)
	void HealBuilding(float HealAmount);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;




protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void HandleBuildingDestroyed();

	// Replication callbacks
	UFUNCTION()
	void OnRep_CurrentHealth();

	UFUNCTION()
	void OnRep_IsConstructed();

	UFUNCTION()
	void OnRep_IsDestroyed();

public:

	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
    
	// Set the building's static mesh
	UFUNCTION(BlueprintCallable, Category = "Building Setup")
	void SetBuildingMesh(UStaticMesh* NewMesh);
    
	// Apply damage to the building
	UFUNCTION(BlueprintCallable, Category = "Building Functions")
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
    
	// Complete construction of the building
	UFUNCTION(BlueprintCallable, Category = "Building Functions")
	virtual void CompleteConstruction();
    
	// Check if building is constructed
	UFUNCTION(BlueprintPure, Category = "Building Functions")
	bool IsConstructed() const { return bIsConstructed; }
    
	// Get building health as percentage
	UFUNCTION(BlueprintPure, Category = "Building Functions")
	float GetHealthPercentage() const;

	UFUNCTION(BlueprintCallable, Category = "Building Functions")
	void SetOwningPlayer(ACustomPlayerState* InOwningPlayer);
};
