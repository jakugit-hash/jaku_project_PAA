#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GlobalEnums.h"
#include "MyGameMode.generated.h"


// Forward declarations
class AGridManager;
class ATurnManager;
class AUnitActions;
class AUnit;
class AGridCell;
class UPlacementWidget;
class ACoinTossManager;
class UCoinWidget;
class UWBP_ActionWidget;


UCLASS()
class PROJECT_PAA_API AMyGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMyGameMode();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    void LogTurnState();
    

    void InitGameplayManagers();

    
public:
    void HandleActionPhase();
    void SetupPlayerInput();
    // Function to start the placement phase
    void StartPlacementPhase();

    // Function to handle unit placement
    void HandleUnitPlacement(FVector2D CellPosition);

    // Function to handle AI placement
    void HandleAIPlacement();

    // Function to set the selected unit type
    void SetSelectedUnitType(const FString& UnitType);

    // Function to start the player's turn
    void StartPlayerTurn();

    void StartActionPhase(); // Called when placement ends

     // Track whose turn it is to place units
        bool bIsPlayerTurn;
        
        // Function to check if a cell is valid for placement
        bool IsCellValidForPlacement(FVector2D CellPosition);

 bool PlaceUnit(const FString& UnitType, const FVector2D& CellPosition);

    UFUNCTION()
    void CheckTurnCompletion();
    
    void ShowActionWidget(AUnit* SelectedUnit);
    // Handle coin toss result
    UFUNCTION()
    void HandleCoinTossResult(bool bIsPlayerTurnResult);
    void HandlePlacementPhase();

    UFUNCTION(BlueprintCallable, BlueprintPure)
    const TArray<FString>& GetPlayerUnitsToPlace() const { return PlayerUnitsToPlace; }

    UFUNCTION(BlueprintCallable)
    bool CanSelectUnitType(const FString& UnitType) const;

    UFUNCTION(BlueprintCallable)
       bool CanPlaceSniper() const;

    
     UFUNCTION(BlueprintCallable)
        bool CanPlaceBrawler() const;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    AUnit* SelectedUnit = nullptr;  // Track currently selected unit

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EGamePhase CurrentGamePhase = EGamePhase::Placement;

    UPROPERTY(BlueprintReadOnly)
    TArray<AUnit*> PlayerUnits;

    UPROPERTY(BlueprintReadOnly)
    TArray<AUnit*> AIUnits;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    AGridManager* GridManager;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    AUnitActions* UnitActions= nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    ATurnManager* TurnManager= nullptr;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bWaitingForPlayerAction = false;


    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bWaitingForMoveTarget; 

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bWaitingForAttackTarget;
    
    UFUNCTION()
    void HandleMoveAction();

    UFUNCTION()
    void HandleAttackAction();

    UFUNCTION()
    void EndPlayerTurn();
    

    UFUNCTION(BlueprintCallable, Category="UI")
    void HideActionWidget();

    UFUNCTION()
    void HandleUnitSelection(AUnit* NewSelection);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bWaitingForMove = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bWaitingForAttack = false;
    
    UFUNCTION()
    void ClearSelection();

    UPROPERTY()
    class UWBP_ActionWidget* ActionWidget;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UWBP_ActionWidget> ActionWidgetClass;

    
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

    UFUNCTION(BlueprintCallable)
    void EndTurn();

    UPROPERTY(Transient)
    bool bActionPhaseStarted = false;

    

private:
    
    
    // Track which units need to be placed
    TArray<FString> PlayerUnitsToPlace;
    TArray<FString> AIUnitsToPlace;

    // Currently selected unit type for placement
    FString SelectedUnitType;

    
   
    bool bHasPlacedSniper = false;
    bool bHasPlacedBrawler = false;
   
};