// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LightWatermarkActor.h"
#include "GameFramework/Actor.h"
#include "LightWatermarkManager.generated.h"

UENUM(BlueprintType)
enum class ELightPlacementMode : uint8
{
	ByCount,
	ByLightSize
};

class UBoxComponent;

UCLASS(Abstract, HideCategories = ("Collision", "Rendering", "Input", "Actor", "LOD", "Cooking", "Replication", "Navigation", "HLOD", "Physics"))
class UEWATERMARKTOOL_API ALightWatermarkManager : public AActor
{
	GENERATED_BODY()

public:
	ALightWatermarkManager();

	virtual void OnConstruction(const FTransform& Transform) override;
	
	UPROPERTY(EditAnywhere, Category = "Watermark|Light Placement")
	ELightPlacementMode PlacementMode = ELightPlacementMode::ByCount;

	UPROPERTY(EditAnywhere, meta=(EditCondition="PlacementMode == ELightPlacementMode::ByCount"))
	int32 NumLightsX = 3;

	UPROPERTY(EditAnywhere, meta=(EditCondition="PlacementMode == ELightPlacementMode::ByCount"))
	int32 NumLightsY = 3;

	UPROPERTY(EditAnywhere, meta=(EditCondition="PlacementMode == ELightPlacementMode::ByLightSize", ClampMin="0.1"))
	float LightSpacingMultiplier = 1.0f;

protected:
	virtual void BeginPlay() override;

	void ClearSpawnedLights();
	void SpawnLights();
	bool ShouldSpawn() const;

	virtual void Destroyed() override;

protected:
	UPROPERTY(VisibleDefaultsOnly, Category="Watermark Light")
	UBoxComponent* BoxComponent;

	UPROPERTY(EditDefaultsOnly, Category="Watermark Light|Config")
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
