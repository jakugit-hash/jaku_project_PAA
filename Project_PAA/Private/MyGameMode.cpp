#include "MyGameMode.h"
#include "GridManager.h"
#include "Components/Button.h"
#include "PlacementWidget.h"
#include "Sniper.h"
#include "Brawler.h"
#include "Unit.h"
#include "CoinWidget.h"
#include "CoinTossManager.h"
#include "GlobalEnums.h"
#include "TurnManager.h"   
#include "UnitActions.h"
#include "WBP_ActionWidget.h"
#include "Kismet/GameplayStatics.h"


AMyGameMode::AMyGameMode(): bWaitingForMoveTarget(false),
      bWaitingForAttackTarget(false),
      ActionWidget(nullptr),
      CurrentActionState(EUnitActionState::None)
{
    // Set default values
    bIsPlayerTurn = false;

    //  pointers 
    CoinWidget = nullptr;
    GridManager = nullptr;
    PlacementWidget = nullptr;
    CoinTossManager = nullptr;
    UnitActions = nullptr;
    TurnManager = nullptr;
    SelectedUnit = nullptr;

    // Assign the PlacementWidgetClass in the constructor
    static ConstructorHelpers::FClassFinder<UUserWidget> PlacementWidgetBP(TEXT("/Game/widgets/WBP_PlacementWidget"));
    if (PlacementWidgetBP.Succeeded())
    {
        PlacementWidgetClass = PlacementWidgetBP.Class;
        UE_LOG(LogTemp, Warning, TEXT("PlacementWidgetClass assigned successfully!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find PlacementWidgetClass!"));
    }

    // Assign the CoinWidgetClass in the constructor
    static ConstructorHelpers::FClassFinder<UUserWidget> CoinWidgetBP(TEXT("/Game/Widgets/WBP_CoinWidget"));
    if (CoinWidgetBP.Succeeded())
    {
        CoinWidgetClass = CoinWidgetBP.Class;
        UE_LOG(LogTemp, Warning, TEXT("CoinWidgetClass assigned successfully!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find CoinWidgetClass!"));
    }

    static ConstructorHelpers::FClassFinder<UUserWidget> WidgetFinder(TEXT("/Game/widgets/WBP_ActionWidget"));
    if (WidgetFinder.Succeeded())
    {
        ActionWidgetClass = WidgetFinder.Class;
        UE_LOG(LogTemp, Warning, TEXT("Found ActionWidget class: %s"), *GetNameSafe(ActionWidgetClass));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find ActionWidget class!"));
    }
}

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    InitGameplayManagers();
    
    // Find and assign the GridManager
    GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
    if (!GridManager)
    {
        UE_LOG(LogTemp, Error, TEXT("GridManager not found in the level!"));
        return;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GridManager found and assigned successfully!"));
    }

    // Spawn the CoinTossManager
    CoinTossManager = GetWorld()->SpawnActor<ACoinTossManager>();
    if (!CoinTossManager)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn CoinTossManager!"));
        return;
    }
    
    
    // Bind the coin toss result handler
    CoinTossManager->OnCoinTossComplete.AddDynamic(this, &AMyGameMode::HandleCoinTossResult);

    // Create and display the CoinWidget
    if (CoinWidgetClass)
    {
        CoinWidget = CreateWidget<UCoinWidget>(GetWorld(), CoinWidgetClass);
        if (CoinWidget)
        {
            CoinWidget->SetCoinTossManager(CoinTossManager);
            CoinWidget->AddToViewport();

            // Set input mode to UI only
            APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
            if (PlayerController)
            {
                PlayerController->SetInputMode(FInputModeUIOnly());
                PlayerController->bShowMouseCursor = true;
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create CoinWidget!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CoinWidgetClass is null!"));
    }

    TurnManager = GetWorld()->SpawnActor<ATurnManager>();
    UnitActions = GetWorld()->SpawnActor<AUnitActions>();

    if (!GridManager || !TurnManager || !UnitActions)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Failed to initialize gameplay systems!"));
    }
    /*if (ActionWidgetClass)
    {
        ActionWidget = CreateWidget<UWBP_ActionWidget>(GetWorld(), ActionWidgetClass);
        if (ActionWidget)
        {
            ActionWidget->AddToViewport();
            ActionWidget->SetVisibility(ESlateVisibility::Collapsed);
            ActionWidget->Setup(this); 
        }
    }*/
}


