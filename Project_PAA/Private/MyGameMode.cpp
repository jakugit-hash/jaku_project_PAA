#include "MyGameMode.h"
#include "GridManager.h"
#include "PlacementWidget.h"
#include "Sniper.h"
#include "Brawler.h"
#include "Kismet/GameplayStatics.h"

AMyGameMode::AMyGameMode()
{
    // Set default values
    bIsPlayerTurn = false;

    // Initialize pointers to nullptr
    GridManager = nullptr;
    PlacementWidget = nullptr;

    // Assign the PlacementWidgetClass in the constructor
    static ConstructorHelpers::FClassFinder<UUserWidget> PlacementWidgetBP(TEXT("/Game/widgets/WBP_PlacementWidget.WBP_PlacementWidget'"));
    if (PlacementWidgetBP.Succeeded())
    {
        PlacementWidgetClass = PlacementWidgetBP.Class;
        UE_LOG(LogTemp, Warning, TEXT("PlacementWidgetClass assigned successfully!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find PlacementWidgetClass!"));
    }
}

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Find or spawn the GridManager
    GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
    if (!GridManager)
    {
        UE_LOG(LogTemp, Error, TEXT("GridManager not found in the level! Spawning a new one..."));
        FActorSpawnParameters SpawnParams;
        GridManager = GetWorld()->SpawnActor<AGridManager>(AGridManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    }

    if (!GridManager)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn GridManager!"));
        return;
    }
}

void AMyGameMode::HandleCoinTossResult(bool bIsPlayerTurnResult)
{
    // Set who starts the placement phase
    bIsPlayerTurn = bIsPlayerTurnResult;

    // Start the placement phase
    StartPlacementPhase();
}

void AMyGameMode::StartPlacementPhase()
{
    // Initialize units to place
    PlayerUnitsToPlace = { TEXT("Sniper"), TEXT("Brawler") };
    AIUnitsToPlace = { TEXT("Sniper"), TEXT("Brawler") };

    // Create and display the PlacementWidget
    if (PlacementWidgetClass)
    {
        PlacementWidget = CreateWidget<UPlacementWidget>(GetWorld(), PlacementWidgetClass);
        if (PlacementWidget)
        {
            PlacementWidget->SetGameMode(this);
            PlacementWidget->AddToViewport();
        }
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
    if (IsCellValidForPlacement(CellPosition))
    {
        // Place the selected unit
        PlaceUnit(SelectedUnitType, CellPosition);

        // Remove the placed unit from the list
        if (bIsPlayerTurn)
        {
            PlayerUnitsToPlace.Remove(SelectedUnitType);
        }
        else
        {
            AIUnitsToPlace.Remove(SelectedUnitType);
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
        UE_LOG(LogTemp, Warning, TEXT("%s placed at (%d, %d)"), *UnitType, CellPosition.X, CellPosition.Y);
    }
}

bool AMyGameMode::IsCellValidForPlacement(FVector2D CellPosition)
{
    AGridCell* Cell = GridManager->GetCellAtPosition(CellPosition);
    return Cell && !Cell->IsObstacle() && !Cell->IsOccupied();
}

void AMyGameMode::StartPlayerTurn()
{
    UE_LOG(LogTemp, Warning, TEXT("Player's Turn!"));
    // Implement player turn logic here
}