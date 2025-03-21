
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridManager.h"
#include "Unit.generated.h"

// Forward declaration
class AGridCell;

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

	// Grid position
	FVector2D GetGridPosition() const { return GridPosition; }
	void SetGridPosition(FVector2D NewPosition);
	
	// Movement
	void MoveToCell(FVector2D NewPosition);

	// Static Mesh Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit")
	UStaticMeshComponent* UnitMesh;
	
private:
	FVector2D GridPosition; // Current position on the grid

	// Helper function to get the GridManager
	AGridManager* GetGridManager() const;
};