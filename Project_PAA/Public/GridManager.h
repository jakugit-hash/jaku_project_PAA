
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridCell.h"
#include "GridManager.generated.h"

UCLASS()
class PROJECT_PAA_API AGridManager : public AActor
{
    GENERATED_BODY()

private:
    int32 ObstacleCount = 0;
    TArray<TArray<bool>> ObstacleMap; // Stores obstacle placements

    // Spawn probability for obstacles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ClampMin = "0.0", ClampMax = "1.0"), meta = (AllowPrivateAccess = "true"))
    float SpawnProbability = 0.15f;

    // Obstacle Blueprint Reference
    UPROPERTY(EditAnywhere, Category = "Grid")
    TSubclassOf<AActor> ObstacleBlueprint;

public:
    // Sets default values for this actor's properties
    AGridManager();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 GridSizeX = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 GridSizeY = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    float CellSize = 100.0f;

    // Cell Blueprint Reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    TSubclassOf<AGridCell> GridCellBlueprint;

    
    TArray<TArray<AGridCell*>> GridArray;

    // Grid Functions
    void CreateGrid();
    void GenerateObstacles();
    void DestroyGrid();

    // Obstacle Handling
    void CreateObstacleMap(TArray<TArray<bool>>& OutObstacleMap);
    bool AreAllCellsReachable(const TArray<TArray<bool>>& InObstacleMap);
    void BFS(const TArray<TArray<bool>>& InObstacleMap, TArray<TArray<bool>>& Visited, int32 StartX, int32 StartY);

    // Utility Functions
    FString GetCellName(int32 X, int32 Y);
    FVector GetCellWorldPosition(int32 X, int32 Y);
    bool FindRandomEmptyCell(int32& OutX, int32& OutY);

    AGridCell* GetCellAtPosition(FVector2D Position) const;

    // Helper functions for base units
    bool IsCellFree(FVector2D CellPosition) const;
    FVector GetCellWorldPosition(int32 X, int32 Y) const;

public:
    // Functions to get grid and cell dimensions
    int32 GetGridSizeX() const { return GridSizeX; }
    int32 GetGridSizeY() const { return GridSizeY; }
    float GetCellSize() const { return CellSize; }
};