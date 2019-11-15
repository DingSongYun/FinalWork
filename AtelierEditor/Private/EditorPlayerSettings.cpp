// Fill out your copyright notice in the Description page of Project Settings.


#include "EditorPlayerSettings.h"
#include "UObjectIterator.h"

EditorPlayerSettings * EditorPlayerSettings::Get()
{
	static EditorPlayerSettings* Instance;

	if (Instance != nullptr)
	{
		return Instance;
	}

	for (TObjectIterator<EditorPlayerSettings> SettingsIt(RF_NoFlags); SettingsIt; ++SettingsIt)
	{
		Instance = *SettingsIt;
		break;
	}

	return Instance;
}
