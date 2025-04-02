#pragma once

#include "CoreMinimal.h"
#include "Unit.h"
#include "Sniper.generated.h"

UCLASS()
class PROJECT_PAA_API ASniper : public AUnit
{
	GENERATED_BODY()
    
public:
	ASniper();

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* PlayerMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* AIMaterial;

	virtual void SetAsPlayerUnit(bool bIsPlayer) override;
	virtual bool IsSniper() const override { return true; }


protected:
	virtual void BeginPlay() override;
};