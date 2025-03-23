#include "TopDownCamera.h"
#include "GridManager.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"

// Sets default values
ATopDownCamera::ATopDownCamera()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create Camera Component
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	RootComponent = CameraComponent;
}

void ATopDownCamera::BeginPlay()
{
	Super::BeginPlay();

	// Set up the camera
	SetupCamera();

	// Assign this camera as the player's view
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		PlayerController->SetViewTarget(this);
	}
}

void ATopDownCamera::SetupCamera()
{
	if (!GridManager)
	{
		UE_LOG(LogTemp, Error, TEXT("GridManager is not assigned!"));
		return;
	}

	// Calculate the center of the grid
	float GridCenterX = (GridManager->GetGridSizeX() * GridManager->GetCellSize()) / 2.0f;
	float GridCenterY = (GridManager->GetGridSizeY() * GridManager->GetCellSize()) / 2.0f;

	// Calculate the camera height based on the grid size
	float CameraHeight = FMath::Max(GridManager->GetGridSizeX(), GridManager->GetGridSizeY()) * GridManager->GetCellSize() * 0.8f;

	// Set the camera position and rotation
	FVector CameraLocation = FVector(GridCenterX, GridCenterY, CameraHeight);
	FRotator CameraRotation = FRotator(-90.0f, 0.0f, 0.0f); // Face downward

	CameraComponent->SetWorldLocationAndRotation(CameraLocation, CameraRotation);
}