void AMyGameMode::HandleCoinTossResult(bool bIsPlayerTurnResult)
{
    UE_LOG(LogTemp, Warning, TEXT("AMyGameMode::HandleCoinTossResult called! Result: %s"), bIsPlayerTurnResult ? TEXT("Player") : TEXT("AI"));

    // Set who starts the placement phase
    bIsPlayerTurn = bIsPlayerTurnResult;

    // Remove the CoinWidget from the viewport
    if (CoinWidget)
    {
        CoinWidget->RemoveFromParent();
        CoinWidget = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("CoinWidget removed from viewport!"));
    }

    // Start the placement phase
    StartPlacementPhase();
}

void AMyGameMode::HandlePlacementPhase()
{
    UE_LOG(LogTemp, Warning, TEXT("Handling Placement Phase"));
    
    if (PlayerUnitsToPlace.Num() > 0)
    {
        if (!PlacementWidget && PlacementWidgetClass)
        {
            PlacementWidget = CreateWidget<UPlacementWidget>(GetWorld(), PlacementWidgetClass);
            if (PlacementWidget)
            {
                PlacementWidget->SetGameMode(this);
                PlacementWidget->AddToViewport();
                UE_LOG(LogTemp, Warning, TEXT("Placement widget shown"));
            }
        }
    }
    else
    {
        StartActionPhase();
    }
}

void AMyGameMode::HandleActionPhase()
{
    UE_LOG(LogTemp, Warning, TEXT("Handling Action Phase"));
    
    // Auto-select first available unit
    for (AUnit* Unit : PlayerUnits)
    {
        if (!Unit->bHasMovedThisTurn || !Unit->bHasAttackedThisTurn)
        {
            SelectedUnit = Unit;
            ShowActionWidget(Unit);
            UE_LOG(LogTemp, Warning, TEXT("Auto-selected unit %s for actions"), *Unit->GetName());
            break;
        }
    }
}

void AMyGameMode::SetupPlayerInput()
{
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
        PC->bEnableClickEvents = true;
        PC->bEnableMouseOverEvents = true;
        UE_LOG(LogTemp, Warning, TEXT("Player input configured"));
    }
}
void AMyGameMode::StartPlacementPhase()
{
    UE_LOG(LogTemp, Warning, TEXT("AMyGameMode::StartPlacementPhase called!"));

    // Initialize units to place
    PlayerUnitsToPlace = { TEXT("Sniper"), TEXT("Brawler") };
    AIUnitsToPlace = { TEXT("Sniper"), TEXT("Brawler") };

    // Create and display the PlacementWidget
    if (PlacementWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Creating PlacementWidget..."));
        PlacementWidget = CreateWidget<UPlacementWidget>(GetWorld(), PlacementWidgetClass);
        if (PlacementWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("PlacementWidget created successfully!"));
            PlacementWidget->SetGameMode(this);
            PlacementWidget->AddToViewport();

            // Set input mode to UI only
            if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
            {
                FInputModeGameAndUI InputMode;
                InputMode.SetWidgetToFocus(PlacementWidget->TakeWidget());
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                PlayerController->bEnableClickEvents = true;
                PlayerController->bEnableMouseOverEvents = true;                PlayerController->SetInputMode(InputMode);
                PlayerController->bShowMouseCursor = true;
                // Debug: Print input settings
                UE_LOG(LogTemp, Warning, TEXT("PlayerController settings - Click: %d, MouseOver: %d"), 
                    PlayerController->bEnableClickEvents, PlayerController->bEnableMouseOverEvents);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create PlacementWidget!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlacementWidgetClass is null!"));
    }

    // Start with the winner of the coin toss
    if (bIsPlayerTurn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player starts placing units."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AI starts placing units."));
        HandleAIPlacement();
    }
}

void AMyGameMode::SetSelectedUnitType(const FString& UnitType)
{
    SelectedUnitType = UnitType;
}

bool AMyGameMode::CanSelectUnitType(const FString& UnitType) const
{
    return PlayerUnitsToPlace.Contains(UnitType);
}


