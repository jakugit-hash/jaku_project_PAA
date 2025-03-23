#include "MyGameMode.h"
#include "GridManager.h"
#include "PlacementWidget.h"
#include "Sniper.h"
#include "Brawler.h"
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
            APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
            if (PlayerController)
            {
                PlayerController->SetInputMode(FInputModeUIOnly());
                PlayerController->bShowMouseCursor = true;
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

void AMyGameMode::HandleUnitPlacement(FVector2D CellPosition)
{
    UE_LOG(LogTemp, Warning, TEXT("Attempting to place unit at (%f, %f)"), CellPosition.X, CellPosition.Y);

    if (IsCellValidForPlacement(CellPosition))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cell is valid for placement."));

        // Place the selected unit
        PlaceUnit(SelectedUnitType, CellPosition);

        // Remove the placed unit from the list
        if (bIsPlayerTurn)
        {
            PlayerUnitsToPlace.Remove(SelectedUnitType);
            UE_LOG(LogTemp, Warning, TEXT("Player placed a %s. Remaining units: %d"), *SelectedUnitType, PlayerUnitsToPlace.Num());
        }
        else
        {
            AIUnitsToPlace.Remove(SelectedUnitType);
            UE_LOG(LogTemp, Warning, TEXT("AI placed a %s. Remaining units: %d"), *SelectedUnitType, AIUnitsToPlace.Num());
        }

        // Check if all units have been placed
        if (PlayerUnitsToPlace.Num() == 0 && AIUnitsToPlace.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("All units placed. Starting the game."));
            StartPlayerTurn();
        }
        else
        {
            // Switch turns
            bIsPlayerTurn = !bIsPlayerTurn;

            if (bIsPlayerTurn)
            {
                UE_LOG(LogTemp, Warning, TEXT("Player's turn to place a unit."));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("AI's turn to place a unit."));
                HandleAIPlacement();
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid cell for placement."));
    }
}

void AMyGameMode::HandleAIPlacement()
{
    // AI selects a random unit to place
    if (AIUnitsToPlace.Num() > 0)
    {
        SelectedUnitType = AIUnitsToPlace[FMath::RandRange(0, AIUnitsToPlace.Num() - 1)];

        // Find a random valid cell for placement
        int32 X, Y;
        if (GridManager->FindRandomEmptyCell(X, Y))
        {
            HandleUnitPlacement(FVector2D(X, Y));
        }
    }
}

void AMyGameMode::PlaceUnit(FString UnitType, FVector2D CellPosition)
{
    FVector WorldPosition = GridManager->GetCellWorldPosition(CellPosition.X, CellPosition.Y);
    AUnit* NewUnit = nullptr;

    UE_LOG(LogTemp, Warning, TEXT("Attempting to spawn %s at (%f, %f)"), *UnitType, CellPosition.X, CellPosition.Y);

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
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn %s at (%f, %f)"), *UnitType, CellPosition.X, CellPosition.Y);
    }
}

bool AMyGameMode::IsCellValidForPlacement(FVector2D CellPosition)
{
    AGridCell* Cell = GridManager->GetCellAtPosition(CellPosition);
    return Cell && !Cell->IsObstacle() && !Cell->IsOccupied();
}



void AMyGameMode::HandleCellClick(FVector2D CellPosition)
{
    if (!GridManager)
    {
        UE_LOG(LogTemp, Error, TEXT("GridManager is null!"));
        return;
    }
    // Switch turns
    bIsPlayerTurn = !bIsPlayerTurn;

    if (bIsPlayerTurn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player's turn to place a unit."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AI's turn to place a unit."));
        HandleAIPlacement();
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
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid cell for placement."));
    }
}

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
                UE_LOG(LogTemp, Warning, TEXT("PlacementWidget created and displayed!"));
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
        UE_LOG(LogTemp, Warning, TEXT("CoinWidget removed and destroyed!"));
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
        UE_LOG(LogTemp, Warning, TEXT("CoinTossManager destroyed!"));
    }

    // Reset state variables
    PlayerUnitsToPlace.Empty();
    AIUnitsToPlace.Empty();
    SelectedUnitType = TEXT("");
    bIsPlayerTurn = false;

    UE_LOG(LogTemp, Warning, TEXT("GameMode state reset!"));
}