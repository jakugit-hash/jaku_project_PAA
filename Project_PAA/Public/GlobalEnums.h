// Project_PAA/Source/Project_PAA/Public/GlobalEnums.h
#pragma once

#include "CoreMinimal.h"
#include "GlobalEnums.generated.h" // Required for UENUM()

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	Placement      UMETA(DisplayName = "Unit Placement Phase"),
	UnitAction     UMETA(DisplayName = "Combat Action Phase")
};

UENUM(BlueprintType)
enum class EUnitActionState : uint8
{
	None        UMETA(DisplayName="Nessuna azione"),
	Selecting   UMETA(DisplayName="Selezione unità"),
	Moving      UMETA(DisplayName="Movimento"),
	Attacking   UMETA(DisplayName="Attacco")
};


// Note: No class - this is a global enumeration