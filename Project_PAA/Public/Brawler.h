#pragma once

#include "CoreMinimal.h"
#include "Unit.h"
#include "Brawler.generated.h"

UCLASS()
class PROJECT_PAA_API ABrawler : public AUnit
{
	GENERATED_BODY()

public:
	ABrawler();

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* PlayerMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* AIMaterial;

	virtual void SetAsPlayerUnit(bool bIsPlayer) override;

protected:
	virtual void BeginPlay() override;
};
