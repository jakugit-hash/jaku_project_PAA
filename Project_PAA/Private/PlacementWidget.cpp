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
		UE_LOG(LogTemp, Warning, TEXT("SniperButton bound successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SniperButton is null!"));
	}

	if (BrawlerButton)
	{
		BrawlerButton->OnClicked.AddDynamic(this, &UPlacementWidget::OnBrawlerButtonClicked);
		UE_LOG(LogTemp, Warning, TEXT("BrawlerButton bound successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BrawlerButton is null!"));
	}
}

void UPlacementWidget::OnSniperButtonClicked()
{
	if (GameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("Sniper button clicked!"));
		GameMode->SetSelectedUnitType(TEXT("Sniper"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode is null!"));
	}
}

void UPlacementWidget::OnBrawlerButtonClicked()
{
	if (GameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("Brawler button clicked!"));
		GameMode->SetSelectedUnitType(TEXT("Brawler"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode is null!"));
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