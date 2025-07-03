// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonTypes/WatermarkCommonTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WatermarkAssetFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class UEWATERMARKTOOL_API UWatermarkAssetFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category="Watermark")
	static FString StringToBitString(const FString& Input);

	static void RenderUserWidgetToBitmap(UUserWidget* Widget, int32 TargetWidth, int32 TargetHeight, TArray<FColor>& OutPixels);
	
	static void RenderSlateWidgetToBitmap(TSharedRef<SWidget> SlateWidget, int32 TargetWidth, int32 TargetHeight, TArray<FColor>& OutPixels);
	
	//Texture Functions

	static bool ValidateWatermarkParameters(const uint8* HostPixels, const int32 HostWidth, const int32 HostHeight,
		const TArray<FColor>& WatermarkColors, const int32 TargetWidth, const int32 TargetHeight, EWatermarkValidationResult& OutResult);
	
	static bool ReadTexturePixels(UTexture2D* Texture, TArray<FColor>& OutPixels, int32& OutWidth, int32& OutHeight);

	static void ResizePixels(const TArray<FColor>& Src, int32 SrcW, int32 SrcH, int32 DestW, int32 DestH, TArray<FColor>& Out);

	static void ApplyImageOverlayWatermark(int32 Width, int32 Height, const TArray<FColor>& InBitmap, UTexture2D* WatermarkTexture);
	
	static FColor AlphaBlend(const FColor& Src, const FColor& Dst);
	
	static void ApplyInvisibleTextureWatermark(int32 Width, int32 Height, const TArray<FColor>& InBitmap, UTexture2D* WatermarkTexture);

};
