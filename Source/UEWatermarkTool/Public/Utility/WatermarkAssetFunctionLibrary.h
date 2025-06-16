// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
/*
	UFUNCTION(BlueprintCallable, Category="Watermark")
	static UTexture2D* CreateBitmaskTexture(const FString& BitString);
*/
	UFUNCTION(BlueprintPure, Category="Watermark")
	static FString StringToBitString(const FString& Input);

	static void RenderUserWidgetToBitmap(UUserWidget* Widget, int32 TargetWidth, int32 TargetHeight, TArray<FColor>& OutPixels);
	
	static void RenderSlateWidgetToBitmap(TSharedRef<SWidget> SlateWidget, int32 TargetWidth, int32 TargetHeight, TArray<FColor>& OutPixels);
	
	UFUNCTION(BlueprintCallable, Category = "Watermark|RenderTarget")
	static void EmbedLSBOnRenderTarget(UTextureRenderTarget2D* RenderTarget, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "Watermark|RenderTarget")
	static FString ExtractLSBFromRenderTarget(UTextureRenderTarget2D* RenderTarget, int32 MessageLength);

	UFUNCTION(BlueprintCallable, Category="Watermark|StaticMesh")
	static void EmbedWatermarkInStaticMesh(UStaticMesh* StaticMesh, const FString& NumericPattern);

	UFUNCTION(BlueprintCallable, Category="Watermark|StaticMesh")
	static FString ExtractWatermarkFromStaticMesh(UStaticMesh* StaticMesh, int32 DecimalDigits);
};
