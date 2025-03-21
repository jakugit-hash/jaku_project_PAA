
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "GridManager.h"
#include "TopDownCamera.generated.h"

UCLASS()
class PROJECT_PAA_API ATopDownCamera : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATopDownCamera();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	UCameraComponent* CameraComponent;

	// Reference to GridManager
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	AGridManager* GridManager;
    
	// Set Camera Position and Rotation
	void SetupCamera();

};
