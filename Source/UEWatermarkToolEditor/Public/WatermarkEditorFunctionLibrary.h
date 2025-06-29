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

	UFUNCTION(BlueprintCallable, Category="Watermark Asset|General")
	static bool SaveAsset(UObject* AssetToSave);

#pragma region TextureWatermark
	
	UFUNCTION(BlueprintCallable, Category="Watermark Asset|Texture")
	static UTexture2D* CreateDebugVisibleWatermarkedTexture(UTexture2D* Host, UTexture2D* Watermark, const FString& PackagePath = "", const FString& AssetName = "", bool bOverwriteOriginal = true);

	UFUNCTION(BlueprintCallable, Category="Watermark Asset|Texture")
	static UTexture2D* BlendTextures(UTexture2D* Base, UTexture2D* Overlay, float Alpha);

	UFUNCTION(BlueprintCallable, Category="Watermark Asset|Texture")
	static void EmbedQuantizedWatermark(UTexture2D* HostTexture, UTexture2D* WatermarkTexture);

	UFUNCTION(BlueprintCallable, Category="Watermark Asset|Texture")
	static UTexture2D* ExtractQuantizedWatermark(UTexture2D* WatermarkedTexture);

	UFUNCTION(BlueprintCallable, Category="Watermark Asset|Texture")
	static void EmbedTextureWatermarkWithRGBThresholdBit(UTexture2D* HostTexture, UTexture2D* WatermarkTexture);

	UFUNCTION(BlueprintCallable, Category="Watermark Asset|Texture")
	static UTexture2D* ExtractTextureWatermarkUsingRGBThresholdBit(UTexture2D* WatermarkedTexture);

#pragma endregion TextureWatermark
	
#pragma region StaticMeshWatermark
	
	UFUNCTION(BlueprintCallable, Category="Watermark Asset|Static Mesh")
	static void EmbedWatermarkDecimal(UStaticMesh* StaticMesh, const FString& Seed, const FString& WatermarkPattern, int32 VertexCount);
	
	UFUNCTION(BlueprintCallable, Category="Watermark Asset|Static Mesh")
	static FString ExtractWatermarkDecimal(UStaticMesh* StaticMesh, const FString& Seed, int32 VertexCount, int32 DecimalDigits);

	UFUNCTION(BlueprintCallable, Category="Watermark Asset|Static Mesh")
	static bool VerifyWatermarkDecimal(UStaticMesh* StaticMesh, const FString& Seed, const FString& ExpectedPattern, int32 VertexCount, float ConfidenceThreshold);

#pragma endregion StaticMeshWatermark
	
private:

	static bool ReadTexturePixels(UTexture2D* Texture, TArray<FColor>& OutPixels, int32& OutWidth, int32& OutHeight);
	static void ResizePixels(const TArray<FColor>& Src, int32 SrcW, int32 SrcH, int32 DestW, int32 DestH, TArray<FColor>& Out);
	static UTexture2D* CreateTransientTextureFromPixels(const TArray<FColor>& Pixels, int32 Width, int32 Height);
	static int32 GetSeedFromString(const FString& Seed);
};
