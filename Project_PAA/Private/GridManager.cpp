#include "GridManager.h"
#include "MyGameMode.h"
#include "GridCell.h"
#include "GlobalEnums.h"
#include "Unit.h"
#include "UnitActions.h"
#include "Containers/Queue.h"            // per TQueue
#include "Containers/Array.h"
#include "Containers/Set.h"
#include "Templates/Greater.h"           // per TGreater<>
#include "WBP_ActionWidget.h"
#include "Kismet/GameplayStatics.h"

// Constructor
AGridManager::AGridManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bGridCreated = false;
    /*// materiale di default per la griglia
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMatFinder(
        TEXT("/Game/StarterContent/Materials/M_Water_Lake.M_Water_Lake"));
    DefaultTileMaterial = DefaultMatFinder.Object;
    
    // materiale blu per il movimento
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MoveMatFinder(
        TEXT("/Game/StarterContent/Materials/M_Metal_Gold.M_Metal_Gold"));
    HighlightMoveMaterial = MoveMatFinder.Object;

    // materiale rosso per l'attacco
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> AttackMatFinder(
        TEXT("/Game/StarterContent/Materials/M_Brick_Clay_New.M_Brick_Clay_New"));
    HighlightAttackMaterial = AttackMatFinder.Object;*/
    // Configurazione aggiuntiva per differenziare i materiali

    
    if (HighlightMoveMaterial)
    {
        UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(HighlightMoveMaterial, this);
        DynMat->SetVectorParameterValue("Color", FLinearColor(0.1f, 0.3f, 1.0f, 0.5f)); // Blu
        HighlightMoveMaterial = DynMat;
    }

    if (HighlightAttackMaterial)
    {
        UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(HighlightAttackMaterial, this);
        DynMat->SetVectorParameterValue("Color", FLinearColor(1.0f, 0.1f, 0.1f, 0.7f)); // Rosso
        HighlightAttackMaterial = DynMat;
    }
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

    if (!DefaultTileMaterial) UE_LOG(LogTemp, Error, TEXT("DefaultTileMaterial non caricato!"));
    if (!HighlightMoveMaterial) UE_LOG(LogTemp, Error, TEXT("HighlightMoveMaterial non caricato!"));
    if (!HighlightAttackMaterial) UE_LOG(LogTemp, Error, TEXT("HighlightAttackMaterial non caricato!"));
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
                NewCell->SetOwner(this);
                // Debug: Visualize collision
                NewCell->CellMesh->SetHiddenInGame(false);
                NewCell->CellMesh->SetVisibility(true);
                
                if (UStaticMeshComponent* Mesh = NewCell->FindComponentByClass<UStaticMeshComponent>())
                {
                    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                    Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
                    Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
                    Mesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
                }
                
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
void AGridManager::CreateObstacleMap(TArray<TArray<bool>>& OutObstacleMap) const
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
bool AGridManager::AreAllCellsReachable(const TArray<TArray<bool>>& InObstacleMap) const
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
void AGridManager::BFS(const TArray<TArray<bool>>& InObstacleMap, TArray<TArray<bool>>& Visited, int32 StartX, int32 StartY) const
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
void AGridManager::HandleCellClick(AGridCell* ClickedCell)
{
    if (!GameMode || !ClickedCell) return;

    // Se la cella ha un'unità
    if (AUnit* ClickedUnit = ClickedCell->GetUnit())
    {
        // Se è la stessa unità già selezionata e l'highlight è attivo
        if (GameMode->SelectedUnit == ClickedUnit && CurrentlyHighlightedUnit == ClickedUnit)
        {
            // Disattiva completamente l'highlight
            ClearHighlights();
            GameMode->SelectedUnit->SetSelected(false);
            GameMode->SelectedUnit = nullptr;
            GameMode->HideActionWidget();
            CurrentlyHighlightedUnit = nullptr;
            return;
        }
        
        // Altrimenti gestisci nuova selezione
        GameMode->HandleUnitSelection(ClickedUnit);
        CurrentlyHighlightedUnit = ClickedUnit;
    }
    // Gestione del movimento
    else if (GameMode->bWaitingForMoveTarget && GameMode->SelectedUnit)
    {
        TryMoveSelectedUnit(ClickedCell);
        CurrentlyHighlightedUnit = nullptr;
    }


    // Attack case
    else if (GameMode->bWaitingForAttack && GameMode->SelectedUnit)
    {
        TryAttackSelectedUnit(ClickedCell);
    }
}


