#include "PlacementWidget.h"
#include "MyGameMode.h"
#include "Components/Button.h"

void UPlacementWidget::SetGameMode(AMyGameMode* InGameMode)
{
	GameMode = InGameMode;
}

void UPlacementWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button click events
	if (SniperButton)
	{
		SniperButton->OnClicked.AddDynamic(this, &UPlacementWidget::OnSniperButtonClicked);
		
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SniperButton is null!"));
	}

	if (BrawlerButton)
	{
		BrawlerButton->OnClicked.AddDynamic(this, &UPlacementWidget::OnBrawlerButtonClicked);
		
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BrawlerButton is null!"));
	}
}

void UPlacementWidget::OnSniperButtonClicked()
{
	if (GameMode && GameMode->CanPlaceSniper())
	{
		GameMode->SetSelectedUnitType("Sniper");
		SniperButton->SetBackgroundColor(FLinearColor::Green);
		BrawlerButton->SetBackgroundColor(FLinearColor::White);
	}
}

void UPlacementWidget::OnBrawlerButtonClicked()
{
	if (GameMode && GameMode->CanPlaceBrawler())
	{
		GameMode->SetSelectedUnitType("Brawler");
		BrawlerButton->SetBackgroundColor(FLinearColor::Green);
		SniperButton->SetBackgroundColor(FLinearColor::White);
	}
}

void UPlacementWidget::UpdateButtonStates()
{
	if (!GameMode) return;
    
	SniperButton->SetIsEnabled(GameMode->CanSelectUnitType("Sniper"));
	BrawlerButton->SetIsEnabled(GameMode->CanSelectUnitType("Brawler"));
}

void UPlacementWidget::ClearSelection()
{
	if (SniperButton) 
	{
		SniperButton->SetBackgroundColor(FLinearColor::Blue);
		SniperButton->SetIsEnabled(true);
	}
	if (BrawlerButton) 
	{
		BrawlerButton->SetBackgroundColor(FLinearColor::Green);
		BrawlerButton->SetIsEnabled(true);
	}
}

void UPlacementWidget::NativeDestruct()
{
	Super::NativeDestruct();

	// Clear button click bindings
	if (SniperButton)
	{
		SniperButton->OnClicked.RemoveAll(this);
	}

	if (BrawlerButton)
	{
		BrawlerButton->OnClicked.RemoveAll(this);
	}
}