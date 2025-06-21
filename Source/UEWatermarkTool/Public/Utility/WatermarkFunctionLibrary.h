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
	static bool ShouldShowLightWatermark();
	
	UFUNCTION(BlueprintCallable, Category = "Watermark Tool")
	static FString GetBuildIdString();
	
	UFUNCTION(BlueprintCallable, Category = "Watermark Tool")
	static UTexture2D* GetDefaultWatermarkTexture();
/*
	UFUNCTION(BlueprintCallable, Category = "Watermark Tool", meta=(WorldContext = "WorldContextObject"))
	static void SetGymMpcScalarValue(FName ParamName, float Value, UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Watermark Tool", meta=(WorldContext = "WorldContextObject"))
	static void UpdateWatermarkGymEnableStatus(UObject* WorldContextObject);
*/

	UFUNCTION(BlueprintPure, Category = "Watermark")
	static UUserWidget* CreateWatermarkUserWidgetFromConfig(APlayerController* PC, bool& bHasSucceeded);

};
