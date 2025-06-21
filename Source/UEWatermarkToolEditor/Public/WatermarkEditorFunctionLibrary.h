// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WatermarkEditorFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class UEWATERMARKTOOLEDITOR_API UWatermarkEditorFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category="Watermark|Debug")
	static UTexture2D* CreateDebugVisibleWatermarkedTexture(UTexture2D* Host, UTexture2D* Watermark, const FString& PackagePath = "", const FString& AssetName = "", bool bOverwriteOriginal = true);

	UFUNCTION(BlueprintCallable, Category="Watermark|Debug")
	static UTexture2D* BlendTextures(UTexture2D* Base, UTexture2D* Overlay, float Alpha);

	UFUNCTION(BlueprintCallable, Category="Watermark|Debug")
	static bool SaveAsset(UObject* AssetToSave);

	UFUNCTION(BlueprintCallable, Category="Watermark|Debug")
	static void EmbedQuantizedWatermark(UTexture2D* HostTexture, UTexture2D* WatermarkTexture);

	UFUNCTION(BlueprintCallable, Category="Watermark|Debug")
	static UTexture2D* ExtractQuantizedWatermark(UTexture2D* WatermarkedTexture);

private:

	static bool ReadTexturePixels(UTexture2D* Texture, TArray<FColor>& OutPixels, int32& OutWidth, int32& OutHeight);
	static void ResizePixels(const TArray<FColor>& Src, int32 SrcW, int32 SrcH, int32 DestW, int32 DestH, TArray<FColor>& Out);
	static UTexture2D* CreateTransientTextureFromPixels(const TArray<FColor>& Pixels, int32 Width, int32 Height);

};
