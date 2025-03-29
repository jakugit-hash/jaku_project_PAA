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

// Note: No class - this is a global enumeration