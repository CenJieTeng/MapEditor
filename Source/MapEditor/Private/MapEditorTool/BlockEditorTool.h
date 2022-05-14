// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveToolBuilder.h"
#include "BaseTools/SingleClickTool.h"
#include "MapEditorActor.h"
#include "MapEditorEnum.h"
#include "BlockEditorTool.generated.h"

/**
 *	Builder for UBlockEditorTool
 */
UCLASS()
class UBlockEditorToolBuilder : public UInteractiveToolBuilder
{
	GENERATED_BODY()

public:
	IToolsContextAssetAPI* AssetAPI;

	UBlockEditorToolBuilder() { AssetAPI = nullptr; }

	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override;
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
};

//UCLASS()
//class UBlockEditorToolProperties : public UInteractiveToolPropertySet
//{
//	GENERATED_BODY()
//
//public:
//	UBlockEditorToolProperties();
//
//	TWeakObjectPtr<AMapEditorActor> MapEditorActor;
//};

/**
 *	
 */
UCLASS()
class UBlockEditorTool : public USingleClickTool
{
	GENERATED_BODY()

public:
	UBlockEditorTool();

	virtual void Setup() override;

	virtual void OnClicked(const FInputDeviceRay& ClickPos) override;

public:
	//UPROPERTY()
	//UBlockEditorToolProperties* Properties;

	UWorld* TargetWorld;

	TWeakObjectPtr<AMapEditorActor> MapEditorActor;

	EMapEditorAction CurAction;

private:
	static UStaticMesh* DefaultMesh;
	static UMaterialInterface* DefaultMaterial;
};
