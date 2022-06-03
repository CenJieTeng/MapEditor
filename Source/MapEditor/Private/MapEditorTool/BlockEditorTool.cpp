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

#include <iomanip>

FVector NormalToAxisDirection(FVector vector, FVector normal)
{
	if (normal.X != 0)
	{
		if (normal.X > 0)
		{
			return vector * FVector(1.0f, -1.0f, 1.0f);
		}
		else
		{
			return vector * FVector(-1.0f, 1.0f, 1.0f);
		}
	}
	if (normal.Y != 0)
	{
		if (normal.Y > 0)
		{
			return vector.RotateAngleAxis(-90.0, FVector(0, 0, 1)) * FVector(1.0f, -1.0f, 1.0f);
		}
		else
		{
			return vector.RotateAngleAxis(90.0, FVector(0, 0, 1)) * FVector(1.0f, -1.0f, 1.0f);
		}
	}
	if (normal.Z != 0)
	{
		if (normal.Z > 0)
		{
			return vector.RotateAngleAxis(-90, FVector(0, 1, 0)).RotateAngleAxis(-90, FVector(0, 0, 1));
		}
		else
		{
			return vector.RotateAngleAxis(90.0, FVector(0, 1, 0)).RotateAngleAxis(-90, FVector(0, 0, 1));
		}
	}

	return normal;
	//return FVector(1.0f, 1.0f, 1.0f);
}

FString FillString(int32 InValue, int32 num)
{
	std::stringstream ss;
	ss << std::setw(num) << std::setfill('0') << InValue;
	return FString(ss.str().c_str());
}

bool UBlockEditorToolBuilder::CanBuildTool(const FToolBuilderState& SceneState) const
{
	return (AssetAPI != nullptr);
}

UInteractiveTool* UBlockEditorToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	UBlockEditorTool* NewTool = NewObject<UBlockEditorTool>(SceneState.ToolManager);
	NewTool->SetWorld(SceneState.World);
	return NewTool;
}

/*
* Tool
*/

UBlockEditorToolProperties::UBlockEditorToolProperties()
{
}

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

	Properties = NewObject<UBlockEditorToolProperties>(this);
	AddToolPropertySource(Properties);
	Properties->RestoreProperties(this);

	UMouseHoverBehavior* HoverBehavior = NewObject<UMouseHoverBehavior>(this);
	HoverBehavior->Initialize(this);
	AddInputBehavior(HoverBehavior);

	//create preview mesh
	PreviewMesh = NewObject<UPreviewMesh>(this, TEXT("PreviewMesh"));
	PreviewMesh->CreateInWorld(TargetWorld, FTransform::Identity);
	PreviewMesh->SetMaterial(PreviewMaterial);
	PreviewMesh->SetVisible(false);

	UpdatePreviewMesh();

	MapEditorActor = nullptr;
}

void UBlockEditorTool::Shutdown(EToolShutdownType ShutdownType)
{
	PreviewMesh->SetVisible(false);
	PreviewMesh->Disconnect();
	PreviewMesh = nullptr;

	Properties->SaveProperties(this);
}

