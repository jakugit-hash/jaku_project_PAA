#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlacementWidget.generated.h"

// Forward declarations
class AMyGameMode;
class UButton;

UCLASS()
class PROJECT_PAA_API UPlacementWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Function to set the GameMode
	void SetGameMode(AMyGameMode* InGameMode);

protected:
	// Called when the widget is constructed
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Functions to handle button clicks
	UFUNCTION()
	void OnSniperButtonClicked();

	UFUNCTION()
	void OnBrawlerButtonClicked();

private:
	// Reference to the GameMode
	UPROPERTY()
	AMyGameMode* GameMode;

	// Bind the buttons
	UPROPERTY(meta = (BindWidget))
	UButton* SniperButton;

	UPROPERTY(meta = (BindWidget))
	UButton* BrawlerButton;
};