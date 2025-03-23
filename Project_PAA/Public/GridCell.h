#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "GridCell.generated.h"

// Forward declarations
class AGridManager;

UCLASS()
class PROJECT_PAA_API AGridCell : public AActor
{
	GENERATED_BODY()

public:
	AGridCell();

protected:
	virtual void BeginPlay() override;

public:
	// Cell Name (e.g., "A1", "B5")
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	FString CellName;

	// Static Mesh for Visualization
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	UStaticMeshComponent* CellMesh;

	// Is this cell an obstacle?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	bool bIsObstacle = false;

	// Is this cell occupied by a unit?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	bool bIsOccupied = false;

	// Grid position of the cell
	int32 GridPositionX;
	int32 GridPositionY;

	// Functions
	void SetObstacle(bool bObstacle);
	bool IsObstacle() const;

	void SetOccupied(bool bOccupied);
	bool IsOccupied() const;

	void SetGridPosition(int32 X, int32 Y);
	void SetCellName(const FString& Name);

	int32 GetGridPositionX() const;
	int32 GetGridPositionY() const;

	// Handle cell click
	UFUNCTION()
	void OnCellClicked(AActor* TouchedActor, FKey ButtonPressed);

private:
	// Default and obstacle materials
	UPROPERTY(EditDefaultsOnly, Category = "Grid")
	UMaterialInterface* DefaultMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Grid")
	UMaterialInterface* ObstacleMaterial;
};