void AGridManager::TryMoveSelectedUnit(AGridCell* TargetCell)
{
    if (!GameMode || !GameMode->SelectedUnit || !TargetCell) return;

    // calcola il path con A*
    TArray<FVector2D> Path = AStarPathfind(
        GameMode->SelectedUnit->GetGridPosition(),
        TargetCell->GetGridPosition(),
        GameMode->SelectedUnit->MovementRange
    );

    // verifica che il path sia valido
    if (Path.Num() > 0 && Path.Last() == TargetCell->GetGridPosition())
    {
        if (GameMode->UnitActions->MoveUnit(GameMode->SelectedUnit, TargetCell->GetGridPosition()))
        {
            TargetCell->SetHighlightColor(FLinearColor::Blue);
            GameMode->SelectedUnit->SetSelected(false);
            GameMode->SelectedUnit = nullptr;
            GameMode->bWaitingForMoveTarget = false;
            ClearHighlights();
            GameMode->CheckTurnCompletion();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No valid path to target cell!"));
    }
}

void AGridManager::HandlePlayerAction(AGridCell* ClickedCell)
{
    if (!GameMode || !GameMode->UnitActions) return;

    // Case 1: Unit Selection
    if (AUnit* ClickedUnit = ClickedCell->GetUnit())
    {
        if (ClickedUnit->bIsPlayerUnit && GameMode->bIsPlayerTurn)
        {
            if (GameMode->SelectedUnit == ClickedUnit)
            {
                ClearHighlights();
                GameMode->SelectedUnit = nullptr;
                GameMode->HideActionWidget();
                return;
            }

            ClearHighlights();
            GameMode->SelectedUnit = ClickedUnit;
            HighlightMovementRange(ClickedUnit->GetGridPosition(), ClickedUnit->MovementRange, true);
            GameMode->ShowActionWidget(ClickedUnit);
        }
        return;
    }

    // Case 2: Movement Execution
    if (GameMode->SelectedUnit && GameMode->bWaitingForMoveTarget)
    {
        // Use IsCellBlocked with the moving unit
        if (!IsCellBlocked(ClickedCell->GetGridPosition().X, ClickedCell->GetGridPosition().Y))
        {
            GameMode->UnitActions->MoveUnit(GameMode->SelectedUnit, ClickedCell->GetGridPosition());
            ClearHighlights();
            GameMode->bWaitingForMoveTarget = false;
        }
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
/*
bool AGridManager::IsCellFree(FVector2D CellPosition) const
{
    AGridCell* Cell = GetCellAtPosition(CellPosition);
    return Cell && !Cell->IsObstacle() && !Cell->IsOccupied();
}
*/

bool AGridManager::IsCellFree(FVector2D CellPosition) const
{
    return !IsCellBlocked(CellPosition.X, CellPosition.Y);
}

void AGridManager::HighlightMovementRange(FVector2D Center, int32 Range, bool bHighlight)
{
    ClearHighlights();
    if (!bHighlight || !GameMode || !GameMode->SelectedUnit) return;

    CurrentlyHighlightedUnit = GameMode->SelectedUnit;

    for (AGridCell* Cell : GridCells)
    {
        if (!Cell || Cell->IsObstacle() || Cell->IsOccupied()) continue;

        FVector2D CellPos = Cell->GetGridPosition();
        if (CellPos == Center) continue;

        // calcola il path usando A*
        TArray<FVector2D> Path = AStarPathfind(Center, CellPos, Range);

        // se la cella è raggiungibile e nel range
        if (Path.Num() > 0 && Path.Last() == CellPos)
        {
            HighlightCell(CellPos.X, CellPos.Y, true, false);
        }
    }
}

void AGridManager::TryAttackSelectedUnit(AGridCell* TargetCell)
{
    if (!GameMode || !GameMode->SelectedUnit || !TargetCell) return;

    AUnit* Attacker = GameMode->SelectedUnit;
    AUnit* Target = TargetCell->GetUnit();

    if (!Target)
    {
        UE_LOG(LogTemp, Warning, TEXT("No unit to attack in the selected cell."));
        return;
    }

    // Verifica se è nemico
    if (Attacker->bIsPlayerUnit == Target->bIsPlayerUnit)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot attack a friendly unit."));
        return;
    }

    // Calcola distanza Manhattan
    FVector2D AttackerPos = Attacker->GetGridPosition();
    FVector2D TargetPos = Target->GetGridPosition();
    int32 Distance = FMath::Abs(TargetPos.X - AttackerPos.X) + FMath::Abs(TargetPos.Y - AttackerPos.Y);

    if (Distance > Attacker->AttackRange)
    {
        UE_LOG(LogTemp, Warning, TEXT("Target is out of range!"));
        return;
    }

    // Attacco valido: calcola danno random
    int32 Damage = FMath::RandRange(Attacker->MinDamage, Attacker->MaxDamage);
    UE_LOG(LogTemp, Warning, TEXT("%s is attacking %s for %d damage"),
        *Attacker->GetName(), *Target->GetName(), Damage);

    Target->HP -= Damage;

    if (Target->HP <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s has been destroyed!"), *Target->GetName());

        // Rimuovi dalla cella
        TargetCell->SetUnit(nullptr);
        Target->DestroyUnit(); // già esistente nel tuo progetto
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("%s HP remaining: %d"), *Target->GetName(), Target->HP);
    }

    // Imposta flag per fine attacco
    Attacker->bHasAttackedThisTurn = true;
    GameMode->bWaitingForAttackTarget = false;
    GameMode->SelectedUnit = nullptr;
    GameMode->CheckTurnCompletion();
    ClearHighlights();
}


/*void AGridManager::HighlightMovementRange(FVector2D Center, int32 Range, bool bHighlight)
{
    // Clear ALL highlights first
    for (AGridCell* Cell : GridCells)
    {
        if (Cell && Cell->CellMesh)
        {
            Cell->SetHighlight(false);
        }
    }

    if (!bHighlight)
    {
        CurrentlyHighlightedUnit = nullptr;
        return;
    }

    // Highlight new cells
    for (AGridCell* Cell : GridCells)
    {
        if (!Cell) continue;

        FVector2D CellPos = Cell->GetGridPosition();
        int32 Distance = FMath::Abs(CellPos.X - Center.X) + FMath::Abs(CellPos.Y - Center.Y);

        if (Distance <= Range)
        {
            // Use IsCellBlocked with the currently selected unit
            if (!IsCellBlocked(CellPos.X, CellPos.Y, CurrentlyHighlightedUnit))
            {
                HighlightCell(CellPos.X, CellPos.Y, true, false);
            }
        }
    }

    CurrentlyHighlightedUnit = GameMode->SelectedUnit;
}*/
/*
void AGridManager::HighlightMovementRange(FVector2D Center, int32 Range, bool bHighlight)
{
    ClearHighlights();
    // se non dobbiamo evidenziare, cancelliamo e basta
    if (!bHighlight)
    {
        UE_LOG(LogTemp, Warning, TEXT("DAMNNNNN"));    
        CurrentlyHighlightedUnit = nullptr;
        return;
    }

    // rimuovi highlight precedente
    CurrentlyHighlightedUnit = GameMode->SelectedUnit;

    for (AGridCell* Cell : GridCells)
    {
        if (!Cell) continue;

        FVector2D CellPos = Cell->GetGridPosition();
        int32 Distance = FMath::Abs(CellPos.X - Center.X) + FMath::Abs(CellPos.Y - Center.Y);

        if (Distance <= Range && IsPathClear(Center, CellPos))
        {
            HighlightCell(CellPos.X, CellPos.Y, true, false); // false = non è range attacco
        }
    }

}*/


void AGridManager::HighlightAttackRange(FVector2D Center, int32 Range, bool bHighlight, bool bIsRangedAttack, AUnit* Attacker)
{
    ClearHighlights();

    if (!bHighlight || !Attacker) 
    {
        CurrentlyHighlightedUnit = nullptr;
        return;
    }

    CurrentlyHighlightedUnit = Attacker;

    for (AGridCell* Cell : GridCells)
    {
        if (!Cell) continue;

        FVector2D CellPos = Cell->GetGridPosition();
        int32 Distance = FMath::Abs(CellPos.X - Center.X) + FMath::Abs(CellPos.Y - Center.Y);

        if (Distance == 0 || Distance > Range) continue;

        AUnit* Target = Cell->GetUnit();

        // ignora celle vuote
        if (!Target) continue;

        // ignora alleati
        if (Target->bIsPlayerUnit == Attacker->bIsPlayerUnit) continue;

        // check distanza melee: serve path
        if (!bIsRangedAttack)
        {
            TArray<FVector2D> Path = AStarPathfind(Center, CellPos, Range);
            if (Path.Num() == 0 || Path.Last() != CellPos) continue; // path bloccato
        }

        // evidenzia la cella con il nemico
        HighlightCell(CellPos.X, CellPos.Y, true, true);
        UE_LOG(LogTemp, Warning, TEXT("→ Highlight cell %s with enemy %s"), *Cell->GetCellName(), *Target->GetName());
    }
}




/*TArray<FVector2D> AGridManager::AStarPathfind(FVector2D Start, FVector2D End, const TArray<TArray<bool>>& ObstacleMap) const
{
    TArray<FVector2D> Path;
    TArray<TTuple<float, FVector2D, TArray<FVector2D>>> OpenSet;
    TSet<FVector2D> ClosedSet;

    // Initialize with start node
    TArray<FVector2D> InitialPath;
    InitialPath.Add(Start);
    OpenSet.Add(MakeTuple(HeuristicCost(Start, End), Start, InitialPath));

    while (OpenSet.Num() > 0)
    {
        // Get node with lowest cost
        OpenSet.Sort([](const auto& A, const auto& B) { return A.Get<0>() < B.Get<0>(); });
        auto CurrentNode = OpenSet[0];
        OpenSet.RemoveAt(0);

        FVector2D CurrentPos = CurrentNode.Get<1>();
        TArray<FVector2D> CurrentPath = CurrentNode.Get<2>();

        if (CurrentPos == End)
        {
            Path = CurrentPath;
            break;
        }

        ClosedSet.Add(CurrentPos);

        // Check neighbors
        TArray<FVector2D> Directions = { FVector2D(1,0), FVector2D(-1,0), FVector2D(0,1), FVector2D(0,-1) };
        for (const FVector2D& Dir : Directions)
        {
            FVector2D NeighborPos = CurrentPos + Dir;

            // Validate neighbor
            if (NeighborPos.X < 0 || NeighborPos.X >= GridSizeX || 
                NeighborPos.Y < 0 || NeighborPos.Y >= GridSizeY) continue;
            if (ObstacleMap[NeighborPos.X][NeighborPos.Y]) continue;
            if (ClosedSet.Contains(NeighborPos)) continue;

            // Calculate new path
            TArray<FVector2D> NewPath = CurrentPath;
            NewPath.Add(NeighborPos);
            float NewCost = CurrentPath.Num() + HeuristicCost(NeighborPos, End);

            // Add to open set
            OpenSet.Add(MakeTuple(NewCost, NeighborPos, NewPath));
        }
    }
    return Path;
}*/

TArray<FVector2D> AGridManager::AStarPathfind(FVector2D Start, FVector2D End, int32 MaxRange) const
{
    struct Node {
        FVector2D Position;
        int32 G; // Cost from start
        int32 H; // Heuristic to end
        int32 F() const { return G + H; }
        TArray<FVector2D> Path;

        Node(FVector2D Pos, int32 GVal, int32 HVal, const TArray<FVector2D>& PrevPath)
            : Position(Pos), G(GVal), H(HVal), Path(PrevPath) 
        {
            Path.Add(Pos);
        }
    };

    TArray<FVector2D> Result;
    TArray<Node> Open;
    TSet<FVector2D> Closed;

    Open.Add(Node(Start, 0, HeuristicCost(Start, End), {}));

    while (Open.Num() > 0)
    {
        // Sort by lowest F cost
        Open.Sort([](const Node& A, const Node& B) {
            return A.F() < B.F();
        });

        Node Current = Open[0];
        Open.RemoveAt(0);

        if (Current.Position == End)
        {
            return Current.Path;
        }

        Closed.Add(Current.Position);

        // 4-direction movement only (no diagonals)
        TArray<FVector2D> Directions = {
            FVector2D(1,0), FVector2D(-1,0),
            FVector2D(0,1), FVector2D(0,-1)
        };

        for (const FVector2D& Dir : Directions)
        {
            FVector2D Neighbor = Current.Position + Dir;

            // Check bounds
            if (!IsValidCell(Neighbor)) continue;
            
            // Check if already evaluated
            if (Closed.Contains(Neighbor)) continue;

            // Check if cell is blocked (ignoring the moving unit)
            if (IsCellBlocked(Neighbor.X, Neighbor.Y)) continue;

            int32 NewG = Current.G + 1;
            if (NewG > MaxRange) continue;

            // Check if this path is better
            bool bFoundBetter = false;
            for (Node& OpenNode : Open)
            {
                if (OpenNode.Position == Neighbor && OpenNode.G <= NewG)
                {
                    bFoundBetter = true;
                    break;
                }
            }

            if (!bFoundBetter)
            {
                int32 H = HeuristicCost(Neighbor, End);
                Open.Add(Node(Neighbor, NewG, H, Current.Path));
            }
        }
    }

    return {}; // No path found
}


TArray<FVector2D> AGridManager::FindPath(FVector2D Start, FVector2D End,  AUnit* MovingUnit)
{
    TArray<FVector2D> Path;

    if (Start == End) return Path;

    TArray<TTuple<float, FVector2D, TArray<FVector2D>>> OpenSet;
    TSet<FVector2D> ClosedSet;

    TArray<FVector2D> InitialPath;
    InitialPath.Add(Start);
    OpenSet.Add(MakeTuple(HeuristicCost(Start, End), Start, InitialPath));

    while (OpenSet.Num() > 0)
    {
        OpenSet.Sort([](const auto& A, const auto& B) { return A.Get<0>() < B.Get<0>(); });
        auto Current = OpenSet[0];
        OpenSet.RemoveAt(0);

        FVector2D CurrentPos = Current.Get<1>();
        TArray<FVector2D> CurrentPath = Current.Get<2>();

        if (CurrentPos == End)
        {
            Path = CurrentPath;
            break;
        }

        ClosedSet.Add(CurrentPos);

        // 4-direction movement
        TArray<FVector2D> Directions = { 
            FVector2D(1,0), FVector2D(-1,0), 
            FVector2D(0,1), FVector2D(0,-1) 
        };
        
        for (FVector2D Dir : Directions)
        {
            FVector2D NextPos = CurrentPos + Dir;

            // Bounds checking
            if (NextPos.X < 0 || NextPos.X >= GridSizeX || 
                NextPos.Y < 0 || NextPos.Y >= GridSizeY)
                continue;

            if (ClosedSet.Contains(NextPos)) continue;

            // Use IsCellBlocked (no specific unit to ignore here)
            if (IsCellBlocked(NextPos.X, NextPos.Y)) 
                continue;

            TArray<FVector2D> NewPath = CurrentPath;
            NewPath.Add(NextPos);
            float NewCost = CurrentPath.Num() + HeuristicCost(NextPos, End);

            OpenSet.Add(MakeTuple(NewCost, NextPos, NewPath));
        }
    }

    return Path;
}


void AGridManager::HighlightCell(int32 X, int32 Y, bool bHighlight, bool bIsAttackRange)
{
    AGridCell* Cell = GetCellAtPosition(FVector2D(X, Y));
    if (!Cell || !Cell->CellMesh) return;

    if (bHighlight)
    {
        UMaterialInterface* MaterialToUse = bIsAttackRange ? HighlightAttackMaterial : HighlightMoveMaterial;
        Cell->CellMesh->SetMaterial(0, MaterialToUse);
        Cell->SetHighlight(true);
    }
    else
    {
        // non serve riassegnare il materiale di default perché è già quello visibile di base
        // quindi non facciamo nulla quicle
    }
}

bool AGridManager::IsValidCell(FVector2D Pos) const
{
    return Pos.X >= 0 && Pos.X < GridSizeX && Pos.Y >= 0 && Pos.Y < GridSizeY;
}

int32 AGridManager::HeuristicCost(FVector2D A, FVector2D B) const
{
    return FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y); // distanza Manhattan
}


