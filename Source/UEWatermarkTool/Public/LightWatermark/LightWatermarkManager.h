// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LightWatermarkActor.h"
#include "GameFramework/Actor.h"
#include "LightWatermarkManager.generated.h"

class UBoxComponent;

UCLASS(Abstract, HideCategories = ("Collision", "Rendering", "Input", "Actor", "LOD", "Cooking", "Replication", "Navigation", "HLOD", "Physics"))
class UEWATERMARKTOOL_API ALightWatermarkManager : public AActor
{
	GENERATED_BODY()

public:
	ALightWatermarkManager();

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif

protected:
	virtual void BeginPlay() override;

	void ClearSpawnedLights();
	void SpawnLights();
	bool ShouldSpawn() const;

	virtual void Destroyed() override;

protected:
	UPROPERTY(VisibleAnywhere, Category="Watermark Light")
	UBoxComponent* BoxComponent;

	UPROPERTY(EditAnywhere, Category="Watermark Light|Config")
	TSubclassOf<ALightWatermarkActor> LightClass;

	UPROPERTY(EditAnywhere, Category="Watermark Light|Config")
	FRotator SpawnedLightsRotator = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category="Watermark Light|Config", AdvancedDisplay)
	bool bEnableInEditor = true;

	UPROPERTY(EditAnywhere, Category="Watermark Light|Config", AdvancedDisplay)
	bool bEnableInBuild = false;

	UPROPERTY(Transient)
	TArray<ALightWatermarkActor*> SpawnedLights;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);
#endif
};
