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

#pragma region TextureWatermark
	
	static UTexture2D* CreateTransientTextureFromPixels(const TArray<FColor>& Pixels, int32 Width, int32 Height);

	static void ProcessTextureWatermarkEmbedding(UTexture2D* HostTexture, UTexture2D* WatermarkTexture,
	                                    TFunction<void(uint8*, int32, int32, const TArray<FColor>&, int32, int32)> EmbedLogic);

	static UTexture2D* ProcessTextureWatermarkExtraction(UTexture2D* WatermarkedTexture,
		TFunction<void(uint8*, int32, int32, TArray<FColor>&)> ExtractLogic);
	
	UFUNCTION(BlueprintCallable, Category="Watermark Asset|Texture")
	static void EmbedQuantizedTextureWatermark(UTexture2D* HostTexture, UTexture2D* WatermarkTexture);
	UFUNCTION(BlueprintCallable, Category="Watermark Asset|Texture")
	static UTexture2D* ExtractQuantizedTextureWatermark(UTexture2D* WatermarkedTexture);
	
	UFUNCTION(BlueprintCallable, Category="Watermark Asset|Texture")
	static void EmbedTextureWatermarkRGBThreshold(UTexture2D* HostTexture, UTexture2D* WatermarkTexture);
	UFUNCTION(BlueprintCallable, Category="Watermark Asset|Texture")
	static UTexture2D* ExtractTextureWatermarkRGBThreshold(UTexture2D* WatermarkedTexture);

#pragma endregion TextureWatermark
	
#pragma region StaticMeshWatermark
	
	UFUNCTION(BlueprintCallable, Category="Watermark Asset|Static Mesh")
	static void EmbedStaticMeshVertexPatternWatermark(UStaticMesh* StaticMesh, const FString& Seed, const FString& WatermarkPattern, int32 VertexCount);
	
	UFUNCTION(BlueprintCallable, Category="Watermark Asset|Static Mesh")
	static FString ExtractStaticMeshVertexPatternWatermark(UStaticMesh* StaticMesh, const FString& Seed, int32 VertexCount, int32 DecimalDigits);

	UFUNCTION(BlueprintCallable, Category="Watermark Asset|Static Mesh")
	static bool VerifyStaticMeshVertexPatternWatermark(UStaticMesh* StaticMesh, const FString& Seed, const FString& ExpectedPattern, int32 VertexCount, float ConfidenceThreshold);

#pragma endregion StaticMeshWatermark
	
private:

	static bool SaveAsset(UObject* AssetToSave);
	
	static int32 GetSeedFromString(const FString& Seed);
};