void AGridManager::ClearHighlights()
{
    for (AGridCell* Cell : GridCells)
    {
        if (!Cell || !Cell->CellMesh) continue;
        
        Cell->SetHighlight(false);
    }
    UE_LOG(LogTemp, Display, TEXT("ClearHighlights VEDERE SE VALE "));
    CurrentlyHighlightedUnit = nullptr;
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

FVector2D AGridCell::GetGridPosition() const 
{ 
    return FVector2D(GridPositionX, GridPositionY); 
}

bool AGridManager::IsCellBlocked(int32 X, int32 Y) const
{
    AGridCell* Cell = GetCellAtPosition(FVector2D(X, Y));
    if (!Cell) return true;

    if (Cell->IsObstacle()) return true;

    // ogni cella occupata è bloccante
    if (Cell->IsOccupied())
    {
        return true;
    }

    return false;
}

bool AGridManager::IsPathBlocked(AGridCell* Start, AGridCell* End)
{
    // solo distanza 1 quindi semplice controllo
    if (!Start || !End) return true;
    if (Start == End) return false;

    FVector2D Dir = End->GetGridPosition() - Start->GetGridPosition();
    if (FMath::Abs(Dir.X) + FMath::Abs(Dir.Y) != 1) return true; // solo vicini ortogonali

    return End->IsObstacle() || End->IsOccupied();
}


bool AGridManager::IsCellAttackable(int32 X, int32 Y, AUnit* Attacker) const
{
    AGridCell* Cell = GetCellAtPosition(FVector2D(X, Y));
    if (!Cell) return false;

    if (Attacker->AttackRange == 1)  // short-range (e.g., Brawler)
    {
        // can't attack over obstacles or empty cells
        if (Cell->IsObstacle()) return false;

        AUnit* Target = Cell->GetUnit();
        if (!Target || Target->bIsPlayerUnit == Attacker->bIsPlayerUnit)
        {
            return false; // no target or same team
        }
        return true;
    }
    else // long-range (e.g., Sniper)
    {
        AUnit* Target = Cell->GetUnit();
        if (!Target || Target->bIsPlayerUnit == Attacker->bIsPlayerUnit)
        {
            return false; // not an enemy
        }
        return true; // sniper ignores obstacles
    }
}

bool AGridManager::IsEnemyAtPosition(FVector2D Pos, bool bIsPlayer)
{
    if (AGridCell* Cell = GetCellAtPosition(Pos))
    {
        if (AUnit* Unit = Cell->GetUnit())
        {
            return Unit->bIsPlayerUnit != bIsPlayer; // nemico se appartiene alla squadra opposta
        }
    }
    return false;
}


FVector AGridManager::GetWorldPositionFromGrid(FVector2D GridPosition) const
{
    float X = GridPosition.X * CellSize;
    float Y = GridPosition.Y * CellSize;
    return FVector(X, Y, 0.0f); // oppure un altro valore Z se ne hai uno fisso
}

FString AGridManager::GetCellNameAtPosition(FVector2D Pos)
{
    TCHAR Letter = 'A' + Pos.X;
    int32 Number = Pos.Y + 1;
    return FString::Printf(TEXT("%c%d"), Letter, Number);
}
