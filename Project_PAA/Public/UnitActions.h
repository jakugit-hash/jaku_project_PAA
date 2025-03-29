#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnitActions.generated.h"

class AUnit;
class AGridManager;

UCLASS()
class PROJECT_PAA_API AUnitActions : public AActor
{
	GENERATED_BODY()

public:
	AUnitActions();

	UFUNCTION(BlueprintCallable)
	bool MoveUnit(AUnit* Unit, FVector2D TargetPosition);

	UFUNCTION(BlueprintCallable)
	bool AttackUnit(AUnit* Attacker, AUnit* Target);

	/*// Add this to public section
	UFUNCTION(BlueprintCallable)
	void CheckTurnCompletion(AMyGameMode* GameMode);
	*/

protected:
	virtual void BeginPlay() override;

private:
	bool IsValidMove(AUnit* Unit, FVector2D TargetPosition);
	bool IsValidAttack(AUnit* Attacker, AUnit* Target);
	AGridManager* GetGridManager() const;
};