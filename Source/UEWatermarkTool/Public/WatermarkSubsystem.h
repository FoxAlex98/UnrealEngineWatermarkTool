// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/GameViewportSubsystem.h"
#include "WatermarkSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class UEWATERMARKTOOL_API UWatermarkSubsystem : public UGameViewportSubsystem
{
	GENERATED_BODY()

public:
	void OnGameStart(UGameInstance* GameInstance);
	void OnGameEnd(UGameInstance* GameInstance);
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};