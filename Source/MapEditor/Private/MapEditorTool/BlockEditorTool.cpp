// Fill out your copyright notice in the Description page of Project Settings.


#include "MapEditorTool/BlockEditorTool.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/ConstructorHelpers.h"
#include "InteractiveToolManager.h"

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

UStaticMesh* UBlockEditorTool::DefaultMesh = nullptr;
UMaterialInterface* UBlockEditorTool::DefaultMaterial = nullptr;

UBlockEditorTool::UBlockEditorTool()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> FindMesh(TEXT("/Game/StarterContent/Shapes/Shape_Cube.Shape_Cube"));
	DefaultMesh = FindMesh.Object;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> FindMaterial(TEXT("/Game/StarterContent/Materials/M_Concrete_Tiles.M_Concrete_Tiles"));
	DefaultMaterial = FindMaterial.Object;
}

void UBlockEditorTool::Setup()
{
	USingleClickTool::Setup();

	//Properties = NewObject<UBlockEditorToolProperties>();
	//AddToolPropertySource(Properties);

	MapEditorActor = nullptr;
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
		static int count = 1;
		FName ComponentName = FName(FString::FromInt(count));
		
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
					count += 1;
				}
			}
		}
	}
}
