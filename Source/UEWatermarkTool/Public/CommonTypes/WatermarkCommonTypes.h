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

UENUM(BlueprintType)
enum class EScreenshotWatermarkMode : uint8
{
	None,
	TextOverlay,
	ImageOverlay,
	Slate,
	UMG,
	FontRasterOverlay
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