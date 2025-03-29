#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GlobalEnums.h"
#include "TurnManager.generated.h"

class AMyGameMode;
class AUnit;

UCLASS()
class PROJECT_PAA_API ATurnManager : public AActor
{
	GENERATED_BODY()

public:
	ATurnManager();

	UFUNCTION(BlueprintCallable)
	void StartActionPhase(AMyGameMode* GameMode);

	UFUNCTION(BlueprintCallable)
	void EndTurn(AMyGameMode* GameMode);

	UFUNCTION(BlueprintCallable)
	void ExecuteAITurn(AMyGameMode* GameMode);

	UFUNCTION(BlueprintCallable)
	void CheckTurnCompletion(AMyGameMode* GameMode);

protected:
	virtual void BeginPlay() override;

private:
	void ProcessAIMovement(AMyGameMode* GameMode);
	void ProcessAIAttacks(AMyGameMode* GameMode);
	AUnit* FindNearestEnemy(AUnit* AIUnit);
	bool FindMovePositionForAI(AUnit* AIUnit, FVector2D& OutBestPosition);
	
};