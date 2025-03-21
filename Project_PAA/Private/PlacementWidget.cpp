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

	if (BrawlerButton)
	{
		BrawlerButton->OnClicked.AddDynamic(this, &UPlacementWidget::OnBrawlerButtonClicked);
	}
}

void UPlacementWidget::OnSniperButtonClicked()
{
	if (GameMode)
	{
		GameMode->SetSelectedUnitType(TEXT("Sniper"));
	}
}

void UPlacementWidget::OnBrawlerButtonClicked()
{
	if (GameMode)
	{
		GameMode->SetSelectedUnitType(TEXT("Brawler"));
	}
}