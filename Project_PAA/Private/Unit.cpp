

#include "Unit.h"
#include "GridManager.h" 
#include "GridCell.h"    
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AUnit::AUnit()
{
    PrimaryActorTick.bCanEverTick = false;
    UnitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UnitMesh"));
    RootComponent = UnitMesh;

    // Assign a default cube mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (MeshAsset.Succeeded())
    {
        UnitMesh->SetStaticMesh(MeshAsset.Object);
    }

    // Assign a default material
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialAsset(TEXT("/Script/Engine.Material'/Game/StarterContent/Materials/M_Metal_Brushed_Nickel.M_Metal_Brushed_Nickel'"));
    if (MaterialAsset.Succeeded())
    {
        UnitMesh->SetMaterial(0, MaterialAsset.Object);
    }
}

// Called when the game starts or when spawned
void AUnit::BeginPlay()
{
    Super::BeginPlay();
}

// Get the GridManager instance
AGridManager* AUnit::GetGridManager() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;
    
    // Find the GridManager in the level
    AGridManager* GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
    if (!GridManager)
    {
        UE_LOG(LogTemp, Error, TEXT("GridManager not found in the level!"));
    }
    return GridManager;
}

// Set the unit's grid position
void AUnit::SetGridPosition(FVector2D NewPosition)
{
    AGridManager* GridManager = GetGridManager();
    if (!GridManager)
    {
        UE_LOG(LogTemp, Error, TEXT("GridManager is not found!"));
        return;
    }

    // Mark the current cell as unoccupied
    AGridCell* CurrentCell = GridManager->GetCellAtPosition(GridPosition);
    if (CurrentCell)
    {
        CurrentCell->SetOccupied(false);
    }

    // Update the grid position
    GridPosition = NewPosition;

    // Mark the new cell as occupied
    AGridCell* NewCell = GridManager->GetCellAtPosition(NewPosition);
    if (NewCell)
    {
        NewCell->SetOccupied(true);
    }

    // Update the unit's world position
    FVector WorldPosition = GridManager->GetCellWorldPosition(NewPosition.X, NewPosition.Y);
    SetActorLocation(WorldPosition);
}

// Move the unit to a new cell
void AUnit::MoveToCell(FVector2D NewPosition)
{
    AGridManager* GridManager = GetGridManager();
    if (!GridManager)
    {
        UE_LOG(LogTemp, Error, TEXT("GridManager is not found!"));
        return;
    }

    // Check if the new position is within movement range
    int32 Distance = FMath::Abs(NewPosition.X - GridPosition.X) + FMath::Abs(NewPosition.Y - GridPosition.Y);
    if (Distance > MovementRange)
    {
        UE_LOG(LogTemp, Warning, TEXT("Target cell is out of movement range!"));
        return;
    }

    // Check if the target cell is free
    if (GridManager->IsCellFree(NewPosition))
    {
        // Move the unit to the new position
        SetGridPosition(NewPosition);
        UE_LOG(LogTemp, Warning, TEXT("Unit moved to (%.2f, %.2f)"), NewPosition.X, NewPosition.Y);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Target cell is occupied or blocked!"));
    }
}