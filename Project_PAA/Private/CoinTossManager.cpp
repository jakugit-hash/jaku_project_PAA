

#include "CoinTossManager.h"

ACoinTossManager::ACoinTossManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACoinTossManager::BeginPlay()
{
	Super::BeginPlay();
}

bool ACoinTossManager::PerformCoinToss()
{
	// Randomly decide heads (true) or tails (false)
	return FMath::RandBool();
}

void ACoinTossManager::DecideStartingPlayer()
{
	bool bIsPlayerTurn = PerformCoinToss();
	UE_LOG(LogTemp, Warning, TEXT("Coin toss result: %s"), bIsPlayerTurn ? TEXT("Player") : TEXT("AI"));

	// Broadcast the result
	if (OnCoinTossComplete.IsBound())
	{
		OnCoinTossComplete.Broadcast(bIsPlayerTurn);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("OnCoinTossComplete is not bound!"));
	}
}

