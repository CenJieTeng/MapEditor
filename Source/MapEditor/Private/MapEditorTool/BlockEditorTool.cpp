// Fill out your copyright notice in the Description page of Project Settings.


#include "MapEditorTool/BlockEditorTool.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/ConstructorHelpers.h"
#include "InteractiveToolManager.h"
#include "BaseBehaviors/MouseHoverBehavior.h"

//#include "Generators/SweepGenerator.h"
#include "Generators/GridBoxMeshGenerator.h"
//#include "Generators/RectangleMeshGenerator.h"
//#include "Generators/SphereGenerator.h"
//#include "Generators/BoxSphereGenerator.h"
//#include "Generators/DiscMeshGenerator.h"

bool UBlockEditorToolBuilder::CanBuildTool(const FToolBuilderState& SceneState) const
{
	return (AssetAPI != nullptr);
}

UInteractiveTool* UBlockEditorToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	UBlockEditorTool* NewTool = NewObject<UBlockEditorTool>(SceneState.ToolManager);
	NewTool->TargetWorld = SceneState.World;
	return NewTool;
}

/*
* Tool
*/

//UBlockEditorToolProperties::UBlockEditorToolProperties()
//{
//	MapEditorActor = nullptr;
//}

UBlockEditorTool::UBlockEditorTool()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> FindMesh(TEXT("/MapEditor/Shapes/Shape_Cube.Shape_Cube"));
	DefaultMesh = FindMesh.Object;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> FindMaterial(TEXT("/MapEditor/Materials/M_Concrete_Tiles.M_Concrete_Tiles"));
	DefaultMaterial = FindMaterial.Object;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> FindPreviewMaterial(TEXT("/MapEditor/Materials/M_Wireframe.M_Wireframe"));
	PreviewMaterial = FindPreviewMaterial.Object;
}

void UBlockEditorTool::Setup()
{
	USingleClickTool::Setup();

	//Properties = NewObject<UBlockEditorToolProperties>();
	//AddToolPropertySource(Properties);

	UMouseHoverBehavior* HoverBehavior = NewObject<UMouseHoverBehavior>(this);
	HoverBehavior->Initialize(this);
	AddInputBehavior(HoverBehavior);

	//create preview mesh
	PreviewMesh = NewObject<UPreviewMesh>(this, TEXT("PreviewMesh"));
	PreviewMesh->CreateInWorld(TargetWorld, FTransform::Identity);
	PreviewMesh->SetMaterial(PreviewMaterial);
	PreviewMesh->SetVisible(false);

	FDynamicMesh3 NewMesh;
	GenerateMesh(&NewMesh);
	PreviewMesh->UpdatePreview(&NewMesh);

	MapEditorActor = nullptr;
}

void UBlockEditorTool::Shutdown(EToolShutdownType ShutdownType)
{
	PreviewMesh->SetVisible(false);
	PreviewMesh->Disconnect();
	PreviewMesh = nullptr;
}

void UBlockEditorTool::OnClicked(const FInputDeviceRay& ClickPos)
{
	if (MapEditorActor == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("MapEditorActor not found, Make sure there is MapEditorActor in the scene and select it from the ComboBox below!"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("BlockEditorTool OnClick"));

	if (MapEditorActor.IsValid())
	{
		FName ComponentName = FName(FString::FromInt(MapEditorActor->GetBlockIdCount()));
		
		if (DefaultMesh)
		{
			FVector RayStart = ClickPos.WorldRay.Origin;
			FVector RayEnd = ClickPos.WorldRay.PointAt(999999);
			FCollisionObjectQueryParams QueryParams(FCollisionObjectQueryParams::AllObjects);
			FHitResult Result;
			bool bHitWorld = TargetWorld->LineTraceSingleByObjectType(Result, RayStart, RayEnd, QueryParams);

			if (Result.Actor.IsValid() && Result.Actor->GetUniqueID() == MapEditorActor->GetUniqueID() && bHitWorld)
			{
				if (CurAction == EMapEditorAction::del)
				{
					if (Result.Component.IsValid())
					{
						Result.Component->DestroyComponent();
					}
					return;
				}

				if (Result.Component.IsValid())
				{
					FVector NewLocationOffset = (100 * Result.Normal);
					UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(MapEditorActor.Get(), ComponentName, RF_Transactional);
					Component->SetStaticMesh(DefaultMesh);
					Component->SetMaterial(0, DefaultMaterial);
					Component->SetWorldLocation(Result.Component->GetComponentLocation() + NewLocationOffset);
					Component->RegisterComponent();
					MapEditorActor->AddInstanceComponent(Component);
					MapEditorActor->SetBlockIdCount(MapEditorActor->GetBlockIdCount() + 1);
				}
			}
		}
	}
}

FInputRayHit UBlockEditorTool::BeginHoverSequenceHitTest(const FInputDeviceRay& PressPos)
{
	return FInputRayHit(0.0f);
}

void UBlockEditorTool::OnBeginHover(const FInputDeviceRay& DevicePos)
{
	//UpdatePreviewPosition(DevicePos);
}

bool UBlockEditorTool::OnUpdateHover(const FInputDeviceRay& DevicePos)
{
	if (MapEditorActor.IsValid())
	{
		//UpdatePreviewPosition(DevicePos);
		PreviewMesh->SetVisible(true);
		FVector RayStart = DevicePos.WorldRay.Origin;
		FVector RayEnd = DevicePos.WorldRay.PointAt(999999);
		FCollisionObjectQueryParams QueryParams(FCollisionObjectQueryParams::AllObjects);
		FHitResult Result;
		bool bHitWorld = TargetWorld->LineTraceSingleByObjectType(Result, RayStart, RayEnd, QueryParams);
		if (Result.Actor.IsValid() && Result.Actor->GetUniqueID() == MapEditorActor->GetUniqueID() && bHitWorld)
		{
			if (CurAction == EMapEditorAction::del)
			{
				if (Result.Component.IsValid())
				{
					PreviewMesh->SetVisible(true);
					FTransform NewTransform;
					NewTransform.SetLocation(Result.Component->GetComponentLocation() + FVector(0.0f, 0.0, 50.0f));
					PreviewMesh->SetTransform(NewTransform);
				}
				return true;
			}

			if (Result.Component.IsValid())
			{
				FVector NewLocationOffset = (100 * Result.Normal);
				PreviewMesh->SetVisible(true);
				FTransform NewTransform;
				NewTransform.SetLocation(Result.Component->GetComponentLocation() + FVector(0.0f, 0.0, 50.0f) + NewLocationOffset);
				PreviewMesh->SetTransform(NewTransform);
			}
		}
		else
		{
			PreviewMesh->SetVisible(false);
		}
	}
	
	return true;
}

void UBlockEditorTool::OnEndHover()
{
}

void UBlockEditorTool::GenerateMesh(FDynamicMesh3* OutMesh) const
{
	FGridBoxMeshGenerator BoxGen;
	BoxGen.Box = FOrientedBox3d(FVector3d::Zero(), 0.5 * FVector3d(100.f, 100.f, 100.0f));
	BoxGen.Generate();
	OutMesh->Copy(&BoxGen);
}