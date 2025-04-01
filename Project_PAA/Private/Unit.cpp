#include "Unit.h"
#include "GridManager.h"
#include "GridCell.h"
#include "MyGameMode.h"
#include "Sniper.h"
#include "Brawler.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"

AUnit::AUnit()
{
	PrimaryActorTick.bCanEverTick = false;
	UnitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UnitMesh"));
	RootComponent = UnitMesh;
	
	// Impostazioni di collisione (queste servono comunque!)
	UnitMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	UnitMesh->SetCollisionObjectType(ECC_Pawn);
	UnitMesh->SetCollisionResponseToAllChannels(ECR_Block);
	UnitMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (MeshAsset.Succeeded())
	{
		UnitMesh->SetStaticMesh(MeshAsset.Object);
	}

}

void AUnit::BeginPlay()
{
	Super::BeginPlay();
	if (UnitMesh)
	{
		UnitMesh->OnClicked.AddDynamic(this, &AUnit::OnClicked);
	}
}

void AUnit::SetSelected(bool bSelected)
{
	bIsSelected = bSelected;
}

FVector2D AUnit::GetGridPosition() const
{
	return GridPosition;
}

void AUnit::SetGridPosition(FVector2D NewPosition)
{
	AGridManager* GridManager = GetGridManager();
	if (!GridManager) return;

	if (AGridCell* CurrentCell = GridManager->GetCellAtPosition(GridPosition))
	{
		if (CurrentCell->GetUnit() == this)
		{
			CurrentCell->SetUnit(nullptr);
		}
	}

	GridPosition = NewPosition;

	if (AGridCell* NewCell = GridManager->GetCellAtPosition(NewPosition))
	{
		NewCell->SetUnit(this);
		SetActorLocation(GridManager->GetCellWorldPosition(NewPosition.X, NewPosition.Y));
	}
}

void AUnit::MoveToCell(FVector2D NewPosition)
{
	AGridManager* GridManager = GetGridManager();
	if (!GridManager) return;

	if (AGridCell* CurrentCell = GridManager->GetCellAtPosition(GridPosition))
	{
		CurrentCell->SetOccupied(false);
	}

	GridPosition = NewPosition;

	if (AGridCell* NewCell = GridManager->GetCellAtPosition(NewPosition))
	{
		NewCell->SetOccupied(true);
		SetActorLocation(GridManager->GetCellWorldPosition(NewPosition.X, NewPosition.Y));
	}
}

bool AUnit::CanAttack() const
{
	return Health > 0 && !bHasAttackedThisTurn;
}

AGridManager* AUnit::GetGridManager() const
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	return Cast<AGridManager>(UGameplayStatics::GetActorOfClass(World, AGridManager::StaticClass()));
}

void AUnit::ApplyTeamMaterials(bool bIsPlayer)
{
	if (!UnitMesh) return;

	if (ABrawler* Brawler = Cast<ABrawler>(this))
	{
		if (Brawler->PlayerMaterial && Brawler->AIMaterial)
		{
			UnitMesh->SetMaterial(0, bIsPlayer ? Brawler->PlayerMaterial : Brawler->AIMaterial);
		}
	}
	else if (ASniper* SniperUnit = Cast<ASniper>(this))
	{
		if (SniperUnit->PlayerMaterial && SniperUnit->AIMaterial)
		{
			UnitMesh->SetMaterial(0, bIsPlayer ? SniperUnit->PlayerMaterial : SniperUnit->AIMaterial);
		}
	}
}


void AUnit::SetAsPlayerUnit(bool bIsPlayer)
{
	bIsPlayerUnit = bIsPlayer;
	ApplyTeamMaterials(bIsPlayer);
}

void AUnit::OnClicked(UPrimitiveComponent* ClickedComp, FKey ButtonPressed)
{
	if (AMyGameMode* GM = Cast<AMyGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->HandleUnitSelection(this);
	}
}

void AUnit::DestroyUnit()
{
	if (AGridManager* GridManager = GetGridManager())
	{
		if (AGridCell* Cell = GridManager->GetCellAtPosition(GridPosition))
		{
			if (Cell->GetUnit() == this)
			{
				Cell->SetUnit(nullptr);
			}
		}
	}

	if (AMyGameMode* GameMode = Cast<AMyGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (bIsPlayerUnit)
			GameMode->PlayerUnits.Remove(this);
		else
			GameMode->AIUnits.Remove(this);
	}

	Destroy();
}
