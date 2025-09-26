#include "resource.h"


FResource::FResource(const FName ResourceName, const int32 initial_amount = 0, const float initial_weight = 1.0 )

{
	this->ResourceName = ResourceName;
	ResourceAmount = initial_amount;
	Weight = initial_weight;
}

