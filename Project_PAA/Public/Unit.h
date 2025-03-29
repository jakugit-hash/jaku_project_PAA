
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GlobalEnums.h"
#include "Unit.generated.h"

// Forward declaration
class AGridCell;
class AGridManager;

UCLASS()
class PROJECT_PAA_API AUnit : public AActor
{
	GENERATED_BODY()

public:
	AUnit();

protected:
	virtual void BeginPlay() override;

public:

	// Unit properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	int32 Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	int32 MovementRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	int32 AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	int32 MinDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	int32 MaxDamage;

	// Static Mesh Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit")
	UStaticMeshComponent* UnitMesh;
	
	UFUNCTION(BlueprintCallable)
	void SetSelected(bool bSelected);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasMovedThisTurn = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsPlayerUnit = true; // Set to false for AI units

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasAttackedThisTurn = false;
    
	// Call this when creating units in PlaceUnit():
	void SetAsPlayerUnit(bool bIsPlayer) { bIsPlayerUnit = bIsPlayer; }

	UFUNCTION(BlueprintCallable, Category = "Unit")
	bool CanAttack() const 
	{
		return Health > 0 && 
			  !bHasAttackedThisTurn && 
			  (bIsPlayerUnit || !GetWorld()->GetFirstPlayerController());
	}

	
	UFUNCTION(BlueprintCallable)
	void SetGridPosition(FVector2D NewPosition);
	
	UFUNCTION(BlueprintCallable)
	FVector2D GetGridPosition() const;

	// Movement
	void MoveToCell(FVector2D NewPosition);
	void OnClicked(UPrimitiveComponent* ClickedComp, FKey ButtonPressed);
	void DestroyUnit();

	UPROPERTY(VisibleAnywhere)
	bool bIsSelected = false;
private:
	FVector2D GridPosition; // Current position on the grid

	// Helper function to get the GridManager
	AGridManager* GetGridManager() const;

	
};