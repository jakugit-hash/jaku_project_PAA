
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoinTossManager.generated.h"

UCLASS()
class PROJECT_PAA_API ACoinTossManager : public AActor
{
	GENERATED_BODY()

public:
	ACoinTossManager();

protected:
	virtual void BeginPlay() override;

public:
	// Function to perform the coin toss
	bool PerformCoinToss();

	// Function to decide who starts the game
	UFUNCTION(BlueprintCallable, Category = "Coin Toss")
	void DecideStartingPlayer();

	// Event to notify the result of the coin toss
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoinTossComplete, bool, bIsPlayerTurn);
	UPROPERTY(BlueprintAssignable, Category = "Coin Toss")
	FOnCoinTossComplete OnCoinTossComplete;
};