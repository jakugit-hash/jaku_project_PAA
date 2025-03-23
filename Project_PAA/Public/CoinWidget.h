#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CoinWidget.generated.h"

// Forward declarations
class ACoinTossManager;
class UButton;
class UTextBlock;

UCLASS()
class PROJECT_PAA_API UCoinWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Function to set the CoinTossManager
	void SetCoinTossManager(ACoinTossManager* Manager);

protected:
	// Called when the widget is constructed
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Function to handle the CoinButton click
	UFUNCTION()
	void OnCoinButtonClicked();

private:
	// Reference to the CoinTossManager
	UPROPERTY()
	ACoinTossManager* CoinTossManager;

	// Bind the CoinButton
	UPROPERTY(meta = (BindWidget))
	UButton* CoinButton;

	// Bind the ResultText
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ResultText;
};