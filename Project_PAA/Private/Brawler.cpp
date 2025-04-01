#include "Brawler.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"

ABrawler::ABrawler()
{
	Health = 40;
	MovementRange = 6;
	AttackRange = 1;
	MinDamage = 1;
	MaxDamage = 6;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlayerMat(TEXT("/Game/Blueprints/HP_Brawler.HP_Brawler"));
	if (PlayerMat.Succeeded()) PlayerMaterial = PlayerMat.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> AIMat(TEXT("/Game/Blueprints/HP_Brawler2.HP_Brawler2"));
	if (AIMat.Succeeded()) AIMaterial = AIMat.Object;
}

void ABrawler::SetAsPlayerUnit(bool bIsPlayer)
{
	Super::SetAsPlayerUnit(bIsPlayer);
}

void ABrawler::BeginPlay()
{
	Super::BeginPlay();
}
