// Copyright Epic Games, Inc. All Rights Reserved.

#include "MapEditorEdMode.h"
#include "MapEditorEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "EditorModeManager.h"

#include "MapEditorTool/BlockEditorTool.h"

const FEditorModeID FMapEditorEdMode::EM_MapEditorEdModeId = TEXT("EM_MapEditorEdMode");

FMapEditorEdMode::FMapEditorEdMode()
{
	ToolsContext = nullptr;
}

FMapEditorEdMode::~FMapEditorEdMode()
{
	if (ToolsContext != nullptr)
	{
		ToolsContext->ShutdownContext();
		ToolsContext = nullptr;
	}
}

void FMapEditorEdMode::Enter()
{
	FEdMode::Enter();

	//初始化Tool适配器
	ToolsContext = NewObject<UEdModeInteractiveToolsContext>(GetTransientPackage(), TEXT("ToolsContext"), RF_Transactional);
	ToolsContext->InitializeContextFromEdMode(this);

	//注册ToolBuilder,字符串标识符与 ToolBuilder 实现相关联
	auto BlockEditorToolBuilder = NewObject<UBlockEditorToolBuilder>();
	BlockEditorToolBuilder->AssetAPI = ToolsContext->GetAssetAPI();
	ToolsContext->ToolManager->RegisterToolType(TEXT("BlockEditorTool"), BlockEditorToolBuilder);
	
	//选择默认的Tool
	ToolsContext->ToolManager->SelectActiveToolType(EToolSide::Left, TEXT("BlockEditorTool"));

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FMapEditorEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}
}

void FMapEditorEdMode::Exit()
{
	ToolsContext->ShutdownContext();
	ToolsContext = nullptr;

	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();
}

void FMapEditorEdMode::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{
	FEdMode::Tick(ViewportClient, DeltaTime);

	if (ToolsContext != nullptr)
	{
		ToolsContext->Tick(ViewportClient, DeltaTime);
	}
}

void FMapEditorEdMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	FEdMode::Render(View, Viewport, PDI);

	if (ToolsContext != nullptr)
	{
		ToolsContext->Render(View, Viewport, PDI);
	}
}

bool FMapEditorEdMode::CanAutoSave() const
{
	return ToolsContext->ToolManager->HasAnyActiveTool() == false;
}

bool FMapEditorEdMode::UsesToolkits() const
{
	return true;
}

void FMapEditorEdMode::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(ToolsContext);
}

bool FMapEditorEdMode::MouseEnter(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y)
{
	bool bHandled = ToolsContext->MouseEnter(ViewportClient, Viewport, x, y);
	return bHandled;
}

bool FMapEditorEdMode::MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y)
{
	bool bHandled = ToolsContext->MouseMove(ViewportClient, Viewport, x, y);
	return bHandled;
}

bool FMapEditorEdMode::MouseLeave(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
	bool bHandled = ToolsContext->MouseLeave(ViewportClient, Viewport);
	return bHandled;
}

bool FMapEditorEdMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	if (Key == EKeys::One && Event == EInputEvent::IE_Pressed && ViewportClient->IsCtrlPressed())
	{
		((FMapEditorEdModeToolkit*)Toolkit.Get())->ToggleCheckBox(EMapEditorAction::none, "Group1");
	}
	if (Key == EKeys::Two && Event == EInputEvent::IE_Pressed && ViewportClient->IsCtrlPressed())
	{
		((FMapEditorEdModeToolkit*)Toolkit.Get())->ToggleCheckBox(EMapEditorAction::add, "Group1");
	}
	if (Key == EKeys::Three && Event == EInputEvent::IE_Pressed && ViewportClient->IsCtrlPressed())
	{
		((FMapEditorEdModeToolkit*)Toolkit.Get())->ToggleCheckBox(EMapEditorAction::del, "Group1");
	}
	if (Key == EKeys::Four && Event == EInputEvent::IE_Pressed && ViewportClient->IsCtrlPressed())
	{
		((FMapEditorEdModeToolkit*)Toolkit.Get())->ToggleCheckBox(EMapEditorAction::replace, "Group1");
	}

	bool bHandled = FEdMode::InputKey(ViewportClient, Viewport, Key, Event);
	bHandled |= ToolsContext->InputKey(ViewportClient, Viewport, Key, Event);
	return bHandled;
}

bool FMapEditorEdMode::InputAxis(FEditorViewportClient* InViewportClient, FViewport* Viewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime)
{
	// mouse axes: EKeys::MouseX, EKeys::MouseY, EKeys::MouseWheelAxis
	return FEdMode::InputAxis(InViewportClient, Viewport, ControllerId, Key, Delta, DeltaTime);
}


bool FMapEditorEdMode::StartTracking(FEditorViewportClient* InViewportClient, FViewport* InViewport)
{
	bool bHandled = FEdMode::StartTracking(InViewportClient, InViewport);
	bHandled |= ToolsContext->StartTracking(InViewportClient, InViewport);
	return bHandled;
}

bool FMapEditorEdMode::CapturedMouseMove(FEditorViewportClient* InViewportClient, FViewport* InViewport, int32 InMouseX, int32 InMouseY)
{
	bool bHandled = ToolsContext->CapturedMouseMove(InViewportClient, InViewport, InMouseX, InMouseY);
	return bHandled;
}

bool FMapEditorEdMode::EndTracking(FEditorViewportClient* InViewportClient, FViewport* InViewport)
{
	bool bHandled = ToolsContext->EndTracking(InViewportClient, InViewport);
	return bHandled;
}