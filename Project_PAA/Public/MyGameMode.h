#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameMode.generated.h"

// Forward declarations
class AGridManager;
class UPlacementWidget;
class ACoinTossManager;
class UCoinWidget;
class AUnit;

UCLASS()
class PROJECT_PAA_API AMyGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMyGameMode();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // Function to start the placement phase
    void StartPlacementPhase();

    // Function to handle unit placement
    void HandleUnitPlacement(FVector2D CellPosition);

    // Function to handle AI placement
    void HandleAIPlacement();

    // Function to set the selected unit type
    void SetSelectedUnitType(const FString& UnitType);

    // Handle coin toss result
    UFUNCTION()
    void HandleCoinTossResult(bool bIsPlayerTurnResult);

    // Function to start the player's turn
    void StartPlayerTurn();
    void HandleCellClick(FVector2D Vector2);

    UFUNCTION(BlueprintCallable)
    const TArray<FString>& GetPlayerUnitsToPlace() const { return PlayerUnitsToPlace; }

    UFUNCTION(BlueprintCallable)
    bool CanSelectUnitType(const FString& UnitType) const;
    // Track whose turn it is to place units
    bool bIsPlayerTurn;

   
    // Function to check if a cell is valid for placement
    bool IsCellValidForPlacement(FVector2D CellPosition);


    /*UFUNCTION(BlueprintCallable)
    bool CanPlaceSniper() const { return !bHasPlacedSniper && PlayerUnitsToPlace.Contains("Sniper"); }
    
    UFUNCTION(BlueprintCallable)
    bool CanPlaceBrawler() const { return !bHasPlacedBrawler && PlayerUnitsToPlace.Contains("Brawler"); }*/

    bool PlaceUnit(const FString& UnitType, const FVector2D& CellPosition);
    
    UFUNCTION(BlueprintCallable)
    bool CanPlaceSniper() const;

        
    UFUNCTION(BlueprintCallable)
    bool CanPlaceBrawler() const;

private:
    // Track which units need to be placed
    TArray<FString> PlayerUnitsToPlace;
    TArray<FString> AIUnitsToPlace;

    // Currently selected unit type for placement
    FString SelectedUnitType;

    // Reference to the GridManager
    UPROPERTY()
    AGridManager* GridManager;

    // Reference to the PlacementWidget
    UPROPERTY()
    UPlacementWidget* PlacementWidget;

    UPROPERTY()
    UCoinWidget* CoinWidget;

    // Widget class to use for the placement UI
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UPlacementWidget> PlacementWidgetClass;

    // Widget class to use for the coin toss UI
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UCoinWidget> CoinWidgetClass;

    // Reference to the CoinTossManager
    UPROPERTY()
    ACoinTossManager* CoinTossManager;


    bool bHasPlacedSniper = false;
    bool bHasPlacedBrawler = false;
   
};