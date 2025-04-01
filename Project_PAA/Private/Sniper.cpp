#include "Sniper.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"

ASniper::ASniper()
{
	Health = 20;
	MovementRange = 3;
	AttackRange = 10;
	MinDamage = 4;
	MaxDamage = 8;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlayerMat(TEXT("/Game/Blueprints/HP_Sniper.HP_Sniper"));
	if (PlayerMat.Succeeded()) PlayerMaterial = PlayerMat.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> AIMat(TEXT("/Game/Blueprints/HP_Sniper2.HP_Sniper2"));
	if (AIMat.Succeeded()) AIMaterial = AIMat.Object;
}

void ASniper::SetAsPlayerUnit(bool bIsPlayer)
{
	Super::SetAsPlayerUnit(bIsPlayer);
}

void ASniper::BeginPlay()
{
	Super::BeginPlay();
}
