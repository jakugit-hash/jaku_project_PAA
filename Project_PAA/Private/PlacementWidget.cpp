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
	if (GameMode && GameMode->GetPlayerUnitsToPlace().Contains("Sniper"))
	{
		GameMode->SetSelectedUnitType("Sniper");
		// Visual feedback
		SniperButton->SetBackgroundColor(FLinearColor::Green);
		BrawlerButton->SetBackgroundColor(FLinearColor::White);
	}
}

void UPlacementWidget::OnBrawlerButtonClicked()
{
	if (GameMode && GameMode->GetPlayerUnitsToPlace().Contains("Brawler"))
	{
		GameMode->SetSelectedUnitType("Brawler");
		// Visual feedback
		BrawlerButton->SetBackgroundColor(FLinearColor::Green);
		SniperButton->SetBackgroundColor(FLinearColor::White);
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