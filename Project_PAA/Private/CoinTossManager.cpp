

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
	OnCoinTossComplete.Broadcast(bIsPlayerTurn);
}