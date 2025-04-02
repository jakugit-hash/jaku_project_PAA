#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "GridCell.generated.h"

// Forward declarations
class AGridManager;
class AUnit;
class AMyGameMode;

UCLASS()
class PROJECT_PAA_API AGridCell : public AActor
{
	GENERATED_BODY()

public:
	AGridCell();

protected:
	virtual void BeginPlay() override;

public:
	// nome cella (es. "A1")
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	FString CellName;

	// mesh visiva
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	UStaticMeshComponent* CellMesh;

	// è un ostacolo?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	bool bIsObstacle = false;

	// è occupata?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	bool bIsOccupied = false;

	// posizione griglia
	int32 GridPositionX;
	int32 GridPositionY;

	// click cella
	UFUNCTION()
	void OnCellClicked(UPrimitiveComponent* ClickedComponent, FKey ButtonPressed);

	// highlight
	UFUNCTION(BlueprintCallable)
	void SetHighlight(bool bHighlight);

	UFUNCTION(BlueprintCallable)
	void SetHighlightColor(FLinearColor NewColor);

	// gestione cella
	void SetObstacle(bool bObstacle);
	bool IsObstacle() const;

	void SetOccupied(bool bOccupied);
	bool IsOccupied() const;

	void SetGridPosition(int32 X, int32 Y);
	void SetCellName(const FString& Name);

	int32 GetGridPositionX() const;
	int32 GetGridPositionY() const;
	FVector2D GetGridPosition() const;

	// gestione unità
	UFUNCTION(BlueprintCallable)
	void SetUnit(AUnit* Unit);

	UFUNCTION(BlueprintCallable)
	AUnit* GetUnit() const;
	
	UFUNCTION(BlueprintCallable)
	FString GetCellName() const { return CellName; }


private:
	// materiali
	UPROPERTY(EditDefaultsOnly, Category = "Grid")
	UMaterialInterface* DefaultMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Grid")
	UMaterialInterface* ObstacleMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Grid")
	UMaterialInterface* HighlightMoveMaterial;
};
