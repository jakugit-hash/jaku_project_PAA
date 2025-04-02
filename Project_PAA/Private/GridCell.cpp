#include "GridCell.h"
#include "GridManager.h"
#include "Unit.h"
#include "MyGameMode.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"
#include "Materials/MaterialInstanceDynamic.h"

AGridCell::AGridCell()
{
    PrimaryActorTick.bCanEverTick = false;

    CellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CellMesh"));
    RootComponent = CellMesh;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Plane.Plane"));
    if (MeshAsset.Succeeded())
    {
        CellMesh->SetStaticMesh(MeshAsset.Object);
        CellMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 0.1f));
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMaterialAsset(TEXT("/Script/Engine.Material'/Game/StarterContent/Materials/M_Water_Ocean.M_Water_Ocean'"));
    if (DefaultMaterialAsset.Succeeded())
    {
        DefaultMaterial = DefaultMaterialAsset.Object;
        CellMesh->SetMaterial(0, DefaultMaterial);
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> ObstacleMaterialAsset(TEXT("/Script/Engine.Material'/Game/StarterContent/Materials/M_Brick_Clay_New.M_Brick_Clay_New'"));
    if (ObstacleMaterialAsset.Succeeded())
    {
        ObstacleMaterial = ObstacleMaterialAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MoveMatFinder(TEXT("/Game/StarterContent/Materials/M_Metal_Gold.M_Metal_Gold"));
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

    EnableInput(GetWorld()->GetFirstPlayerController());

    if (UStaticMeshComponent* MeshComponent = FindComponentByClass<UStaticMeshComponent>())
    {
        MeshComponent->OnClicked.AddDynamic(this, &AGridCell::OnCellClicked);
    }
}

void AGridCell::OnCellClicked(UPrimitiveComponent* ClickedComponent, FKey ButtonPressed)
{
    AMyGameMode* GameMode = Cast<AMyGameMode>(GetWorld()->GetAuthGameMode());
    if (!GameMode) return;

    if (GameMode->CurrentGamePhase == EGamePhase::Placement)
    {
        GameMode->HandleUnitPlacement(FVector2D(GridPositionX, GridPositionY));
    }
    else if (GameMode->CurrentGamePhase == EGamePhase::UnitAction)
    {
        if (GameMode->GridManager)
        {
            GameMode->GridManager->HandleCellClick(this);
        }
    }
}

void AGridCell::SetObstacle(bool bObstacle)
{
    if (bIsObstacle == bObstacle) return;

    bIsObstacle = bObstacle;

    if (bIsObstacle && ObstacleMaterial)
    {
        UMaterialInstanceDynamic* DynamicMat = UMaterialInstanceDynamic::Create(ObstacleMaterial, this);
        if (DynamicMat)
        {
            float Shade = FMath::RandRange(0.3f, 1.0f);
            DynamicMat->SetVectorParameterValue(FName("ColorTint"), FLinearColor(Shade, Shade, Shade, 1.0f));
            CellMesh->SetMaterial(0, DynamicMat);
            CellMesh->SetWorldScale3D(FVector(1.2f, 1.2f, 2.0f));
            CellMesh->MarkRenderStateDirty();
        }
    }
    else if (DefaultMaterial)
    {
        CellMesh->SetMaterial(0, DefaultMaterial);
        CellMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 0.1f));
        CellMesh->MarkRenderStateDirty();
    }
}

bool AGridCell::IsObstacle() const
{
    return bIsObstacle;
}

void AGridCell::SetOccupied(bool bOccupied)
{
    bIsOccupied = bOccupied;
}

bool AGridCell::IsOccupied() const
{
    return bIsOccupied;
}

void AGridCell::SetGridPosition(int32 X, int32 Y)
{
    GridPositionX = X;
    GridPositionY = Y;
}

void AGridCell::SetCellName(const FString& Name)
{
    CellName = Name;
}

int32 AGridCell::GetGridPositionX() const { return GridPositionX; }
int32 AGridCell::GetGridPositionY() const { return GridPositionY; }

void AGridCell::SetUnit(AUnit* Unit)
{
    bIsOccupied = (Unit != nullptr);
    UE_LOG(LogTemp, Warning, TEXT("Cell %s at (%d,%d) - Occupation set to: %s"), 
        *CellName, GridPositionX, GridPositionY, 
        bIsOccupied ? TEXT("Occupied") : TEXT("Empty"));
}

AUnit* AGridCell::GetUnit() const
{
    TArray<AActor*> OverlappingActors;
    GetOverlappingActors(OverlappingActors, AUnit::StaticClass());

    if (OverlappingActors.Num() > 0)
    {
        if (AUnit* FoundUnit = Cast<AUnit>(OverlappingActors[0]))
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
