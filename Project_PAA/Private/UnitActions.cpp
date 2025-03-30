#include "UnitActions.h"
#include "Unit.h"
#include "GridManager.h"
#include "TurnManager.h" 
#include "MyGameMode.h"
#include "Kismet/GameplayStatics.h"

AUnitActions::AUnitActions()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AUnitActions::BeginPlay()
{
	Super::BeginPlay();
	// Initialization code if needed
}

bool AUnitActions::MoveUnit(AUnit* Unit, FVector2D TargetPosition)
{
	if (!Unit || Unit->bHasMovedThisTurn) return false;

	AGridCell* TargetCell = GetGridManager()->GetCellAtPosition(TargetPosition);
	if (!TargetCell || TargetCell->IsObstacle() || TargetCell->IsOccupied())
		return false;

	
	const AGridManager* GridManager = GetGridManager();
	if (!GridManager) return false;

	// Validate distance (Manhattan)
	int32 Distance = FMath::Abs(TargetPosition.X - Unit->GetGridPosition().X) + 
					FMath::Abs(TargetPosition.Y - Unit->GetGridPosition().Y);
	if (Distance > Unit->MovementRange) return false;
	

	// Execute move
	Unit->SetGridPosition(TargetPosition);
	Unit->bHasMovedThisTurn = true;
	Unit->bIsSelected = false;

	if (AMyGameMode* GameMode = Cast<AMyGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (ATurnManager* TurnManager = GameMode->TurnManager)
		{
			TurnManager->CheckTurnCompletion(GameMode);
		}
	}
	return true;
}

bool AUnitActions::IsValidMove(AUnit* Unit, FVector2D TargetPosition)
{
	AGridManager* GridManager = GetGridManager();
	if (!GridManager || !Unit) return false;

	// Check distance
	float Distance = FMath::Abs(TargetPosition.X - Unit->GetGridPosition().X) + 
					FMath::Abs(TargetPosition.Y - Unit->GetGridPosition().Y);
	if (Distance > Unit->MovementRange) return false;

	// Check if target cell is free
	if (AGridCell* TargetCell = GridManager->GetCellAtPosition(TargetPosition))
	{
		return !TargetCell->IsOccupied() && !TargetCell->IsObstacle();
	}
	return false;
}

AGridManager* AUnitActions::GetGridManager() const
{
	TArray<AActor*> GridManagers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGridManager::StaticClass(), GridManagers);
	return GridManagers.Num() > 0 ? Cast<AGridManager>(GridManagers[0]) : nullptr;
}


bool AUnitActions::AttackUnit(AUnit* Attacker, AUnit* Target)
{
	if (!Attacker || !Target || 
	!Attacker->CanAttack() ||  // Use the unit's built-in check
	FVector2D::Distance(Attacker->GetGridPosition(), Target->GetGridPosition()) > Attacker->AttackRange)
	{
		return false;
	}

	// Range check
	float Distance = FVector2D::Distance(
		Attacker->GetGridPosition(),
		Target->GetGridPosition()
	);
    
	if (Distance > Attacker->AttackRange)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack failed - Out of range"));
		return false;
	}

	// Damage calculation
	int32 Damage = FMath::RandRange(Attacker->MinDamage, Attacker->MaxDamage);
	Target->Health -= Damage;

	// Handle target death
	if (Target->Health <= 0)
	{
		Target->Destroy();
	}

	// Update attacker state
	Attacker->bHasAttackedThisTurn = true;
	return true;
}