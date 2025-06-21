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

	UFUNCTION(BlueprintCallable, Category="Watermark|Debug")
	static UTexture2D* CreateDebugVisibleWatermarkedTexture(UTexture2D* Host, UTexture2D* Watermark, const FString& PackagePath = "", const FString& AssetName = "", bool bOverwriteOriginal = true);

};
