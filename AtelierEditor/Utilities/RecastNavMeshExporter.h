#pragma once

#include "RecastNavMeshGenerator.h"

class RecastExporter
{
public:
	static void ExportNavigationData(const FRecastNavMeshGenerator* Generator, const FString& FileName);
};
