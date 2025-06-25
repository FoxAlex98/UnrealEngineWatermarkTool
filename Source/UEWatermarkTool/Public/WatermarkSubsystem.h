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
	static FColor AlphaBlend(const FColor& Src, const FColor& Dst);

	void OnSeamlessTravelStart(UWorld* World, const FString& URL);
	void OnPostWorldCreation(UWorld* World);
	void OnPostLoadMapWithWorld(UWorld* World);
	void OnPostWorldInitialization(UWorld* World, FWorldInitializationValues InitializationValue);

	void OnScreenshotCaptured(int Width, int Height, const TArray<FColor>& InBitmap);
	static void OnViewportResizedEvent(FViewport* Viewport, unsigned I);
	static void OnScreenshotRequestProcessed();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void AddWatermarkToViewport();
	
private:
	
	void AddSlateWatermark();
	void AddUMGWatermark();

	static FString FindLatestScreenshot();

	UWorld* GetGameWorldContextless();

	static void ApplyImageOverlayWatermark(int32 Width, int32 Height, const TArray<FColor>& InBitmap, UTexture2D* WatermarkTexture);

	UPROPERTY(Transient)
	UWorld* CurrentGameWorldRef;
};