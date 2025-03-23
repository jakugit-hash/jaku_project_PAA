#include "CoinWidget.h"
#include "CoinTossManager.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UCoinWidget::SetCoinTossManager(ACoinTossManager* Manager)
{
	CoinTossManager = Manager;
}

void UCoinWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Log to verify NativeConstruct is called
	UE_LOG(LogTemp, Warning, TEXT("CoinWidget NativeConstruct called!"));

	// Bind the CoinButton's OnClicked event
	if (CoinButton)
	{
		CoinButton->OnClicked.AddDynamic(this, &UCoinWidget::OnCoinButtonClicked);
		UE_LOG(LogTemp, Warning, TEXT("CoinButton bound successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CoinButton is null!"));
	}
}

void UCoinWidget::OnCoinButtonClicked()
{
	// Call the DecideStartingPlayer function in CoinTossManager
	if (CoinTossManager)
	{
		CoinTossManager->DecideStartingPlayer();
		if (ResultText)
		{
			ResultText->SetText(FText::FromString(TEXT("Coin toss in progress...")));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CoinTossManager is null!"));
	}
}

void UCoinWidget::NativeDestruct()
{
	Super::NativeDestruct();

	// Clear button click bindings
	if (CoinButton)
	{
		CoinButton->OnClicked.RemoveAll(this);
	}

	// Clear any other bound delegates
	if (CoinTossManager)
	{
		CoinTossManager->OnCoinTossComplete.RemoveAll(this);
	}
}