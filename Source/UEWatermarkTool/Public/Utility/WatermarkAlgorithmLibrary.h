// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonTypes/WatermarkCommonTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WatermarkAlgorithmLibrary.generated.h"

/**
 * 
 */
UCLASS()
class UEWATERMARKTOOL_API UWatermarkAlgorithmLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	static void QuantizedLSBEmbed(uint8* HostPixels, int32 HostWidth, int32 HostHeight, const TArray<FColor>& WatermarkColors, int32 TargetWidth, int32 TargetHeight);

	static void QuantizedLSBExtract(uint8* Pixels, int32 Width, int32 Height, TArray<FColor>& OutPixels);

	static void RGBThresholdLSBEmbed(uint8* HostPixels, int32 HostWidth, int32 HostHeight, const TArray<FColor>& WatermarkColors, int32 TargetWidth, int32 TargetHeight);

	static void RGBThresholdLSBExtract(uint8* Pixels, int32 Width, int32 Height, TArray<FColor>& OutPixels);

};
