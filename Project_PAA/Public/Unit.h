#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GlobalEnums.h"
#include "Unit.generated.h"

class AGridCell;
class AGridManager;

UCLASS()
class PROJECT_PAA_API AUnit : public AActor
{
	GENERATED_BODY()

public:
	AUnit();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	int32 Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	int32 MovementRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	int32 AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	int32 MinDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	int32 MaxDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HP = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit")
	UStaticMeshComponent* UnitMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasMovedThisTurn = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsPlayerUnit = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasAttackedThisTurn = false;

	UPROPERTY(VisibleAnywhere)
	bool bIsSelected = false;


	UFUNCTION(BlueprintCallable)
	void SetSelected(bool bSelected);

	UFUNCTION(BlueprintCallable)
	void SetGridPosition(FVector2D NewPosition);

	UFUNCTION(BlueprintCallable)
	FVector2D GetGridPosition() const;

	UFUNCTION(BlueprintCallable, Category = "Unit")
	virtual void SetAsPlayerUnit(bool bIsPlayer);

	UFUNCTION(BlueprintCallable, Category = "Unit")
	bool CanAttack() const;

	void MoveToCell(FVector2D NewPosition);
	void DestroyUnit();

	UFUNCTION()
	void OnClicked(UPrimitiveComponent* ClickedComp, FKey ButtonPressed);

	UFUNCTION(BlueprintCallable)
   virtual bool IsSniper() const { return false; }
	
protected:
	UFUNCTION(BlueprintCallable, Category = "Unit")
	void ApplyTeamMaterials(bool bIsPlayer);


	

private:
	FVector2D GridPosition;
	AGridManager* GetGridManager() const;
};
