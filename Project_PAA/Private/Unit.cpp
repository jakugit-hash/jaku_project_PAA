

#include "Unit.h"
#include "GridManager.h" 
#include "GridCell.h"
#include "MyGameMode.h"
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
    UnitMesh->OnClicked.AddDynamic(this, &AUnit::OnClicked);
    if (UnitMesh)
    {
        UnitMesh->OnClicked.RemoveDynamic(this, &AUnit::OnClicked); // evita doppio bind
        UnitMesh->OnClicked.AddDynamic(this, &AUnit::OnClicked);
        UE_LOG(LogTemp, Warning, TEXT("Unit OnClicked bind effettuato con successo"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UnitMesh is null in BeginPlay!"));
    }
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

void AUnit::SetGridPosition(FVector2D NewPosition)
{
    AGridManager* GridManager = GetGridManager();
    if (!GridManager ) return;

            // Mark current cell as unoccupied
            if (AGridCell* CurrentCell = GridManager->GetCellAtPosition(GridPosition))
            {
                CurrentCell->SetOccupied(false);
            }

            // Update position
            GridPosition = NewPosition;

            // Mark new cell as occupied
            if (AGridCell* NewCell = GridManager->GetCellAtPosition(NewPosition))
            {
                NewCell->SetOccupied(true);
                SetActorLocation(GridManager->GetCellWorldPosition(NewPosition.X, NewPosition.Y));
            }
        }

FVector2D AUnit::GetGridPosition() const
{ return GridPosition; }


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

void AUnit::SetSelected(bool bSelected)
{
    bIsSelected = bSelected;
    
    // Visual feedback
    if (UnitMesh)
    {
        UnitMesh->SetRenderCustomDepth(bSelected);
        UnitMesh->SetCustomDepthStencilValue(bSelected ? 2 : 0);
        
        // Pulse effect when selected
        if (bSelected)
        {
            UnitMesh->SetScalarParameterValueOnMaterials("Pulse", 1.0f);
        }
    }
}

void AUnit::OnClicked(UPrimitiveComponent* ClickedComp, FKey ButtonPressed)
{
    UE_LOG(LogTemp, Error, TEXT("suca tommaso"));
    if (AMyGameMode* GM = Cast<AMyGameMode>(GetWorld()->GetAuthGameMode()))
    {
        GM->HandleUnitSelection(this);
    }
}


void AUnit::DestroyUnit()
{
    // Notify grid cell
    if (AGridManager* GridManager = GetGridManager())
    {
        if (AGridCell* Cell = GridManager->GetCellAtPosition(GridPosition))
        {
            Cell->SetOccupied(false);
        }
    }

    // Remove from GameMode's unit arrays
    if (AMyGameMode* GameMode = Cast<AMyGameMode>(GetWorld()->GetAuthGameMode()))
    {
        if (bIsPlayerUnit)
        {
            GameMode->PlayerUnits.Remove(this);
        }
        else
        {
            GameMode->AIUnits.Remove(this);
        }
    }

    // Destroy actor
    Destroy();
}
