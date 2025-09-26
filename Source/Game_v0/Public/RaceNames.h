#pragma once
#include "RaceNames.generated.h"


UENUM()
enum class ERaceNames: int8
{
	None,
	Elves  UMETA(DisplayName = "Elves"),
	Dwarves UMETA(DisplayName = "Dwarves"),


	MAX UMETA(Hidden)//for iteration
	
	
};


#define FOREACH_RACE(OP)\
	OP(Dwarves, UDwarves)\
	OP(Elves, UElves)