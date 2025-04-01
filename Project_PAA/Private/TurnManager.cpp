#include "TurnManager.h"
#include "MyGameMode.h"
#include "Unit.h"
#include "GridManager.h"
#include "UnitActions.h"
#include "Kismet/GameplayStatics.h"

ATurnManager::ATurnManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATurnManager::BeginPlay()
{
	Super::BeginPlay();
	// Initialization code if needed
}

void ATurnManager::StartActionPhase(AMyGameMode* GameMode)
{
	if (!GameMode || GameMode->CurrentGamePhase != EGamePhase::Placement) return;

	GameMode->CurrentGamePhase = EGamePhase::UnitAction;
	UE_LOG(LogTemp, Warning, TEXT("Combat phase started!"));

	if (GameMode->bIsPlayerTurn)
	{
		// Enable player controls
	}
	else
	{
		ExecuteAITurn(GameMode);
	}
}

void ATurnManager::EndTurn(AMyGameMode* GameMode)
{
	
	if (!GameMode) return;
	//HideActionWidget();

	GameMode->bIsPlayerTurn = !GameMode->bIsPlayerTurn;
    
	if (GameMode->bIsPlayerTurn)
	{
		// Reset player units
		for (AUnit* Unit : GameMode->PlayerUnits)
		{
			Unit->bHasMovedThisTurn = false;
		}
	}
	else
	{
		ExecuteAITurn(GameMode);
	}
}

void ATurnManager::ExecuteAITurn(AMyGameMode* GameMode)
{
	if (!GameMode) return;

	// 1. Movement Phase
	for (AUnit* AIUnit : GameMode->AIUnits)
	{
		if (!AIUnit || AIUnit->bHasMovedThisTurn) continue;

		AUnit* Target = FindNearestEnemy(AIUnit);
		if (!Target) continue;

		// Calculate move direction (simple approach)
		FVector2D Direction = (Target->GetGridPosition() - AIUnit->GetGridPosition()).GetSafeNormal();
		FVector2D TargetPos = AIUnit->GetGridPosition() + (Direction * AIUnit->MovementRange);

		if (GameMode->UnitActions->MoveUnit(AIUnit, TargetPos))
		{
			UE_LOG(LogTemp, Warning, TEXT("AI %s moved to (%.0f,%.0f)"), 
				*AIUnit->GetName(), TargetPos.X, TargetPos.Y);
		}
	}

	// 2. Attack Phase
	for (AUnit* AIUnit : GameMode->AIUnits)
	{
		if (!AIUnit || AIUnit->bHasAttackedThisTurn) continue;

		AUnit* Target = FindNearestEnemy(AIUnit);
		if (!Target) continue;

		if (GameMode->UnitActions->AttackUnit(AIUnit, Target))
		{
			UE_LOG(LogTemp, Warning, TEXT("AI %s attacked %s"), 
				*AIUnit->GetName(), *Target->GetName());
		}
	}

	// 3. End AI Turn
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [GameMode]()
	{
		GameMode->EndTurn();
	}, 2.0f, false); // 2 second delay for visibility
}

void ATurnManager::ProcessAIMovement(AMyGameMode* GameMode)
{
	if (!GameMode || !GameMode->GridManager || !GameMode->UnitActions) return;

	for (AUnit* AIUnit : GameMode->AIUnits)
	{
		
		if (!AIUnit || AIUnit->bHasMovedThisTurn) continue;
		FVector2D BestPosition;
		if (FindMovePositionForAI(AIUnit, BestPosition))
		{
			AIUnit->SetGridPosition(BestPosition);
			AIUnit->bHasMovedThisTurn = true;
			UE_LOG(LogTemp, Warning, TEXT("AI %s moved to [%.0f,%.0f]"), 
				*AIUnit->GetName(), BestPosition.X, BestPosition.Y);
		}
		
		// Find closest player unit
		AUnit* ClosestEnemy = nullptr;
		float MinDistance = MAX_FLT;
        
		for (AUnit* PlayerUnit : GameMode->PlayerUnits)
		{
			float Distance = FVector2D::Distance(AIUnit->GetGridPosition(), 
											   PlayerUnit->GetGridPosition());
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				ClosestEnemy = PlayerUnit;
			}
		}

		// Move toward closest enemy
		if (ClosestEnemy)
		{
			// Simple movement toward enemy (implement proper pathfinding here)
			FVector2D Direction = (ClosestEnemy->GetGridPosition() - AIUnit->GetGridPosition()).GetSafeNormal();
			FVector2D TargetPos = AIUnit->GetGridPosition() + (Direction * AIUnit->MovementRange);
            
			GameMode->UnitActions->MoveUnit(AIUnit, TargetPos);
		}
	}
	}


