// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"
#include "MapEditorTool/BlockEditorTool.h"
#include "MapEditorEnum.h"

class FMapEditorEdModeToolkit : public FModeToolkit
{
public:
	static FName ActionNames[];

public:
	FMapEditorEdModeToolkit();

	~FMapEditorEdModeToolkit();
	
	/** FModeToolkit interface */
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost) override;

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual class FEdMode* GetEditorMode() const override;
	virtual TSharedPtr<class SWidget> GetInlineContent() const override { return ToolkitWidget; }

	FMapEditorEdMode* GetMapEditorMode() const;

	TSharedRef<SWidget> MakeCheckBox(EMapEditorAction action, FString GroupName);

private:
	FReply StartTool(const FString& ToolTypeIdentifier);
	FReply EndTool(EToolShutdownType ShutdownType);

	void HandleCheckBoxChange();

	void HandleSelectEditorMapActor();
	void HandleLevelActorAdded(AActor* InActor);
	void HandleLevelActorDeleted(AActor* InActor);

private:
	TSharedPtr<SWidget> ToolkitWidget;

	TMap<FString, TArray<TSharedPtr<SCheckBox>>> CheckBoxGroupMap;
	EMapEditorAction CurAction;

	TSharedPtr<SComboBox<TWeakObjectPtr<AActor>>> SelectWidget;
	TArray<TWeakObjectPtr<AActor>> MapEditorActorArray;
	int CurSelect;
	FDelegateHandle AddActorHandler;
	FDelegateHandle deleteActorHandler;
	

	
};
