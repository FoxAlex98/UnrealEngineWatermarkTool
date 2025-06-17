// Fill out your copyright notice in the Description page of Project Settings.


#include "LightWatermark/LightWatermarkActor.h"
#include "Components/PrimitiveComponent.h"
#include "UEWatermarkTool.h"
#include "Components/LightComponent.h"

ALightWatermarkActor::ALightWatermarkActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void ALightWatermarkActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ULightComponent* LightComponent = Cast<ULightComponent>(LightComponentRef.GetComponent(this));
	if (LightComponent)
	{
		UE_LOG(LogWatermark, Log, TEXT("AWatermarkLightActor::OnConstruction - Found valid light component %s"), *LightComponent->GetName());
		TryApplyWatermarkMaterial(LightComponent);
	}
	else
	{
		UE_LOG(LogWatermark, Warning, TEXT("AWatermarkLightActor::OnConstruction - No valid light component found"));
	}
	UE_LOG(LogWatermark, Verbose, TEXT("ALightWatermarkActor::OnConstruction - Ready"));
}

void ALightWatermarkActor::TryApplyWatermarkMaterial(ULightComponent* Target)
{
	if (!IsValid(Target))
	{
		UE_LOG(LogWatermark, Log, TEXT("ALightWatermarkActor::TryApplyWatermarkMaterial - Target not valid"));
		return;
	}
		
	UMaterialInterface* Material = WatermarkMaterial.IsValid() ? WatermarkMaterial.Get() : WatermarkMaterial.LoadSynchronous();
	if (!Material)
	{
		UE_LOG(LogWatermark, Log, TEXT("ALightWatermarkActor::TryApplyWatermarkMaterial - Material not valid"));
		return;
	}

	Target->SetMaterial(0, Material);

	UE_LOG(LogWatermark, Log, TEXT("ALightWatermarkActor::TryApplyWatermarkMaterial - Applied to %s"), *Target->GetName());
}