void AMyGameMode::HandleUnitPlacement(FVector2D CellPosition)
{
    // Validate grid and cell
    if (!GridManager || !IsCellValidForPlacement(CellPosition)) 
    {
        return;
    }
    
    // Player placement
    if (bIsPlayerTurn)
    {
        // STRICT validation - must have unit type available
        if (SelectedUnitType.IsEmpty() || !CanSelectUnitType(SelectedUnitType)  || 
           (SelectedUnitType == "Sniper" && bHasPlacedSniper) ||
           (SelectedUnitType == "Brawler" && bHasPlacedBrawler))
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot place %s - invalid selection"), *SelectedUnitType);
            return;
        }

        // Place unit
        if (PlaceUnit(SelectedUnitType, CellPosition))
        {

            if (SelectedUnitType == "Sniper") bHasPlacedSniper = true;
            else if (SelectedUnitType == "Brawler") bHasPlacedBrawler = true;
            
            PlayerUnitsToPlace.Remove(SelectedUnitType);
            SelectedUnitType = "";
            
            if (PlacementWidget) 
            {
                PlacementWidget->ClearSelection();
                // Manually update button states
                PlacementWidget->SniperButton->SetIsEnabled(PlayerUnitsToPlace.Contains("Sniper"));
                PlacementWidget->BrawlerButton->SetIsEnabled(PlayerUnitsToPlace.Contains("Brawler"));
                PlacementWidget->SniperButton->SetIsEnabled(!bHasPlacedSniper);
                PlacementWidget->BrawlerButton->SetIsEnabled(!bHasPlacedBrawler);
            }
        }
    }
    // AI placement (unchanged)
    else if (AIUnitsToPlace.Num() > 0)
    {
        FString AISelection = AIUnitsToPlace[0];
        PlaceUnit(AISelection, CellPosition);
        AIUnitsToPlace.RemoveAt(0);
    }

   

    bIsPlayerTurn = !bIsPlayerTurn;
    
    if (!bIsPlayerTurn && AIUnitsToPlace.Num() > 0)
    {
        HandleAIPlacement();
    }
    // Original turn logic below
         if (PlayerUnitsToPlace.Num() == 0 && AIUnitsToPlace.Num() == 0)
         {
             StartActionPhase();
         
         }

    
}


void AMyGameMode::HandleAIPlacement()
{
    if (AIUnitsToPlace.Num() == 0) {
        // DEBUG
        UE_LOG(LogTemp, Warning, TEXT("AI has no more units to place"));
        return;
    }

    // Find random empty cell
    int32 X, Y;
    if (GridManager->FindRandomEmptyCell(X, Y))
    {
        FString UnitType = AIUnitsToPlace[0];
        if (PlaceUnit(UnitType, FVector2D(X, Y)))
        {
            AIUnitsToPlace.RemoveAt(0);
            UE_LOG(LogTemp, Warning, TEXT("AI placed %s at (%d,%d)"), *UnitType, X, Y);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AI couldn't find empty cell!"));
    }

    // Continue placement or start action phase
    if (AIUnitsToPlace.Num() == 0 && PlayerUnitsToPlace.Num() == 0)
    {
        StartActionPhase();
    }
    else
    {
        // Switch back to player for next placement
        bIsPlayerTurn = !bIsPlayerTurn;
    }
}

bool AMyGameMode::PlaceUnit(const FString& UnitType, const FVector2D& CellPosition)
{
   
    AUnit* NewUnit = nullptr;
 FVector WorldPosition = GridManager->GetCellWorldPosition(CellPosition.X, CellPosition.Y);

    AGridCell* Cell = GridManager->GetCellAtPosition(CellPosition);
    if (!Cell || Cell->IsObstacle() || Cell->IsOccupied()) 
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid placement position!"));
        return false;
    }

    if (UnitType == TEXT("Sniper"))
    {
        NewUnit = GetWorld()->SpawnActor<ASniper>(ASniper::StaticClass(), WorldPosition, FRotator::ZeroRotator);
    }
    else if (UnitType == TEXT("Brawler"))
    {
        NewUnit = GetWorld()->SpawnActor<ABrawler>(ABrawler::StaticClass(), WorldPosition, FRotator::ZeroRotator);
    }

    if (NewUnit)
    {
        NewUnit->SetGridPosition(CellPosition);
        NewUnit->SetAsPlayerUnit(bIsPlayerTurn);
        UE_LOG(LogTemp, Warning, TEXT("%s placed at (%f, %f)"), *UnitType, CellPosition.X, CellPosition.Y);
        
        if (bIsPlayerTurn)
            PlayerUnits.Add(NewUnit);
        else
            AIUnits.Add(NewUnit);
            
        return true; // FIX: Consistent return
    }
    
    return false;
    
}

void AMyGameMode::CheckTurnCompletion()
{
    if (TurnManager)
    {
        TurnManager->CheckTurnCompletion(this);
    }
}

bool AMyGameMode::IsCellValidForPlacement(FVector2D CellPosition)
{
    AGridCell* Cell = GridManager->GetCellAtPosition(CellPosition);
    return Cell && !Cell->IsObstacle() && !Cell->IsOccupied();
}

