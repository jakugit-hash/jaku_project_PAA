#include "UnitActions.h"
#include "Unit.h"
#include "Sniper.h"
#include "Brawler.h"
#include "GridManager.h"
#include "TurnManager.h"
#include "MyGameMode.h"
#include "GridCell.h"
#include "Kismet/GameplayStatics.h"

AUnitActions::AUnitActions()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AUnitActions::BeginPlay()
{
	Super::BeginPlay();
}

bool AUnitActions::MoveUnit(AUnit* Unit, FVector2D TargetPosition)
{
	if (!Unit || Unit->bHasMovedThisTurn) return false;

	AGridManager* GridManager = GetGridManager();
	if (!GridManager) return false;

	TArray<FVector2D> Path = GridManager->AStarPathfind(Unit->GetGridPosition(), TargetPosition, Unit->MovementRange);

	if (Path.Num() == 0 || Path.Last() != TargetPosition)
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid path to the target!"));
		return false;
	}

	Unit->MoveToCell(TargetPosition); // aggiorna tutto
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

	float Distance = FMath::Abs(TargetPosition.X - Unit->GetGridPosition().X) +
		FMath::Abs(TargetPosition.Y - Unit->GetGridPosition().Y);
	if (Distance > Unit->MovementRange) return false;

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
	if (!Attacker || !Target)
	{
		UE_LOG(LogTemp, Error, TEXT("Attacker o Target nulli!"));
		return false;
	}

	if (!Attacker->CanAttack())
	{
		UE_LOG(LogTemp, Warning, TEXT("Attacker %s non può attaccare!"), *Attacker->GetName());
		return false;
	}

	FVector2D AttPos = Attacker->GetGridPosition();
	FVector2D TargetPos = Target->GetGridPosition();

	int32 Distance = FMath::Abs(AttPos.X - TargetPos.X) + FMath::Abs(AttPos.Y - TargetPos.Y);
	UE_LOG(LogTemp, Warning, TEXT("Attacker at (%.0f, %.0f), Target at (%.0f, %.0f), Range: %d, Distance: %d"),
		AttPos.X, AttPos.Y, TargetPos.X, TargetPos.Y, Attacker->AttackRange, Distance);

	if (Distance > Attacker->AttackRange)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack failed - Out of range"));
		return false;
	}

	if (!Attacker->IsA(ASniper::StaticClass()))
	{
		if (AMyGameMode* GM = Cast<AMyGameMode>(Attacker->GetWorld()->GetAuthGameMode()))
		{
			if (AGridManager* GridManager = GM->GridManager)
			{
				AGridCell* FromCell = GridManager->GetCellAtPosition(AttPos);
				AGridCell* ToCell = GridManager->GetCellAtPosition(TargetPos);
				if (FromCell && ToCell)
				{
					bool bBlocked = GridManager->IsPathBlocked(FromCell, ToCell);
					if (bBlocked)
					{
						UE_LOG(LogTemp, Warning, TEXT("Attack failed - Obstacle in path"));
						return false;
					}
				}
			}
		}
	}

	int32 Damage = FMath::RandRange(Attacker->MinDamage, Attacker->MaxDamage);
	Target->Health -= Damage;

	UE_LOG(LogTemp, Warning, TEXT("%s ha attaccato %s causando %d danni"), *Attacker->GetName(), *Target->GetName(), Damage);

	if (Target->Health <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s è stato distrutto!"), *Target->GetName());
		Target->DestroyUnit();
	}

	// contrattacco
	if (Distance == 1 && Target->CanAttack())
	{
		int32 CounterDamage = FMath::RandRange(1, 3);
		Attacker->Health -= CounterDamage;

		UE_LOG(LogTemp, Warning, TEXT("%s ha ricevuto un contrattacco da %s con %d danni"), *Attacker->GetName(), *Target->GetName(), CounterDamage);

		if (Attacker->Health <= 0)
		{
			Attacker->DestroyUnit();
			UE_LOG(LogTemp, Warning, TEXT("%s è stato distrutto dal contrattacco!"), *Attacker->GetName());
		}
	}

	Attacker->bHasAttackedThisTurn = true;
	return true;
}
