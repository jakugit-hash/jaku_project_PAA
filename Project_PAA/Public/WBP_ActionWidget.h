#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WBP_ActionWidget.generated.h"

class AMyGameMode;

UCLASS()
class PROJECT_PAA_API UWBP_ActionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Setup(AMyGameMode* GameMode);


	UFUNCTION()
	void UpdateButtons(bool bCanMove, bool bCanAttack);
	void NativeConstruct();
	void SetVisibility(ESlateVisibility InVisibility);

	UPROPERTY(meta = (BindWidget))
	class UButton* MoveButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* AttackButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* EndTurnButton;

private:
	
	UFUNCTION()
	void OnMoveClicked();

	UFUNCTION()
	void OnAttackClicked();

	UFUNCTION()
	void OnEndTurnClicked();
	
	UPROPERTY()
	AMyGameMode* GameModeRef;
};