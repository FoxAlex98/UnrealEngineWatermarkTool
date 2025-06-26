#pragma once

#include "UIWatermarkSlateTypes.h"
#include "WatermarkCommonTypes.generated.h"

UENUM(BlueprintType)
enum class EWidgetWatermarkType : uint8
{
	SlateWatermark,
	UserWidgetWatermark
};

UENUM(BlueprintType)
enum class EWatermarkType : uint8
{
	TextWatermark,
	ImageWatermark
};

USTRUCT(BlueprintType)
struct FWatermarkEnableStatus
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Enable In Editor")
	bool bEnableInEditor = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Enable In Build")
	bool bEnableInBuild = false;

	bool ShouldBeEnabled() const
	{
#if WITH_EDITOR
		return bEnableInEditor;
#else
		return bEnableInBuild;
#endif
	}
};

USTRUCT(BlueprintType)
struct FWatermarkSlateWidgetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watermark", DisplayName="Watermark Name")
	FName Name = FName("Watermark Name");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watermark")
	EWatermarkType Type = EWatermarkType::TextWatermark;
		
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watermark")
	bool bIsEnabled = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watermark", meta=(EditCondition = "bIsEnabled && Type == EWatermarkType::TextWatermark", EditConditionHides))
	FUIWatermarkText TextData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watermark", meta=(EditCondition = "bIsEnabled && Type == EWatermarkType::ImageWatermark", EditConditionHides))
	FUIWatermarkImage ImageData;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watermark", meta=(EditCondition = "bIsEnabled", EditConditionHides))
	FUIWatermarkBase CommonData;
};