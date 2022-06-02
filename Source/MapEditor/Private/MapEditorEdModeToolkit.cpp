// Copyright Epic Games, Inc. All Rights Reserved.

#include "MapEditorEdModeToolkit.h"
#include "MapEditorEdMode.h"
#include "Engine/Selection.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBarTrack.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "MapEditorActor.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "FMapEditorEdModeToolkit"

FName FMapEditorEdModeToolkit::ActionNames[] = { "None", "Add Block", "Del Block", "Replace Material"};

FMapEditorEdModeToolkit::FMapEditorEdModeToolkit()
{
}

FMapEditorEdModeToolkit::~FMapEditorEdModeToolkit()
{
	GEditor->OnLevelActorAdded().Remove(AddActorHandler);
	GEditor->OnLevelActorDeleted().Remove(deleteActorHandler);
}

void FMapEditorEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	struct Locals
	{
		static bool IsWidgetEnabled()
		{
			return true;
		}
	};

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs(
		/*bUpdateFromSelection=*/ false,
		/*bLockable=*/ false,
		/*bAllowSearch=*/ false,
		FDetailsViewArgs::HideNameArea,
		/*bHideSelectionTip=*/ true,
		/*InNotifyHook=*/ nullptr,
		/*InSearchInitialKeyFocus=*/ false,
		/*InViewIdentifier=*/ NAME_None);
	DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Show;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.bAllowMultipleTopLevelObjects = true;

	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	TArray<AActor*> arr;
	UGameplayStatics::GetAllActorsOfClass(GEditor->GetWorldContexts()[0].World(), AMapEditorActor::StaticClass(), arr);
	for (auto v : arr)
	{
		MapEditorActorArray.Add(MakeWeakObjectPtr(v));
	}
	CurSelect = -1;
	CurAction = EMapEditorAction::none;

	AddActorHandler = GEditor->OnLevelActorAdded().AddRaw(this, &FMapEditorEdModeToolkit::HandleLevelActorAdded);
	deleteActorHandler = GEditor->OnLevelActorDeleted().AddRaw(this, &FMapEditorEdModeToolkit::HandleLevelActorDeleted);

	GetMapEditorMode()->GetToolManager()->OnToolEnded.AddLambda(
		[this](UInteractiveToolManager* ToolManager, UInteractiveTool* Tool) {
			ToggleCheckBox(EMapEditorAction::none, "Group1");
		});

	SAssignNew(ToolkitWidget, SScrollBarTrack)
		.IsEnabled_Static(&Locals::IsWidgetEnabled)
		.TopSlot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("Debug.Border"))
				[
					SNew(SWrapBox)
					.PreferredSize(300.0f)
					+ SWrapBox::Slot()
					.Padding(5)
					[
						MakeCheckBox(EMapEditorAction::none, "Group1")
					]
					+ SWrapBox::Slot()
					.Padding(5)
					[
						MakeCheckBox(EMapEditorAction::add, "Group1")
					]
					+ SWrapBox::Slot()
					.Padding(5)
					[
						MakeCheckBox(EMapEditorAction::del, "Group1")
					]
					+ SWrapBox::Slot()
					.Padding(5)
					[
						MakeCheckBox(EMapEditorAction::replace, "Group1")
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(EVerticalAlignment::VAlign_Center)
				.Padding(20, 0, 0, 0)
				[
					SNew(STextBlock).Text(FText::FromString("Select EditActor"))
				]
				+ SHorizontalBox::Slot()
				.VAlign(EVerticalAlignment::VAlign_Center)
				.Padding(0, 0, 50, 0)
				[
					SAssignNew(SelectWidget, SComboBox<TWeakObjectPtr<AActor>>)
					.OptionsSource(&MapEditorActorArray)
					.OnGenerateWidget_Lambda([this](TWeakObjectPtr<AActor> InValue)->TSharedRef<SWidget>
					{
						if (InValue.IsValid())
							return SNew(STextBlock).Text(FText::FromString(UKismetSystemLibrary::GetDisplayName(InValue.Get()))); // 下拉项的Widget
						return SNullWidget::NullWidget;
					})
					.OnSelectionChanged_Lambda([this](TWeakObjectPtr<AActor> SelectObj, ESelectInfo::Type SelectType) {
						CurSelect = MapEditorActorArray.Find(SelectObj);
						HandleSelectEditorMapActor();
					})
					[
						SNew(STextBlock)
						.Text_Lambda(
							[this]() {
								if ((CurSelect < 0 || CurSelect > MapEditorActorArray.Num() - 1))
									return FText::FromString("");
								else
								{
									if (MapEditorActorArray[CurSelect].IsValid())
									{
										return FText::FromString(UKismetSystemLibrary::GetDisplayName(MapEditorActorArray[CurSelect].Get()));
									}
									else
										return FText::FromString("");
								}
									
							})
					]
				]
			]
			+ SVerticalBox::Slot().HAlign(HAlign_Fill).FillHeight(1.f)
				[
					DetailsView->AsShared()
				]
		];
		
	FModeToolkit::Init(InitToolkitHost);
}

FName FMapEditorEdModeToolkit::GetToolkitFName() const
{
	return FName("MapEditorEdMode");
}

FText FMapEditorEdModeToolkit::GetBaseToolkitName() const
{
	return NSLOCTEXT("MapEditorEdModeToolkit", "DisplayName", "MapEditorEdMode Tool");
}

class FEdMode* FMapEditorEdModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(FMapEditorEdMode::EM_MapEditorEdModeId);
}

FMapEditorEdMode* FMapEditorEdModeToolkit::GetMapEditorMode() const
{
	return (FMapEditorEdMode*)GetEditorMode();
}

