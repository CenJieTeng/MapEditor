// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveToolBuilder.h"
#include "BaseTools/SingleClickTool.h"
#include "MapEditorActor.h"
#include "MapEditorEnum.h"
#include "PreviewMesh.h"
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

UCLASS()
class UBlockEditorToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	UBlockEditorToolProperties();

	UPROPERTY(EditAnywhere, NonTransactional, Category = Options)
	bool bShowPreviewMesh = true;

	UPROPERTY(EditAnywhere, NonTransactional, Category = Options, meta = (UIMin = "1", UIMax = "10", ClampMin = "1", ClampMax = "100"))
	int x = 1;

	UPROPERTY(EditAnywhere, NonTransactional, Category = Options, meta = (UIMin = "1", UIMax = "10", ClampMin = "1", ClampMax = "100"))
	int y = 1;

	UPROPERTY(EditAnywhere, NonTransactional, Category = Options, meta = (UIMin = "1", UIMax = "10", ClampMin = "1", ClampMax = "100"))
	int z = 1;

	UPROPERTY(EditAnywhere, NonTransactional, Category = Material)
	TWeakObjectPtr<UMaterialInterface> CustomMaterial;

	UPROPERTY(EditAnywhere, NonTransactional, Category = Material)
	bool bWireFrame = true;
};

/**
 *	
 */
UCLASS()
class UBlockEditorTool : public USingleClickTool, public IHoverBehaviorTarget
{
	GENERATED_BODY()

public:
	UBlockEditorTool();

	virtual void Setup() override;

	virtual void Shutdown(EToolShutdownType ShutdownType) override;

	virtual void OnClicked(const FInputDeviceRay& ClickPos) override;

	virtual void OnPropertyModified(UObject* PropertySet, FProperty* Property) override;

	// IHoverBehaviorTarget interface
	virtual FInputRayHit BeginHoverSequenceHitTest(const FInputDeviceRay& PressPos) override;
	virtual void OnBeginHover(const FInputDeviceRay& DevicePos) override;
	virtual bool OnUpdateHover(const FInputDeviceRay& DevicePos) override;
	virtual void OnEndHover() override;

	virtual void SetWorld(UWorld* World);
	virtual void SetMapEditorActor(TWeakObjectPtr<AMapEditorActor> Actor);
	virtual void SetAction(EMapEditorAction InAction);

protected:
	virtual void GenerateMesh(FDynamicMesh3* OutMesh, FVector3d Size) const;
	virtual void UpdatePreviewMesh();

protected:
	UPROPERTY()
	UBlockEditorToolProperties* Properties;

	UWorld* TargetWorld;

	TWeakObjectPtr<AMapEditorActor> MapEditorActor;

	EMapEditorAction CurAction;

private:
	UStaticMesh* DefaultMesh;
	UMaterialInterface* DefaultMaterial;
	UPreviewMesh* PreviewMesh;
	UMaterialInterface* PreviewMaterial;
};
