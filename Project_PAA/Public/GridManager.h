#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridCell.h"
#include "MyGameMode.h"
#include "GridManager.generated.h"

UCLASS()
class PROJECT_PAA_API AGridManager : public AActor
{
    GENERATED_BODY()

public:
    AGridManager();

protected:
    virtual void BeginPlay() override;
    virtual void BeginDestroy() override;

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

    // Functions
    void CreateGrid();
    void GenerateObstacles();
    void DestroyGrid();
    void HandleCellClick(FVector2D CellPosition);

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

private:
    // Flag to track if the grid has been created
    bool bGridCreated;

    // Spawn probability for obstacles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ClampMin = "0.0", ClampMax = "1.0"), meta = (AllowPrivateAccess = "true"))
    float SpawnProbability = 0.15f;
};