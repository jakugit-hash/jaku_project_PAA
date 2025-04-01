#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GlobalEnums.h"
#include "GridCell.h"
#include "GridManager.generated.h"

// Forward declaration
class AUnit;
class AMyGameMode;
class AUnitActions;

UCLASS()
class PROJECT_PAA_API AGridManager : public AActor
{
    GENERATED_BODY()

public:
    AGridManager();

protected:
    virtual void BeginPlay() override;

public:
    // Grid dimensions and properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 GridSizeX = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 GridSizeY = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    float CellSize = 100.0f;

    // Array to store grid cells
    UPROPERTY()
    TArray<AGridCell*> GridCells;

    // Reference to the GameMode
    UPROPERTY()
    AMyGameMode* GameMode;

    // Spawn probability for obstacles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ClampMin = "0.0", ClampMax = "1.0"), meta = (AllowPrivateAccess = "true"))
    float SpawnProbability = 0.15f;

    // Obstacle Blueprint Reference
    UPROPERTY(EditAnywhere, Category = "Grid")
    TSubclassOf<AActor> ObstacleBlueprint;


    // Functions
    void CreateGrid();
    void GenerateObstacles();
    void DestroyGrid();
    bool IsCellBlocked(int32 X, int32 Y) const;

    UFUNCTION()
    void HandleCellClick(AGridCell* ClickedCell);
    
    UFUNCTION()
    void TryMoveSelectedUnit(AGridCell* TargetCell);
    
    void HandlePlayerAction(AGridCell* ClickedCell);
    
    
   // bool IsPathClear(FVector2D Start, FVector2D End) const;
    void ClearHighlights();
    
    // Utility Functions
    FString GetCellName(int32 X, int32 Y);
    FVector GetCellWorldPosition(int32 X, int32 Y);
    bool FindRandomEmptyCell(int32& OutX, int32& OutY);

    AGridCell* GetCellAtPosition(FVector2D Position) const;
    bool IsCellFree(FVector2D CellPosition) const;

    // Functions to get grid and cell dimensions
    int32 GetGridSizeX() const { return GridSizeX; }
    int32 GetGridSizeY() const { return GridSizeY; }
    float GetCellSize() const { return CellSize; }
    
void CreateObstacleMap(TArray<TArray<bool>>& OutObstacleMap) const;
    bool AreAllCellsReachable(const TArray<TArray<bool>>& InObstacleMap) const;
    void BFS(const TArray<TArray<bool>>& InObstacleMap, TArray<TArray<bool>>& Visited, int32 StartX, int32 StartY) const;

   
    TArray<FVector2D> FindPath(FVector2D Start, FVector2D End , AUnit* MovingUnit);
    TArray<FVector2D> AStarPathfind(FVector2D Start, FVector2D End, int32 MaxRange) const;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    UMaterialInterface* DefaultTileMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    UMaterialInterface* HighlightMoveMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    UMaterialInterface* HighlightAttackMaterial;

    /*bool bIsMovementRangeVisible;
    bool bIsHighlightActive;
    AUnit* LastHighlightedUnit;*/
    //void LoadMaterials();
    
    UPROPERTY()
    AUnit* CurrentlyHighlightedUnit = nullptr;
    void HighlightMovementRange(FVector2D Center, int32 Range, bool bHighlight);
    // In GridManager.h
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void HighlightAttackRange(FVector2D Center, int32 Range, bool bHighlight, bool bIsRangedAttack = false);
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void HighlightCell(int32 X, int32 Y, bool bHighlight, bool bIsAttackRange = false);
    bool IsValidCell(FVector2D Pos) const;

private:
    // Flag to track if the grid has been created
    bool bGridCreated;
    int32 HeuristicCost(FVector2D A, FVector2D B) const;

};