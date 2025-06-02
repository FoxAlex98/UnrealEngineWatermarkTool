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

	//Events
	void OnGameStart(UGameInstance* GameInstance);
	void OnGameEnd(UGameInstance* GameInstance);
	void OnLevelChange(ULevel* NewLevel, ULevel* OldLevel, UWorld* World);

	void OnSeamlessTravelStart(UWorld* World, const FString& URL);
	void OnPostWorldCreation(UWorld* World);
	void OnPostWorldInitialization(UWorld* World, FWorldInitializationValues InitializationValue);
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void AddWatermarkToViewport();
	
private:
	
	void AddSlateWatermark();
	void AddUMGWatermark();

	UWorld* GetGameWorldContextless();
};