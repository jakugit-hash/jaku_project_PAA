#include "WBP_ActionWidget.h"
#include "MyGameMode.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"

void UWBP_ActionWidget::Setup(AMyGameMode* GameMode)
{
    GameModeRef = GameMode;

    if (MoveButton)
    {
        MoveButton->OnClicked.AddUniqueDynamic(this, &UWBP_ActionWidget::OnMoveClicked);
    }

    if (AttackButton)
    {
        AttackButton->OnClicked.AddUniqueDynamic(this, &UWBP_ActionWidget::OnAttackClicked);
    }

    if (EndTurnButton)
    {
        EndTurnButton->OnClicked.AddUniqueDynamic(this, &UWBP_ActionWidget::OnEndTurnClicked);
    }

    UE_LOG(LogTemp, Warning, TEXT("Widget buttons bound successfully!"));
}

void UWBP_ActionWidget::OnMoveClicked()
{
	if (GameModeRef)
	{
		GameModeRef->HandleMoveAction();
	}
}

void UWBP_ActionWidget::OnAttackClicked()
{
	if (GameModeRef) GameModeRef->HandleAttackAction();
}

void UWBP_ActionWidget::OnEndTurnClicked()
{
	if (GameModeRef)
	{
		GameModeRef->ClearSelection(); // Added cleanup
		GameModeRef->EndTurn();
	}
}

void UWBP_ActionWidget::UpdateButtons(bool bCanMove, bool bCanAttack)
{
	if(MoveButton) MoveButton->SetIsEnabled(bCanMove);
	if(AttackButton) 
	{
		AttackButton->SetIsEnabled(bCanAttack);
		AttackButton->SetVisibility(bCanAttack ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UWBP_ActionWidget::UpdateBordersVisibility(bool bIsPlayerTurn)
{
	if (HPBORDER)
	{
		HPBORDER->SetVisibility(bIsPlayerTurn ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (AIBORDER)
	{
		AIBORDER->SetVisibility(bIsPlayerTurn ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
	}
}

void UWBP_ActionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UE_LOG(LogTemp, Warning, TEXT("ActionWidget Constructed"));
    
	if (!MoveButton) UE_LOG(LogTemp, Error, TEXT("Missing MoveButton!"));
	if (!AttackButton) UE_LOG(LogTemp, Error, TEXT("Missing AttackButton!"));
	if (!EndTurnButton) UE_LOG(LogTemp, Error, TEXT("Missing EndTurnButton!"));
	
	//esempi di utilizzo dei nuovi border (opzionale)
	if (HPBORDER)
	{
		HPBORDER->SetVisibility(ESlateVisibility::Hidden); //oppure Hidden, come vuoi
	}

	if (AIBORDER)
	{
		AIBORDER->SetVisibility(ESlateVisibility::Hidden);
	}
	if (BRAWLERHEALTH)
		BRAWLERHEALTH->SetPercent(1.0f);

	if (SNIPERHEALTH)
		SNIPERHEALTH->SetPercent(1.0f);

}

void UWBP_ActionWidget::SetVisibility(ESlateVisibility InVisibility)
{
	Super::SetVisibility(InVisibility);
	UE_LOG(LogTemp, Warning, TEXT("Widget visibility set to: %d"), (int32)InVisibility);
}