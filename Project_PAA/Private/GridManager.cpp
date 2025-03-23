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
    if (GridCells.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("GridCells is empty! Cannot generate obstacles."));
        return;
    }

    // Generate obstacles
    for (AGridCell* Cell : GridCells)
    {
        if (Cell && FMath::FRand() < SpawnProbability)
        {
            Cell->SetObstacle(true);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Obstacle generation completed."));
}

// Handle cell clicks
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

// Get cell name
FString AGridManager::GetCellName(int32 X, int32 Y)
{
    return FString::Printf(TEXT("%c%d"), 'A' + X, Y + 1);
}

// Get world position of a grid cell
FVector AGridManager::GetCellWorldPosition(int32 X, int32 Y)
{
    return FVector(X * CellSize, Y * CellSize, 1.0f);
}

// Find a random empty cell
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

// Get a cell at a specific position
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

// Check if a cell is free
bool AGridManager::IsCellFree(FVector2D CellPosition) const
{
    AGridCell* Cell = GetCellAtPosition(CellPosition);
    return Cell && !Cell->IsObstacle() && !Cell->IsOccupied();
}

// Called when the actor is being destroyed
void AGridManager::BeginDestroy()
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