// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LightWatermarkActor.generated.h"

UCLASS(Abstract, HideCategories = ("Collision", "Rendering", "Input", "Actor", "LOD", "Cooking", "Replication", "Navigation", "HLOD", "Physics"))
class UEWATERMARKTOOL_API ALightWatermarkActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ALightWatermarkActor();

	UFUNCTION(BlueprintCallable, Category="Watermark Light")
	void TryApplyWatermarkMaterial(ULightComponent* Target);

protected:

	UPROPERTY(EditAnywhere, Category = "Watermark Light|Config")
	FComponentReference LightComponentRef;

	UPROPERTY(EditDefaultsOnly, Category="Watermark Light|Config")
	TSoftObjectPtr<UMaterialInterface> WatermarkMaterial;

	virtual void OnConstruction(const FTransform& Transform) override;

private:
	
	UPROPERTY(VisibleDefaultsOnly, Category="Watermark Light")
	USceneComponent* Root;
};
