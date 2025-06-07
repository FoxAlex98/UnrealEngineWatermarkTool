// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WatermarkFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class UEWATERMARKTOOL_API UWatermarkFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category = "Watermark Tool")
	static bool ShouldShowWatermarkUI();
	
	UFUNCTION(BlueprintCallable, Category = "Watermark Tool")
	static bool ShouldShowWatermarkGym();

	UFUNCTION(BlueprintCallable, Category = "Watermark Tool", meta=(WorldContext = "WorldContextObject"))
	static void SetGymMpcScalarValue(FName ParamName, float Value, UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Watermark Tool", meta=(WorldContext = "WorldContextObject"))
	static void UpdateWatermarkGymEnableStatus(UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category = "Watermark")
	static void EmbedLSBWatermark(UTexture2D* Texture, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "Watermark")
	static FString ExtractLSBWatermark(UTexture2D* Texture, int32 MessageLength);

	UFUNCTION(BlueprintCallable, Category = "Watermark")
	static void EmbedDCTWatermark(UTexture2D* Texture, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "Watermark")
	static FString ExtractDCTWatermark(UTexture2D* Texture, int32 MessageLength);

	UFUNCTION(BlueprintCallable, Category = "Audio Watermark")
	static void EmbedSpreadSpectrumWatermark(USoundWave* SoundWave, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "Audio Watermark")
	static FString ExtractSpreadSpectrumWatermark(USoundWave* SoundWave, int32 MessageLength);
};
