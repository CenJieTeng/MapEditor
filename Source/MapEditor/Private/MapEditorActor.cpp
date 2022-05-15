// Fill out your copyright notice in the Description page of Project Settings.


#include "MapEditorActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AMapEditorActor::AMapEditorActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BlockIdCount = 0;

	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/StarterContent/Shapes/Shape_Cube.Shape_Cube"));
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMaterial(TEXT("/Game/StarterContent/Materials/M_Concrete_Tiles.M_Concrete_Tiles"));
	if (DefaultMaterial.Object)
	{
		StaticMesh->SetMaterial(0, DefaultMaterial.Object);
	}
}

void AMapEditorActor::PostRegisterAllComponents()
{
	FNavigationSystem::OnActorRegistered(*this);

	for (auto v : this->GetInstanceComponents())
	{
		BlockIdCount = FMath::Max(BlockIdCount, FCString::Atoi(*(v->GetFName().ToString())));
	}
	BlockIdCount += 1;
}

// Called when the game starts or when spawned
void AMapEditorActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMapEditorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

