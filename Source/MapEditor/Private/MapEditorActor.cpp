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

	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/StarterContent/Shapes/Shape_Cube.Shape_Cube"));
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("x0000y0000z0000"));
	StaticMesh->SetCustomPrimitiveDataVector3(0, FVector::ZeroVector);
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

	SetFlags(EObjectFlags::RF_Transactional);
}

void AMapEditorActor::PostRegisterAllComponents()
{
	FNavigationSystem::OnActorRegistered(*this);

	for (auto v : this->GetComponents())
	{
		UStaticMeshComponent* Component = Cast<UStaticMeshComponent>(v);
		if (Component == nullptr)
			continue;
		FVector Coord = Component->GetRelativeLocation() / 100;
		Coord = FVector(FMath::RoundToInt(Coord.X), FMath::RoundToInt(Coord.Y), FMath::RoundToInt(Coord.Z));
		Component->SetCustomPrimitiveDataVector3(0, Coord);
	}
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

