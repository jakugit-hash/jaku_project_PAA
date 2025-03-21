
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameMode.generated.h"

// Forward declarations
class ACoinTossManager;
class UCoinWidget;

UCLASS()
class PROJECT_PAA_API AMyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMyGameMode();

protected:
	virtual void BeginPlay() override;

public:
	// Function to spawn the grid and units
	void InitializeGame();

	// Function to start the placement phase
	void StartPlacementPhase();

	// Function to handle unit placement
	void HandleUnitPlacement(FVector2D CellPosition);

	// Function to handle AI placement
	void HandleAIPlacement();

	// Function to set the selected unit type
	void SetSelectedUnitType(const FString& UnitType);



	/*// Turn management
	void StartPlayerTurn();
	void StartAITurn();
	void EndTurn();
	*/

	// Handle coin toss result
	UFUNCTION()
	void HandleCoinTossResult(bool bIsPlayerTurnResult);

private:


	// Track which units need to be placed
	TArray<FString> PlayerUnitsToPlace;
	TArray<FString> AIUnitsToPlace;

	// Currently selected unit type for placement
	FString SelectedUnitType;

	
	// Reference to the GridManager
	UPROPERTY()
	class AGridManager* GridManager;

	UPROPERTY()
	UPlacementWidget* PlacementWidget;

	// Function to place a unit on the grid
	void PlaceUnit(FString UnitType, FVector2D CellPosition);

	// Function to check if a cell is valid for placement
	bool IsCellValidForPlacement(FVector2D CellPosition);
	
	// Turn tracking
	bool bIsPlayerTurn;

	/*// Coin Toss Manager
	UPROPERTY()
	ACoinTossManager* CoinTossManager;
	
	// Coin Toss Widget
	UPROPERTY()
	TSubclassOf<UUserWidget> CoinWidget;

	// Widget class to use for the coin toss UI
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> CoinWidgetClass; // La classe del widget (Blueprint)
	
	UUserWidget* CoinWidgetInstance; // L'istanza effettiva*/

};