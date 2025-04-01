#include "GridCell.h"
#include "GridManager.h"
#include "Unit.h"
#include "MyGameMode.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
AGridCell::AGridCell()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create the Static Mesh Component for the cell
    CellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CellMesh"));
    RootComponent = CellMesh;

    // Assign a default cube mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Plane.Plane"));
    if (MeshAsset.Succeeded())
    {
        CellMesh->SetStaticMesh(MeshAsset.Object);
        CellMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 0.1f)); // Thin tile
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load default mesh for GridCell!"));
    }

    // Assign a default material
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMaterialAsset(TEXT("/Script/Engine.Material'/Game/StarterContent/Materials/M_Water_Ocean.M_Water_Ocean'"));
    if (DefaultMaterialAsset.Succeeded())
    {
        DefaultMaterial = DefaultMaterialAsset.Object;
        CellMesh->SetMaterial(0, DefaultMaterial);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load default material for GridCell!"));
    }

    // Initialize ObstacleMaterial
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> ObstacleMaterialAsset(TEXT("/Script/Engine.Material'/Game/StarterContent/Materials/M_Brick_Clay_New.M_Brick_Clay_New'"));
    if (ObstacleMaterialAsset.Succeeded())
    {
        ObstacleMaterial = ObstacleMaterialAsset.Object;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load obstacle material for GridCell!"));
    }

    // materiale blu per il movimento
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MoveMatFinder(
        TEXT("/Game/StarterContent/Materials/M_Metal_Gold.M_Metal_Gold"));
    HighlightMoveMaterial = MoveMatFinder.Object;

    if (CellMesh)
    {
        CellMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        CellMesh->SetCollisionObjectType(ECC_WorldStatic);
        CellMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
        CellMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
        CellMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
        CellMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    }
    
}

void AGridCell::BeginPlay()
{
    Super::BeginPlay();

    // Enable input for this actor
    EnableInput(GetWorld()->GetFirstPlayerController());
    
    // Get the static mesh component
    UStaticMeshComponent* MeshComponent = FindComponentByClass<UStaticMeshComponent>();
    if (MeshComponent)
    {
        // Setup click event using OnComponentClicked
        MeshComponent->OnClicked.AddDynamic(this, &AGridCell::OnCellClicked);
       
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GridCell %s: No mesh component found!"), *CellName);
    }
}

void AGridCell::OnCellClicked(UPrimitiveComponent* ClickedComponent, FKey ButtonPressed)
{
    UE_LOG(LogTemp, Warning, TEXT("ClickedComponent: %s"), *ClickedComponent->GetName());
    UE_LOG(LogTemp, Warning, TEXT("Component Collision: %d"), ClickedComponent->GetCollisionEnabled() != ECollisionEnabled::NoCollision);

    AMyGameMode* GameMode = Cast<AMyGameMode>(GetWorld()->GetAuthGameMode());
    if (!GameMode)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get GameMode!"));
        return;
    }

    if (GameMode->CurrentGamePhase == EGamePhase::Placement)
    {
        GameMode->HandleUnitPlacement(FVector2D(GridPositionX, GridPositionY));
    }
    else if (GameMode->CurrentGamePhase == EGamePhase::UnitAction)
    {
        AGridManager* GridManager = GameMode->GridManager;
        if (GridManager)
        {
            GridManager->HandleCellClick(this);
        }
    }
}


// Set obstacle status
void AGridCell::SetObstacle(bool bObstacle)
{
    if (bIsObstacle == bObstacle)
    {
        return; // Avoid reapplying material unnecessarily
    }
    bIsObstacle = bObstacle;

    if (!ObstacleMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("Obstacle material is NULL! Cannot apply to cell: %s"), *GetActorLabel());
        return;
    }

    if (bIsObstacle)
    {
        // Create a dynamic material instance for variation
        UMaterialInstanceDynamic* DynamicMat = UMaterialInstanceDynamic::Create(ObstacleMaterial, this);
        if (DynamicMat)
        {
            float RandomShade = FMath::RandRange(0.3f, 1.0f);
            DynamicMat->SetVectorParameterValue(FName("ColorTint"), FLinearColor(RandomShade, RandomShade, RandomShade, 1.0f));
            CellMesh->SetMaterial(0, DynamicMat);
            CellMesh->SetWorldScale3D(FVector(1.2f, 1.2f, 2.0f)); // Scale up for obstacles
            CellMesh->MarkRenderStateDirty();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create Dynamic Material for: %s"), *GetActorLabel());
        }
    }
    else
    {
        // Revert to the default material
        if (DefaultMaterial)
        {
            CellMesh->SetMaterial(0, DefaultMaterial);
            CellMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 0.1f)); // Reset scale
            CellMesh->MarkRenderStateDirty();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to load default material for GridCell!"));
        }
    }
}

// Check if this cell is an obstacle
bool AGridCell::IsObstacle() const
{
    return bIsObstacle;
}

// Set occupied status
void AGridCell::SetOccupied(bool bOccupied)
{
    if (bIsOccupied && bOccupied)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot occupy obstacle cell!"));
        return;
    }
    bIsOccupied = bOccupied;
}

// Check if this cell is occupied
bool AGridCell::IsOccupied() const
{
    return bIsOccupied;
}


// Set grid position
void AGridCell::SetGridPosition(int32 X, int32 Y)
{
    GridPositionX = X;
    GridPositionY = Y;
}

// Set cell name
void AGridCell::SetCellName(const FString& Name)
{
    CellName = Name;
}

// Get grid position X
int32 AGridCell::GetGridPositionX() const
{
    return GridPositionX;
}

// Get grid position Y
int32 AGridCell::GetGridPositionY() const
{
    return GridPositionY;
}


// In GridCell.cpp
void AGridCell::SetUnit(AUnit* Unit)
{
    // Use existing bIsOccupied flag
    bIsOccupied = (Unit != nullptr);
    
    // Log the change
    UE_LOG(LogTemp, Warning, TEXT("Cell %s at (%d,%d) - Occupation set to: %s"), 
        *CellName, GridPositionX, GridPositionY, 
        bIsOccupied ? TEXT("Occupied") : TEXT("Empty"));
}

AUnit* AGridCell::GetUnit() const
{
    // Use the existing GetOverlappingActors method from your code
    TArray<AActor*> OverlappingActors;
    GetOverlappingActors(OverlappingActors, AUnit::StaticClass());
    
    if (OverlappingActors.Num() > 0)
    {
        AUnit* FoundUnit = Cast<AUnit>(OverlappingActors[0]);
        if (FoundUnit)
        {
            UE_LOG(LogTemp, Log, TEXT("Cell %s at (%d,%d) - Found unit: %s"), 
                *CellName, GridPositionX, GridPositionY, *FoundUnit->GetName());
            return FoundUnit;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Cell %s at (%d,%d) - No unit found"), 
        *CellName, GridPositionX, GridPositionY);
    return nullptr;
}

void AGridCell::SetHighlight(bool bHighlight)
{
    if (IsObstacle()) return;

    if (CellMesh && DefaultMaterial && HighlightMoveMaterial)
    {
        CellMesh->SetMaterial(0, bHighlight ? DefaultMaterial : HighlightMoveMaterial);
        
    }

    
}

void AGridCell::SetHighlightColor(FLinearColor NewColor)
{
    if (!CellMesh) return;
    
    UMaterialInstanceDynamic* DynMat = CellMesh->CreateAndSetMaterialInstanceDynamic(0);
    if (DynMat)
    {
        DynMat->SetVectorParameterValue("Color", NewColor);
    }
}