bool AMyGameMode::CanPlaceSniper() const
{
    return !bHasPlacedSniper && PlayerUnitsToPlace.Contains("Sniper");
}

bool AMyGameMode::CanPlaceBrawler() const
{
    return !bHasPlacedBrawler && PlayerUnitsToPlace.Contains("Brawler");
}


/*
            // Switch turns- possible option to use if the other doesnt work
            bIsPlayerTurn = false;
            HandleAIPlacement();
   
}*/



/*
void AMyGameMode::PrepareForPlayerActions()
{
    // Reset selection states
    SelectedUnit = nullptr;
    bWaitingForPlayerAction = true;
    
}*/

void AMyGameMode::HandleUnitSelection(AUnit* NewSelection)
{
    if (!NewSelection || !NewSelection->bIsPlayerUnit || !bIsPlayerTurn) return;

    if (SelectedUnit == NewSelection)
    {
        // GridManager->ClearHighlights();
        // SelectedUnit->SetSelected(false);
        // SelectedUnit = nullptr;
        // //GridManager->ClearHighlights();
        bMovementRangeVisible = false;
        HandleMoveAction();
        HideActionWidget();
        return;
    }

    // cambio unità
    if (SelectedUnit) SelectedUnit->SetSelected(false);
    SelectedUnit = NewSelection;
    SelectedUnit->SetSelected(true);

    GridManager->ClearHighlights(); // rimuovi vecchio highlight
    bMovementRangeVisible = false;

    ShowActionWidget(SelectedUnit);
}



void AMyGameMode::ClearSelection()
{
   

    bMovementRangeVisible = false;
    bIsAttackHighlighted = false;

    if (GridManager)
    {
        GridManager->ClearHighlights();
    }
    
 if (SelectedUnit)
    {
        SelectedUnit->SetSelected(false);
        SelectedUnit = nullptr;
    }
    HideActionWidget();
}


void AMyGameMode::StartPlayerTurn()
{
    bIsPlayerTurn = true;
    UE_LOG(LogTemp, Warning, TEXT("=== PLAYER TURN START ==="));
    bWaitingForPlayerAction = false; // Reset this flag

    for (AUnit* Unit : PlayerUnits)
    {
        Unit->bHasMovedThisTurn = false;
        Unit->bHasAttackedThisTurn = false;
        UE_LOG(LogTemp, Warning, TEXT("Reset unit %s for new turn"), *Unit->GetName());
    }

    // Handle different phases
    if (CurrentGamePhase == EGamePhase::Placement)
    {
        HandlePlacementPhase();
    }
    else // UnitAction phase
    {
        HandleActionPhase();
    }

    // Common input setup
    SetupPlayerInput();
    
   
}
void AMyGameMode::InitGameplayManagers()
{
    TurnManager = GetWorld()->SpawnActor<ATurnManager>();
    UnitActions = GetWorld()->SpawnActor<AUnitActions>();

    if (!TurnManager)
    {
        TurnManager = GetWorld()->SpawnActor<ATurnManager>();
    }
    
    if (!UnitActions)
    {
        UnitActions = GetWorld()->SpawnActor<AUnitActions>();
        UnitActions->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
    }
    if (!TurnManager || !UnitActions)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn gameplay managers!"));
    }
}


void AMyGameMode::StartActionPhase()
{
    if (bActionPhaseStarted) return;

    bActionPhaseStarted = true;
    CurrentGamePhase = EGamePhase::UnitAction;
    UE_LOG(LogTemp, Warning, TEXT("=== ACTION PHASE STARTED ==="));

    // Rimuovi widget di piazzamento se presente
    if (PlacementWidget)
    {
        PlacementWidget->RemoveFromParent();
        PlacementWidget = nullptr;
    }

    // CREA E CONFIGURA L'ACTION WIDGET
    if (ActionWidgetClass)
    {
        ActionWidget = CreateWidget<UWBP_ActionWidget>(GetWorld(), ActionWidgetClass);
        if (ActionWidget)
        {
            ActionWidget->AddToViewport();
            ActionWidget->Setup(this); 
            UE_LOG(LogTemp, Warning, TEXT("ActionWidget creato e Setup eseguito"));
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Player Units: %d, AI Units: %d"), PlayerUnits.Num(), AIUnits.Num());

    if (bIsPlayerTurn)
    {
        HandleActionPhase();
        UE_LOG(LogTemp, Warning, TEXT("Player turn started - awaiting input"));
    }
    else
    {
        // Turno AI
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            if (TurnManager) TurnManager->ExecuteAITurn(this);
        }, 1.0f, false);
    }
}



