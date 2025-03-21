
// Fill out your copyright notice in the Description page of Project Settings.


#include "TopDownCamera.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
// Sets default values
ATopDownCamera::ATopDownCamera()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	// Create Camera Component
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	RootComponent = CameraComponent;

	// Set Default Position
	SetupCamera();
}
// Setup Camera View
void ATopDownCamera::SetupCamera()
{
	if (!CameraComponent) return;

	float CameraHeight = (GridManager) ? (GridManager->GetGridSizeX() * GridManager->GetCellSize() * 2.0f) : 5000.0f;

	if (GridManager)
	{
		float GridSizeX = GridManager->GetGridSizeX();
		float GridSizeY = GridManager->GetGridSizeY();
		float CellSize = GridManager->GetCellSize();

		float GridCenterX = (GridSizeX * CellSize) / 2.0f;
		float GridCenterY = (GridSizeY * CellSize) / 2.0f;

		CameraComponent->SetWorldLocation(GetActorLocation()+ FVector(GridCenterX, GridCenterY, CameraHeight));
		CameraComponent->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f), false); // Face downward
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GridManager is NOT assigned! Please set it in the Editor."));
		CameraComponent->SetWorldLocation(FVector(1250.0f, 1250.0f, CameraHeight));
		CameraComponent->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f), false);
	}
}

// Called when the game starts or when spawned
void ATopDownCamera::BeginPlay()
{
	Super::BeginPlay();
	// Assign this camera as the player's view
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		PlayerController->SetViewTarget(this);
	}
	
}



