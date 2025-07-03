// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/GameViewportSubsystem.h"
#include "CommonTypes/WatermarkCommonTypes.h"
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

	void OnSeamlessTravelStart(UWorld* World, const FString& URL);
	void OnPostLoadMapWithWorld(UWorld* World);
	void OnPostWorldInitialization(UWorld* World, FWorldInitializationValues InitializationValue);

	void OnScreenshotCaptured(int Width, int Height, const TArray<FColor>& InBitmap);

	//Subsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//Watermark	
	void AddWatermarkToViewport();

	UFUNCTION(BlueprintCallable, Category="Watermark")
	void ForceAddWatermarkToViewport(EWidgetWatermarkType WidgetWatermarkTypeToShow);

	UFUNCTION(BlueprintCallable, Category="Watermark")
	void ForceRemoveWatermarkToViewport(EWidgetWatermarkType WidgetWatermarkTypeToShow);
	
private:
	
	void AddSlateWatermark();
	void AddUMGWatermark();

	void RemoveSlateWatermark();
	void RemoveUMGWatermark();

	void ApplyWatermarkToScreenshot(int32 Width, int32 Height, const TArray<FColor>& InBitmap);

	static UWorld* GetGameWorldContextless();

	UPROPERTY(Transient)
	UWorld* CurrentGameWorldRef;
};