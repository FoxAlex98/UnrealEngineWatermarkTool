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

	UFUNCTION(BlueprintPure, Category="Watermark")
	static FString StringToBitString(const FString& Input);

	static void RenderUserWidgetToBitmap(UUserWidget* Widget, int32 TargetWidth, int32 TargetHeight, TArray<FColor>& OutPixels);
	
	static void RenderSlateWidgetToBitmap(TSharedRef<SWidget> SlateWidget, int32 TargetWidth, int32 TargetHeight, TArray<FColor>& OutPixels);
	
};
