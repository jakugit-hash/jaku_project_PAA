#include "GridCell.h"
#include "GridManager.h"
#include "Unit.h"
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
   // UE_LOG(LogTemp, Warning, TEXT("GridCell %s: Click detected with button %s"), *CellName, *ButtonPressed.ToString());

    if (ClickedComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("ClickedComponent: %s"), *ClickedComponent->GetName());
        UE_LOG(LogTemp, Warning, TEXT("Component Collision: %d"), 
            ClickedComponent->GetCollisionEnabled() != ECollisionEnabled::NoCollision);
    }

    // Notify GameMode
    if (AMyGameMode* GameMode = Cast<AMyGameMode>(GetWorld()->GetAuthGameMode()))
    {
       //UE_LOG(LogTemp, Warning, TEXT("Notifying GameMode about click at (%d, %d)"), GridPositionX, GridPositionY);
        GameMode->HandleUnitPlacement(FVector2D(GridPositionX, GridPositionY));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get GameMode!"));
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

AUnit* AGridCell::GetUnit() const
{
    TArray<AActor*> OverlappingActors;
    GetOverlappingActors(OverlappingActors, AUnit::StaticClass());
    return (OverlappingActors.Num() > 0) ? Cast<AUnit>(OverlappingActors[0]) : nullptr;
}

void AGridCell::SetHighlight(bool bHighlight)
{
    if (!CellMesh) return;

    // Create or get dynamic material instance
    UMaterialInstanceDynamic* DynMaterial = CellMesh->CreateAndSetMaterialInstanceDynamic(0);
    if (DynMaterial)
    {
        // Set highlight color (yellow for highlight, white for normal)
        FLinearColor HighlightColor = bHighlight ? FLinearColor(1.0f, 1.0f, 0.0f, 1.0f) : FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
        DynMaterial->SetVectorParameterValue("Color", HighlightColor);
        
        // Optional: Add emissive glow when highlighted
        float EmissiveStrength = bHighlight ? 5.0f : 0.0f;
        DynMaterial->SetScalarParameterValue("EmissiveStrength", EmissiveStrength);
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