void AMyGameMode::EndTurn()
{
    bIsPlayerTurn = !bIsPlayerTurn;
    
    // Reset unit states
    for (AUnit* Unit : bIsPlayerTurn ? PlayerUnits : AIUnits)
    {
        Unit->bHasMovedThisTurn = false;
        Unit->bHasAttackedThisTurn = false;
    }

    // Start next turn
    if (bIsPlayerTurn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player turn started!"));
    }
    else if (TurnManager)
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            TurnManager->ExecuteAITurn(this);
        }, 0.5f, false);
    }
}


void AMyGameMode::LogTurnState()
{
    UE_LOG(LogTemp, Warning, TEXT("Turn State - Phase: %d, PlayerTurn: %d"), 
        (int32)CurrentGamePhase, 
        bIsPlayerTurn);
}

void AMyGameMode::ShowActionWidget(AUnit* InSelectedUnit)
{
    if (!ActionWidget || !InSelectedUnit) 
    {
        UE_LOG(LogTemp, Warning, TEXT("ShowActionWidget: Missing ActionWidget or InSelectedUnit"));
        return;
    } 
    // Update the member variable
    SelectedUnit = InSelectedUnit;

    // Update button states based on unit's available actions
    ActionWidget->UpdateButtons(
        !SelectedUnit->bHasMovedThisTurn,
        !SelectedUnit->bHasAttackedThisTurn
    );

        ActionWidget->SetVisibility(ESlateVisibility::Visible);
    
    
APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
if (PlayerController)
{
    FInputModeGameAndUI InputMode;
    InputMode.SetWidgetToFocus(ActionWidget->TakeWidget());
    PlayerController->SetInputMode(InputMode);
    UE_LOG(LogTemp, Warning, TEXT("Input mode set to GameAndUI"));
}
}
void AMyGameMode::HideActionWidget()
{
    if (ActionWidget)
    {
        ActionWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void AMyGameMode::HandleMoveAction()
{
    if (!SelectedUnit || !GridManager) return;


    // toggle comportamento
    if (bMovementRangeVisible)
    {
        UE_LOG(LogTemp, Warning, TEXT("HideActionWidget: Movement Range"));
        
        GridManager->ClearHighlights();
        //GridManager->HighlightMovementRange(
        //    SelectedUnit->GetGridPosition(),
        //    SelectedUnit->MovementRange,
        //    false
        //);
        bMovementRangeVisible = false;
    }
    else
    {
        GridManager->ClearHighlights();
        GridManager->HighlightMovementRange(
            SelectedUnit->GetGridPosition(),
            SelectedUnit->MovementRange,
            true
        );
        bMovementRangeVisible = true;
    }

    HideActionWidget();
}




void AMyGameMode::HandleAttackAction()
{
    if (!SelectedUnit || !GridManager) return;

    // cancella eventuale highlight movimento
    if (bMovementRangeVisible)
    {
        GridManager->ClearHighlights();
        bMovementRangeVisible = false;
    }

    CurrentActionState = EUnitActionState::Attacking;

    GridManager->HighlightAttackRange(
        SelectedUnit->GetGridPosition(),
        SelectedUnit->AttackRange,
        true
    );

    HideActionWidget();
}



void AMyGameMode::EndPlayerTurn()
{
    ActionWidget->SetVisibility(ESlateVisibility::Collapsed);
    bIsPlayerTurn = false;
    
    // Start AI turn after delay
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
    {
        TurnManager->ExecuteAITurn(this);
    }, 1.0f, false);
}

void AMyGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (TurnManager) TurnManager->Destroy();
    if (UnitActions) UnitActions->Destroy();
    
    // Destroy GridManager if it exists
    if (GridManager)
    {
        GridManager->Destroy();
        GridManager = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("GridManager destroyed!"));
    }

    // Remove and destroy widgets
    if (CoinWidget)
    {
        CoinWidget->RemoveFromParent();
        CoinWidget = nullptr;
        
    }

    if (PlacementWidget)
    {
        PlacementWidget->RemoveFromParent();
        PlacementWidget = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("PlacementWidget removed and destroyed!"));
    }

    // Destroy CoinTossManager if it exists
    if (CoinTossManager)
    {
        CoinTossManager->Destroy();
        CoinTossManager = nullptr;
       
    }

    // Reset state variables
    //PlayerUnitsToPlace.Empty();
    //AIUnitsToPlace.Empty();
    SelectedUnitType = TEXT("");
    bIsPlayerTurn = false;

    UE_LOG(LogTemp, Warning, TEXT("GameMode state reset!"));
}

