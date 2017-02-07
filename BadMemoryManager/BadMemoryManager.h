#pragma once

#include "resource.h"

typedef struct
{
	ULONGLONG LowerBound;
	ULONGLONG UpperBound;
} BAD_REGION, *PBAD_REGION;

typedef struct
{
	ULONGLONG LowerBound;
	ULONGLONG UpperBound;
	BOOLEAN Status;
} BAD_REGION_STATUS, *PBAD_REGION_STATUS;
