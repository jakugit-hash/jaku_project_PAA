#include "MyGameMode.h"
#include "GridManager.h"
#include "Components/Button.h"
#include "PlacementWidget.h"
#include "Sniper.h"
#include "Brawler.h"
#include "Unit.h"
#include "CoinWidget.h"
#include "CoinTossManager.h"
#include "Kismet/GameplayStatics.h"

AMyGameMode::AMyGameMode()
{
    // Set default values
    bIsPlayerTurn = false;

    // Initialize pointers to nullptr
    GridManager = nullptr;
    PlacementWidget = nullptr;
    CoinTossManager = nullptr;

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
}

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();

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

    // Original turn logic below
    if (PlayerUnitsToPlace.Num() == 0 && AIUnitsToPlace.Num() == 0)
    {
        StartPlayerTurn();
        return;
    }

    bIsPlayerTurn = !bIsPlayerTurn;
    
    if (!bIsPlayerTurn && AIUnitsToPlace.Num() > 0)
    {
        HandleAIPlacement();
    }
}


void AMyGameMode::HandleAIPlacement()
{
    if (AIUnitsToPlace.Num() == 0) return;

    // Find random empty cell
    int32 X, Y;
    if (GridManager->FindRandomEmptyCell(X, Y))
    {
        // Use the first available AI unit
        FString UnitToPlace = AIUnitsToPlace[0];
        HandleUnitPlacement(FVector2D(X, Y));
    }
}

bool AMyGameMode::PlaceUnit(const FString& UnitType, const FVector2D& CellPosition)
{
    FVector WorldPosition = GridManager->GetCellWorldPosition(CellPosition.X, CellPosition.Y);
    AUnit* NewUnit = nullptr;

   

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
        UE_LOG(LogTemp, Warning, TEXT("%s placed at (%f, %f)"), *UnitType, CellPosition.X, CellPosition.Y);
        return true;
    }
    else
    {
        return false;
        //UE_LOG(LogTemp, Error, TEXT("Failed to spawn %s at (%f, %f)"), *UnitType, CellPosition.X, CellPosition.Y);
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


/*void AMyGameMode::HandleCellClick(FVector2D CellPosition)
{
    if (!GridManager)
    {
        UE_LOG(LogTemp, Error, TEXT("GridManager is null!"));
        return;
    }

    // Check if it's the player's turn to place units
    if (!bIsPlayerTurn)
    {
        UE_LOG(LogTemp, Warning, TEXT("It's not the player's turn to place units."));
        return;
    }

    // Check if the selected cell is valid for placement
    AGridCell* Cell = GridManager->GetCellAtPosition(CellPosition);
    if (Cell && !Cell->IsObstacle() && !Cell->IsOccupied())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cell is valid for placement."));

        // Place the selected unit
        PlaceUnit(SelectedUnitType, CellPosition);

        // Mark the cell as occupied
        Cell->SetOccupied(true);

        // Log the placement
        FString CellName = GridManager->GetCellName(CellPosition.X, CellPosition.Y);
        UE_LOG(LogTemp, Warning, TEXT("%s placed at position %s"), *SelectedUnitType, *CellName);

        // Remove the placed unit from the list
        PlayerUnitsToPlace.Remove(SelectedUnitType);
        UE_LOG(LogTemp, Warning, TEXT("Player placed a %s. Remaining units: %d"), *SelectedUnitType, PlayerUnitsToPlace.Num());

        // Check if all units have been placed
        if (PlayerUnitsToPlace.Num() == 0 && AIUnitsToPlace.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("All units placed. Starting the game."));
            StartPlayerTurn();
        }
        else
        {
            // Switch turns
            bIsPlayerTurn = false;
            HandleAIPlacement();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid cell for placement."));
    }
}*/

void AMyGameMode::StartPlayerTurn()
{
    UE_LOG(LogTemp, Warning, TEXT("Player's Turn!"));

    // Check if the player has units left to place
    if (PlayerUnitsToPlace.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player has units to place. Waiting for selection..."));

        // Enable input for unit selection
        APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
        if (PlayerController)
        {
            PlayerController->SetInputMode(FInputModeGameAndUI());
            PlayerController->bShowMouseCursor = true;
            UE_LOG(LogTemp, Warning, TEXT("Player input enabled!"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("PlayerController is null!"));
        }

        // Display the PlacementWidget for unit selection
        if (PlacementWidgetClass)
        {
            PlacementWidget = CreateWidget<UPlacementWidget>(GetWorld(), PlacementWidgetClass);
            if (PlacementWidget)
            {
                PlacementWidget->SetGameMode(this);
                PlacementWidget->AddToViewport();
                
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
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Player has no units left to place. Switching to AI turn."));
        bIsPlayerTurn = false;
        HandleAIPlacement();
    }

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        PC->bShowMouseCursor = true;
        PC->bEnableClickEvents = true;
        PC->bEnableMouseOverEvents = true;
    }
}


void AMyGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

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
    PlayerUnitsToPlace.Empty();
    AIUnitsToPlace.Empty();
    SelectedUnitType = TEXT("");
    bIsPlayerTurn = false;

    UE_LOG(LogTemp, Warning, TEXT("GameMode state reset!"));
}