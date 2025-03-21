
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "GridCell.generated.h"

// Forward declarations
class ABaseUnit;
class AGridManager;


UCLASS()
class PROJECT_PAA_API AGridCell : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY()
	UMaterialInterface* ObstacleMaterial = nullptr;



public:
	AGridCell();

protected:
	virtual void BeginPlay() override;

public:
	// Cell Name (e.g., "A1", "B5")
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Grid")
	FString CellName;

	// Static Mesh for Visualization
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Grid")
	UStaticMeshComponent* CellMesh;

	// Is this cell an obstacle?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Grid")
	bool bIsObstacle = false;

	// Is this cell occupied by a unit?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Grid")
	bool bIsOccupied = false;

	// Set this cell as an obstacle
	void SetObstacle(bool bObstacle);
	bool IsObstacle() const;

	void SetOccupied(bool bOccupied);
	bool IsOccupied() const { return bIsOccupied; }
};