void UBlockEditorTool::OnClicked(const FInputDeviceRay& ClickPos)
{
	if (MapEditorActor == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("MapEditorActor not found, Make sure there is MapEditorActor in the scene and select it from the ComboBox below!"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("BlockEditorTool OnClick"));

	if (MapEditorActor.IsValid() && DefaultMesh)
	{
		FVector RayStart = ClickPos.WorldRay.Origin;
		FVector RayEnd = ClickPos.WorldRay.PointAt(999999);
		FCollisionObjectQueryParams QueryParams(FCollisionObjectQueryParams::AllObjects);
		FHitResult Result;
		bool bHitWorld = TargetWorld->LineTraceSingleByObjectType(Result, RayStart, RayEnd, QueryParams);

		if (Result.Actor.IsValid() && Result.Component.IsValid() && Result.Actor->GetUniqueID() == MapEditorActor->GetUniqueID() && bHitWorld)
		{
			FViewport* viewport = GEditor->GetActiveViewport();
			if (viewport && IsCtrlDown(viewport))
			{
				Properties->CustomMaterial = Result.Component->GetMaterial(0);
				return;
			}

			switch (CurAction)
			{
				case EMapEditorAction::add:
				{
					GEditor->BeginTransaction(FText::FromString("Add Block"));
					MapEditorActor->Modify();
					for (int x = 1; x <= Properties->x; x++)
					{
						for (int y = 1; y <= Properties->y; y++)
						{
							for (int z = 1; z <= Properties->z; z++)
							{
								
								FVector Normal = FVector((int)Result.Normal.X, (int)Result.Normal.Y, (int)Result.Normal.Z);
								FVector NewLocationOffset = NormalToAxisDirection(FVector((x - 1), (y - 1), (z - 1)), Normal) * 100 + Normal * 100;
								FVector NewLocation = Result.Component->GetComponentLocation() + NewLocationOffset;
								FVector Coord = (NewLocation - MapEditorActor->GetActorLocation()) / 100;
								Coord = FVector(FMath::RoundToInt(Coord.X), FMath::RoundToInt(Coord.Y), FMath::RoundToInt(Coord.Z));
								UE_LOG(LogTemp, Warning, TEXT("Block coord %f %f %f"), Coord.X, Coord.Y, Coord.Z);
								
								FName ComponentName = FName(FString("x") + FillString(Coord.X, 4) + FString("y") +  *FillString(Coord.Y, 4) + FString("z") + *FillString(Coord.Z, 4));
								UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(MapEditorActor.Get(), ComponentName, RF_Transactional);
								Component->SetStaticMesh(DefaultMesh);
								if (Properties->CustomMaterial.IsValid())
								{
									Component->SetMaterial(0, Properties->CustomMaterial.Get());
								}
								else {
									Component->SetMaterial(0, DefaultMaterial);
								}
								Component->SetWorldLocation(NewLocation);
								Component->RegisterComponent();
								MapEditorActor->AddInstanceComponent(Component);
							}
						}
					}
					GEditor->EndTransaction();
					break;
				}
				case EMapEditorAction::del:
				{
					GEditor->BeginTransaction(FText::FromString("Del Block"));
					MapEditorActor->Modify();
					Result.Component->DestroyComponent();
					GEditor->EndTransaction();
					break;
				}
				case EMapEditorAction::replace:
				{
					if (Properties->CustomMaterial.IsValid())
					{
						Result.Component->SetMaterial(0, Properties->CustomMaterial.Get());
					}
					break;
				}
			}
		}
	}
}

void UBlockEditorTool::OnPropertyModified(UObject* PropertySet, FProperty* Property)
{
	UpdatePreviewMesh();
}

FInputRayHit UBlockEditorTool::BeginHoverSequenceHitTest(const FInputDeviceRay& PressPos)
{
	return FInputRayHit(0.0f);
}

void UBlockEditorTool::OnBeginHover(const FInputDeviceRay& DevicePos)
{
}

bool UBlockEditorTool::OnUpdateHover(const FInputDeviceRay& DevicePos)
{
	if (Properties->bShowPreviewMesh && MapEditorActor.IsValid())
	{
		FVector RayStart = DevicePos.WorldRay.Origin;
		FVector RayEnd = DevicePos.WorldRay.PointAt(999999);
		FCollisionObjectQueryParams QueryParams(FCollisionObjectQueryParams::AllObjects);
		FHitResult Result;
		bool bHitWorld = TargetWorld->LineTraceSingleByObjectType(Result, RayStart, RayEnd, QueryParams);
		if (Result.Actor.IsValid() && Result.Component.IsValid() && Result.Actor->GetUniqueID() == MapEditorActor->GetUniqueID() && bHitWorld)
		{
			switch (CurAction)
			{
				case EMapEditorAction::add:
				{
					FVector NewLocationOffset = (100 * Result.Normal);
					PreviewMesh->SetVisible(true);
					FTransform NewTransform;
					NewTransform.SetLocation(Result.Component->GetComponentLocation() + FVector(0.0f, 0.0, 50.0f) + NewLocationOffset);
					PreviewMesh->SetTransform(NewTransform);
					break;
				}
				case EMapEditorAction::del:
				case EMapEditorAction::replace:
				{
					if (Properties->bWireFrame && Result.Component.IsValid())
					{
						PreviewMesh->SetVisible(true);
						FTransform NewTransform;
						NewTransform.SetLocation(Result.Component->GetComponentLocation() + FVector(0.0f, 0.0, 50.0f));
						PreviewMesh->SetTransform(NewTransform);
					}
					return true;
				}
			}
		}
		else
		{
			PreviewMesh->SetVisible(false);
		}
	}
	else
	{
		PreviewMesh->SetVisible(false);
	}
	
	return true;
}

void UBlockEditorTool::OnEndHover()
{
}

void UBlockEditorTool::GenerateMesh(FDynamicMesh3* OutMesh, FVector3d Size) const
{
	FGridBoxMeshGenerator BoxGen;
	BoxGen.Box = FOrientedBox3d(FVector3d::Zero(), 0.5 * Size);
	BoxGen.Generate();
	OutMesh->Copy(&BoxGen);
}

void UBlockEditorTool::SetWorld(UWorld* World)
{
	TargetWorld = World;
}

void UBlockEditorTool::SetMapEditorActor(TWeakObjectPtr<AMapEditorActor> Actor)
{
	MapEditorActor = Actor;
}

void UBlockEditorTool::SetAction(EMapEditorAction InAction)
{
	CurAction = InAction;
}

void UBlockEditorTool::SetPreviewVisiable(bool InVisiable)
{
	Properties->bShowPreviewMesh = InVisiable;
}

void UBlockEditorTool::UpdatePreviewMesh()
{
	if (Properties->bWireFrame)
	{
		PreviewMesh->SetMaterial(0, PreviewMaterial);
	}
	else
	{
		if (Properties->CustomMaterial.IsValid())
		{
			PreviewMesh->SetMaterial(0, Properties->CustomMaterial.Get());
		}
		else {
			PreviewMesh->SetMaterial(0, DefaultMaterial);
		}
	}

	FDynamicMesh3 NewMesh;
	GenerateMesh(&NewMesh, FVector3d(100.0f, 100.0f, 100.0f));
	PreviewMesh->UpdatePreview(&NewMesh);
}