bool ATurnManager::FindMovePositionForAI(AUnit* AIUnit, FVector2D& OutBestPosition)
{
	if (!AIUnit || !AIUnit->GetWorld()) return false;

	AUnit* NearestEnemy = FindNearestEnemy(AIUnit);
	if (!NearestEnemy) return false;

	AGridManager* GridManager = Cast<AGridManager>(
		UGameplayStatics::GetActorOfClass(AIUnit->GetWorld(), AGridManager::StaticClass()));
	if (!GridManager) return false;


	// Create temporary obstacle map that matches our pathfinding needs
	TArray<TArray<bool>> TempObstacleMap;
	TempObstacleMap.SetNum(GridManager->GetGridSizeX());
    
	for (int32 X = 0; X < GridManager->GetGridSizeX(); X++)
	{
		TempObstacleMap[X].SetNum(GridManager->GetGridSizeY());
		for (int32 Y = 0; Y < GridManager->GetGridSizeY(); Y++)
		{
			if (AGridCell* Cell = GridManager->GetCellAtPosition(FVector2D(X, Y)))
			{
				// Mark cell as obstructed if it's an obstacle OR occupied by another unit
				TempObstacleMap[X][Y] = Cell->IsObstacle() || (Cell->IsOccupied() && !(X == AIUnit->GetGridPosition().X && Y == AIUnit->GetGridPosition().Y));
			}
			else
			{
				TempObstacleMap[X][Y] = true; // Invalid cells are always obstructed
			}
		}
	}

	// Use existing AStarPathfind with our custom obstacle map
	TArray<FVector2D> Path = GridManager->AStarPathfind(
		AIUnit->GetGridPosition(),
		NearestEnemy->GetGridPosition(),
		AIUnit->MovementRange
	
	);

	// Find farthest reachable point along path
	int32 MovesRemaining = AIUnit->MovementRange;
	FVector2D CurrentPos = AIUnit->GetGridPosition();
	OutBestPosition = CurrentPos;

	for (int32 i = 1; i < Path.Num() && MovesRemaining > 0; i++)
	{
		FVector2D NextPos = Path[i];
		int32 Cost = FMath::Abs(NextPos.X - CurrentPos.X) + 
					FMath::Abs(NextPos.Y - CurrentPos.Y);
        
		if (Cost <= MovesRemaining && GridManager->IsCellFree(NextPos))
		{
			OutBestPosition = NextPos;
			MovesRemaining -= Cost;
			CurrentPos = NextPos;
		}
		else
		{
			break;
		}
	}

	return OutBestPosition != AIUnit->GetGridPosition();
}

void ATurnManager::ProcessAIAttacks(AMyGameMode* GameMode)
{
	if (!GameMode || !GameMode->UnitActions) return;

	for (AUnit* AIUnit : GameMode->AIUnits)
	{
		// Simple AI attack logic - attacks first available target
		for (AUnit* PlayerUnit : GameMode->PlayerUnits)
		{
			if (GameMode->UnitActions->AttackUnit(AIUnit, PlayerUnit))
			{
				break; // Attack once per unit
			}
		}
	}
}

AUnit* ATurnManager::FindNearestEnemy(AUnit* AIUnit)
{
	if (!AIUnit) return nullptr;

	// Properly get GameMode from the world
	AMyGameMode* GameMode = Cast<AMyGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode || !GameMode->PlayerUnits.Num()) return nullptr;

	AUnit* NearestEnemy = nullptr;
	float MinDistance = TNumericLimits<float>::Max();

	for (AUnit* PlayerUnit : GameMode->PlayerUnits)
	{
		if (!PlayerUnit) continue;

		float Distance = FVector2D::Distance(
			AIUnit->GetGridPosition(),
			PlayerUnit->GetGridPosition()
		);

		if (Distance < MinDistance)
		{
			MinDistance = Distance;
			NearestEnemy = PlayerUnit;
		}
	}

	return NearestEnemy;
}

// Add to TurnManager.cpp
void ATurnManager::CheckTurnCompletion(AMyGameMode* GameMode)
{
	if (!GameMode) return;

	bool bAllUnitsActed = true;
	TArray<AUnit*>& CurrentUnits = GameMode->bIsPlayerTurn ? GameMode->PlayerUnits : GameMode->AIUnits;
    
	for (AUnit* Unit : CurrentUnits)
	{
		if (Unit && Unit->Health > 0 && 
		   (!Unit->bHasMovedThisTurn || !Unit->bHasAttackedThisTurn))
		{
			bAllUnitsActed = false;
			break;
		}
	}

	if (bAllUnitsActed)
	{
		// Clean up before turn end
		GameMode->HideActionWidget();
		if (GameMode->SelectedUnit)
		{
			GameMode->SelectedUnit->SetSelected(false);
			GameMode->SelectedUnit = nullptr;
		}

		// Start next turn after delay
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [GameMode]()
		{
			GameMode->EndTurn();
		}, 1.0f, false);
	}
}