TSharedRef<SWidget> FMapEditorEdModeToolkit::MakeCheckBox(EMapEditorAction action, FString GroupName)
{
	auto CheckBox =
		SNew(SCheckBox)
		.Tag(ActionNames[action])
		.IsChecked(CurAction == action ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
		.OnCheckStateChanged_Raw(this, &FMapEditorEdModeToolkit::HandleCheckBoxChange, action, GroupName)
		[
			SNew(STextBlock).Text(FText::FromName(ActionNames[action]))
		];
	//添加到分组
	{
		TArray<TSharedPtr<SCheckBox>>* CheckBoxArray = CheckBoxGroupMap.Find(GroupName);
		TArray<TSharedPtr<SCheckBox>>Arr = CheckBoxArray ? *CheckBoxArray : TArray<TSharedPtr<SCheckBox>>();
		Arr.Add(CheckBox);
		CheckBoxGroupMap.Add(GroupName, Arr);
	}
	return CheckBox;
}

void FMapEditorEdModeToolkit::ToggleCheckBox(EMapEditorAction action, FString GroupName)
{
	CurAction = action;
	for (auto CheckBox : CheckBoxGroupMap[GroupName])
	{
		if (CheckBox->GetTag().IsEqual(ActionNames[CurAction]))
		{
			CheckBox->ToggleCheckedState();
			break;
		}
	}
}

void FMapEditorEdModeToolkit::HandleCheckBoxChange(ECheckBoxState InState, EMapEditorAction action, FString GroupName)
{
	//强制至少勾选一个
	if (InState == ECheckBoxState::Unchecked)
	{
		for (auto CheckBox : CheckBoxGroupMap[GroupName])
		{
			if (CheckBox->GetTag().IsEqual(ActionNames[action]))
			{
				CheckBox->SetIsChecked(ECheckBoxState::Checked);
				break;
			}
		}
		return;
	}
	//取消勾选同一组的其他CheckBox
	for (auto CheckBox : CheckBoxGroupMap[GroupName])
	{
		if (CheckBox->GetTag().IsEqual(ActionNames[action]))
			continue;
		CheckBox->SetIsChecked(ECheckBoxState::Unchecked);
	}
	CurAction = action;

	switch (CurAction)
	{
	case EMapEditorAction::none:
	{
		EndTool(EToolShutdownType::Completed);
		break;
	}
	case EMapEditorAction::add:
	case EMapEditorAction::del:
	case EMapEditorAction::replace:
	{
		StartTool("BlockEditorTool");
		break;
	}
	default:
		break;
	}
}

FReply FMapEditorEdModeToolkit::StartTool(const FString& ToolTypeIdentifier)
{
	if (GetMapEditorMode()->GetToolManager()->SelectActiveToolType(EToolSide::Left, ToolTypeIdentifier) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("ToolManager: Unknown Tool Type %s"), *ToolTypeIdentifier);
	}
	else
	{
		FString ActiveToolName = GetMapEditorMode()->GetToolManager()->GetActiveToolName(EToolSide::Left);
		if (ActiveToolName != ToolTypeIdentifier)
		{
			UE_LOG(LogTemp, Warning, TEXT("Starting Tool Type %s"), *ToolTypeIdentifier);
			GetMapEditorMode()->GetToolManager()->ActivateTool(EToolSide::Left);
			UBlockEditorTool* CurTool = (UBlockEditorTool*)GetMapEditorMode()->GetToolManager()->GetActiveTool(EToolSide::Left);
			if (CurTool)
			{
				DetailsView->SetObjects(CurTool->GetToolProperties(true));
			}
		}
		HandleSelectEditorMapActor();
	}

	return FReply::Handled();
}

FReply FMapEditorEdModeToolkit::EndTool(EToolShutdownType ShutdownType)
{
	UE_LOG(LogTemp, Warning, TEXT("EndTool"));

	GetMapEditorMode()->GetToolManager()->DeactivateTool(EToolSide::Left, ShutdownType);
	DetailsView->SetObject(nullptr);

	return FReply::Handled();
}

void FMapEditorEdModeToolkit::HandleLevelActorAdded(AActor* InActor)
{
	if (InActor->IsA(AMapEditorActor::StaticClass()) && MapEditorActorArray.Find(InActor) == INDEX_NONE)
	{
		MapEditorActorArray.Add(MakeWeakObjectPtr(InActor));
		CurSelect = FMath::Clamp(CurSelect, 0, MapEditorActorArray.Num() - 1);
		HandleSelectEditorMapActor();
	}
}

void FMapEditorEdModeToolkit::HandleLevelActorDeleted(AActor* InActor)
{
	auto Index = MapEditorActorArray.Find(InActor);
	if (InActor->IsA(AMapEditorActor::StaticClass()) && Index != INDEX_NONE)
	{
		MapEditorActorArray.Remove(InActor);
		CurSelect -= Index < CurSelect ? 1 : 0;
		CurSelect = MapEditorActorArray.Num() == 0 ? -1 : FMath::Clamp(CurSelect, 0, MapEditorActorArray.Num() - 1);
		HandleSelectEditorMapActor();
	}
}

void FMapEditorEdModeToolkit::HandleSelectEditorMapActor()
{
	UBlockEditorTool* CurTool = (UBlockEditorTool*)GetMapEditorMode()->GetToolManager()->GetActiveTool(EToolSide::Left);
	if (CurTool == nullptr)
		return;

	if (CurSelect >= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Select MapEditorActor"));
		CurTool->SetMapEditorActor((AMapEditorActor*)MapEditorActorArray[CurSelect].Get());
		CurTool->SetAction(CurAction);
	}
	else
	{
		CurTool->SetMapEditorActor(nullptr);
	}
}

#undef LOCTEXT_NAMESPACE
