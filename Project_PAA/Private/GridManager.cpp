#include "GridManager.h"
#include "MyGameMode.h"
#include "GridCell.h"
#include "Kismet/GameplayStatics.h"

// Constructor
AGridManager::AGridManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bGridCreated = false;
}

// Called when the game starts or when spawned
void AGridManager::BeginPlay()
{
    Super::BeginPlay();

    // Get the GameMode
    GameMode = Cast<AMyGameMode>(GetWorld()->GetAuthGameMode());
    if (!IsValid(GameMode))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get GameMode!"));
    }

    // Create the grid
    CreateGrid();

    // Generate obstacles
    GenerateObstacles();
}

// Create the grid
void AGridManager::CreateGrid()
{
    if (bGridCreated)
    {
        UE_LOG(LogTemp, Warning, TEXT("Grid already created!"));
        return;
    }

    // Clear existing grid cells (if any)
    for (AGridCell* Cell : GridCells)
    {
        if (IsValid(Cell))
        {
            Cell->Destroy();
        }
    }
    GridCells.Empty();

    // Create new grid cells
    for (int32 X = 0; X < GridSizeX; X++)
    {
        for (int32 Y = 0; Y < GridSizeY; Y++)
        {
            FVector WorldLocation = GetCellWorldPosition(X, Y);
            AGridCell* NewCell = GetWorld()->SpawnActor<AGridCell>(AGridCell::StaticClass(), WorldLocation, FRotator::ZeroRotator);
            if (IsValid(NewCell))
            {
                NewCell->SetCellName(FString::Printf(TEXT("%c%d"), 'A' + X, Y + 1));
                NewCell->SetGridPosition(X, Y);
                NewCell->SetOwner(this); // Set GridManager as the owner
                GridCells.Add(NewCell);
                UE_LOG(LogTemp, Warning, TEXT("Created grid cell at (%d, %d)"), X, Y);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to spawn grid cell at (%d, %d)"), X, Y);
            }
        }
    }

    bGridCreated = true;
    UE_LOG(LogTemp, Warning, TEXT("Grid creation completed with %d cells."), GridCells.Num());
}

// Generate obstacles
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
                    AGridCell* Cell = GetCellAtPosition(FVector2D(X, Y));
                    if (Cell)
                    {
                        Cell->SetObstacle(true);
                    }
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

// Create the obstacle map
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

// Check if all cells are reachable
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

// Function to handle cell clicks
void AGridManager::HandleCellClick(FVector2D CellPosition)
{
    if (GridCells.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("GridCells is empty! Cannot handle cell click."));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Cell clicked at (%f, %f)"), CellPosition.X, CellPosition.Y);

    if (IsValid(GameMode))
    {
        GameMode->HandleUnitPlacement(CellPosition);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameMode is null!"));
    }
}

// Function to get cell name
FString AGridManager::GetCellName(int32 X, int32 Y)
{
    return FString::Printf(TEXT("%c%d"), 'A' + X, Y + 1);
}

// Function to get world position of a grid cell
FVector AGridManager::GetCellWorldPosition(int32 X, int32 Y)
{
    return FVector(X * CellSize, Y * CellSize, 1.0f);
}

// Function to find a random empty cell
bool AGridManager::FindRandomEmptyCell(int32& OutX, int32& OutY)
{
    if (GridCells.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("GridCells is empty!"));
        return false;
    }

    // Collect all empty cells
    TArray<FVector2D> EmptyCells;
    for (AGridCell* Cell : GridCells)
    {
        if (IsValid(Cell) && !Cell->IsObstacle() && !Cell->IsOccupied())
        {
            EmptyCells.Add(FVector2D(Cell->GetGridPositionX(), Cell->GetGridPositionY()));
        }
    }

    // Check if there are any empty cells
    if (EmptyCells.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No empty cells found!"));
        return false;
    }

    // Select a random empty cell
    int32 RandomIndex = FMath::RandRange(0, EmptyCells.Num() - 1);
    if (RandomIndex >= 0 && RandomIndex < EmptyCells.Num())
    {
        OutX = EmptyCells[RandomIndex].X;
        OutY = EmptyCells[RandomIndex].Y;
        UE_LOG(LogTemp, Warning, TEXT("Found empty cell at (%d, %d)"), OutX, OutY);
        return true;
    }

    UE_LOG(LogTemp, Error, TEXT("Invalid random index generated!"));
    return false;
}

// Function to get a cell at a specific position
AGridCell* AGridManager::GetCellAtPosition(FVector2D Position) const
{
    int32 X = FMath::RoundToInt(Position.X);
    int32 Y = FMath::RoundToInt(Position.Y);

    if (X >= 0 && X < GridSizeX && Y >= 0 && Y < GridSizeY)
    {
        for (AGridCell* Cell : GridCells)
        {
            if (Cell && Cell->GetGridPositionX() == X && Cell->GetGridPositionY() == Y)
            {
                return Cell;
            }
        }
    }

    return nullptr;
}

// Function to check if a cell is free
bool AGridManager::IsCellFree(FVector2D CellPosition) const
{
    AGridCell* Cell = GetCellAtPosition(CellPosition);
    return Cell && !Cell->IsObstacle() && !Cell->IsOccupied();
}

// Called when the actor is being destroyed
void AGridManager::DestroyGrid()
{
    Super::BeginDestroy();

    // Clean up grid cells
    for (AGridCell* Cell : GridCells)
    {
        if (IsValid(Cell))
        {
            Cell->Destroy();
        }
    }
    GridCells.Empty();

    UE_LOG(LogTemp, Warning, TEXT("GridManager cleaned up!"));
}