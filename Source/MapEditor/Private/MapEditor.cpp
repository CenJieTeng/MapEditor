// Copyright Epic Games, Inc. All Rights Reserved.

#include "MapEditor.h"
#include "MapEditorEdMode.h"

#define LOCTEXT_NAMESPACE "FMapEditorModule"

void FMapEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FEditorModeRegistry::Get().RegisterMode<FMapEditorEdMode>(FMapEditorEdMode::EM_MapEditorEdModeId, LOCTEXT("MapEditorEdModeName", "MapEditorEdMode"), FSlateIcon(), true);
}

void FMapEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEditorModeRegistry::Get().UnregisterMode(FMapEditorEdMode::EM_MapEditorEdModeId);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMapEditorModule, MapEditor)