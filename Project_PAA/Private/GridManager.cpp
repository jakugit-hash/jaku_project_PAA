
#include "GridManager.h"
#include "GameFramework/Actor.h"

// Sets default values
AGridManager::AGridManager() : ObstacleCount(0)
{
    PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AGridManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("AGridManager::BeginPlay called!"));

    // Create the grid
    CreateGrid();

    // Generate obstacles
    GenerateObstacles();
}

// Function to create the grid
void AGridManager::CreateGrid()
{
    
    UE_LOG(LogTemp, Warning, TEXT("Creating Grid..."));

    GridArray.SetNum(GridSizeX);
    for (int32 X = 0; X < GridSizeX; X++)
    {
        GridArray[X].SetNum(GridSizeY);
        for (int32 Y = 0; Y < GridSizeY; Y++)
        {
            FVector CellPosition = FVector(X * CellSize, Y * CellSize, 0);
            AGridCell* NewCell = GetWorld()->SpawnActor<AGridCell>(GridCellBlueprint, CellPosition, FRotator::ZeroRotator);


            if (NewCell)
            {
                GridArray[X][Y] = NewCell;
                FString CellName = GetCellName(X, Y);
                NewCell->SetActorLabel(CellName);
                UE_LOG(LogTemp, Warning, TEXT("Spawned Cell: %s"), *CellName);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to spawn grid cell at (%d, %d)"), X, Y);
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Grid creation completed"));
}

// Function to generate obstacles dynamically while ensuring connectivity
void AGridManager::GenerateObstacles()
{
    if (!ObstacleBlueprint)
    {
        UE_LOG(LogTemp, Error, TEXT("ObstacleBlueprint is not set!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Generating obstacles with probability: %f"), SpawnProbability);

    TArray<TArray<bool>> LocalObstacleMap;
    CreateObstacleMap(LocalObstacleMap);

    for (int32 X = 0; X < GridSizeX; X++)
    {
        for (int32 Y = 0; Y < GridSizeY; Y++)
        {
            if (LocalObstacleMap[X][Y]) // If the cell should contain an obstacle
            {
                FVector TilePosition = GetCellWorldPosition(X, Y);
                AActor* NewObstacle = GetWorld()->SpawnActor<AActor>(ObstacleBlueprint, TilePosition, FRotator::ZeroRotator);
                if (NewObstacle)
                {
                    GridArray[X][Y]->SetObstacle(true);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Failed to spawn obstacle at: X=%d, Y=%d"), X, Y);
                }
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Obstacle generation completed."));
}

// Function to create the obstacle map
void AGridManager::CreateObstacleMap(TArray<TArray<bool>>& OutObstacleMap)
{
    // Ensure OutObstacleMap has correct dimensions
    OutObstacleMap.SetNum(GridSizeX);
    for (int32 X = 0; X < GridSizeX; X++)
    {
        OutObstacleMap[X].SetNum(GridSizeY, EAllowShrinking::No);
        for (int32 Y = 0; Y < GridSizeY; Y++)
        {
            OutObstacleMap[X][Y] = false; // Initialize as empty
        }
    }

    for (int32 X = 0; X < GridSizeX; X++)
    {
        for (int32 Y = 0; Y < GridSizeY; Y++)
        {
            if (FMath::FRand() <= SpawnProbability)
            {
                OutObstacleMap[X][Y] = true;

                // Validate connectivity
                if (!AreAllCellsReachable(OutObstacleMap))
                {
                    OutObstacleMap[X][Y] = false; // Remove obstacle if it blocks paths
                }
            }
        }
    }
}

// Function to check if all cells are reachable
bool AGridManager::AreAllCellsReachable(const TArray<TArray<bool>>& InObstacleMap)
{
    // Initialize visited map
    TArray<TArray<bool>> Visited;
    Visited.SetNum(GridSizeX);
    for (int32 X = 0; X < GridSizeX; X++)
    {
        Visited[X].SetNum(GridSizeY, EAllowShrinking::No);
    }

    // Find a starting cell that is not an obstacle
    int32 StartX = -1, StartY = -1;
    for (int32 X = 0; X < GridSizeX; X++)
    {
        for (int32 Y = 0; Y < GridSizeY; Y++)
        {
            if (!InObstacleMap[X][Y])
            {
                StartX = X;
                StartY = Y;
                break;
            }
        }
        if (StartX != -1) break;
    }

    if (StartX == -1) return false; // No empty cells

    // Perform BFS to visit all reachable cells
    BFS(InObstacleMap, Visited, StartX, StartY);

    // Check if all non-obstacle cells were visited
    for (int32 X = 0; X < GridSizeX; X++)
    {
        for (int32 Y = 0; Y < GridSizeY; Y++)
        {
            if (!InObstacleMap[X][Y] && !Visited[X][Y])
            {
                return false; // Unreachable cell found
            }
        }
    }
    return true; // All cells are reachable
}

// BFS implementation
void AGridManager::BFS(const TArray<TArray<bool>>& InObstacleMap, TArray<TArray<bool>>& Visited, int32 StartX, int32 StartY)
{
    TQueue<FIntPoint> Queue;
    Queue.Enqueue(FIntPoint(StartX, StartY));
    Visited[StartX][StartY] = true;

    while (!Queue.IsEmpty())
    {
        FIntPoint Current;
        Queue.Dequeue(Current);

        // Check adjacent cells (up, down, left, right)
        TArray<FIntPoint> Directions = { FIntPoint(-1, 0), FIntPoint(1, 0), FIntPoint(0, -1), FIntPoint(0, 1) };
        for (const FIntPoint& Dir : Directions)
        {
            int32 NewX = Current.X + Dir.X;
            int32 NewY = Current.Y + Dir.Y;

            // Check if the new cell is valid, not an obstacle, and not visited
            if (NewX >= 0 && NewX < GridSizeX && NewY >= 0 && NewY < GridSizeY &&
                !InObstacleMap[NewX][NewY] && !Visited[NewX][NewY])
            {
                Visited[NewX][NewY] = true;
                Queue.Enqueue(FIntPoint(NewX, NewY));
            }
        }
    }
}

// Function to destroy existing grid
void AGridManager::DestroyGrid()
{
    for (auto& Column : GridArray)
    {
        for (AGridCell* Cell : Column)
        {
            if (Cell)
            {
                Cell->Destroy();
            }
        }
    }
    GridArray.Empty();
}

// Function to get cell name
FString AGridManager::GetCellName(int32 X, int32 Y)
{
    return FString::Printf(TEXT("%c%d"), 'A' + X, Y + 1);
}

AGridCell* AGridManager::GetCellAtPosition(FVector2D Position) const
{
    int32 X = FMath::RoundToInt(Position.X);
    int32 Y = FMath::RoundToInt(Position.Y);

    if (X >= 0 && X < GridSizeX && Y >= 0 && Y < GridSizeY)
    {
        return GridArray[X][Y];
    }

    return nullptr;
}

// Function to get world position of a grid cell
FVector AGridManager::GetCellWorldPosition(int32 X, int32 Y)
{
    return FVector(X * CellSize, Y * CellSize, 1.0f);
}

bool AGridManager::IsCellFree(FVector2D CellPosition) const
{
    int32 X = CellPosition.X;
    int32 Y = CellPosition.Y;

    // Check if the cell is within bounds
    if (X < 0 || X >= GridSizeX || Y < 0 || Y >= GridSizeY)
    {
        return false;
    }

    // Check if the cell is an obstacle or occupied by another unit
    AGridCell* Cell = GridArray[X][Y];
    return !Cell->IsObstacle() && !Cell->IsOccupied(); // Check both obstacle and occupied flags
}

// Function to find a random empty cell
bool AGridManager::FindRandomEmptyCell(int32& OutX, int32& OutY)
{
    TArray<FIntPoint> EmptyCells;

    // Find all empty cells
    for (int32 X = 0; X < GridSizeX; X++)
    {
        for (int32 Y = 0; Y < GridSizeY; Y++)
        {
            if (!GridArray[X][Y]->IsObstacle())
            {
                EmptyCells.Add(FIntPoint(X, Y));
            }
        }
    }

    if (EmptyCells.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("No empty cells available!"));
        return false;
    }

    // Pick a random empty cell
    int32 RandomIndex = FMath::RandRange(0, EmptyCells.Num() - 1);
    OutX = EmptyCells[RandomIndex].X;
    OutY = EmptyCells[RandomIndex].Y;

    return true;
}

void AGridManager::BeginDestroy()
{
    Super::BeginDestroy();

    // Clean up grid cells
    for (AGridCell* Cell : GridCells)
    {
        if (Cell)
        {
            Cell->Destroy();
        }
    }
    GridCells.Empty();

    UE_LOG(LogTemp, Warning, TEXT("GridManager cleaned up!"));
}