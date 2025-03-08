#pragma once

#include "Engine/DeveloperSettings.h"
#include "WatermarkConfig.generated.h"

USTRUCT(BlueprintType)
struct FUIWatermarkText
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category="UI Watermark")
	bool bEnabled;

	UPROPERTY(EditAnywhere, Category="UI Watermark", meta = (MultiLine = "true"))
	FText Text;

	UPROPERTY(EditAnywhere, Category="UI Watermark")
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment;

	UPROPERTY(EditAnywhere, Category="UI Watermark")
	TEnumAsByte<EVerticalAlignment> VerticalAlignment;

	UPROPERTY(EditAnywhere, Category="UI Watermark")
	FIntPoint Padding;

	UPROPERTY(EditAnywhere, Category="UI Watermark")
	FLinearColor Color;

	UPROPERTY(EditAnywhere, Category="UI Watermark")
	FLinearColor ShadowColor;

	UPROPERTY(EditAnywhere, Category="UI Watermark")
	FVector2D ShadowOffset;

private:
	UPROPERTY(EditAnywhere, Category="UI Watermark")
	FSlateFontInfo FontInfo;

public:

	FORCEINLINE void SetFontInfo(const FSlateFontInfo& NewFontInfo)
	{
		FontInfo = NewFontInfo;
	}

	FSlateFontInfo GetFontInfo() const
	{
		if (!IsValid(FontInfo.FontObject))
		{
			return FCoreStyle::GetDefaultFontStyle("BoldCondensed", FontInfo.Size);
		}

		return FontInfo;
	}

	
	FUIWatermarkText() : HorizontalAlignment(HAlign_Center), VerticalAlignment(VAlign_Center)
	{
		bEnabled = true;
		Padding = FIntPoint(10, 10);
		Color = FLinearColor(0.8, 0.8f, 0.8f, 0.2f);
		ShadowColor = FLinearColor(0.f, 0.f, 0.f, 0.0);
		ShadowOffset = FVector2D::ZeroVector;
	}
};

UCLASS(config = Game, defaultconfig)
class UEWATERMARKTOOL_API UWatermarkConfig : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	static const UWatermarkConfig* Get() { return GetDefault<UWatermarkConfig>(); }

	virtual FName GetContainerName() const override { return TEXT("Project"); }
	virtual FName GetSectionName() const override { return TEXT("Watermark Config"); }

	UPROPERTY(Config, EditAnywhere, Category = "UI Watermark")
	bool bEnableWatermarkUI = false;
	
	UPROPERTY(Config, EditAnywhere, Category = "UI Watermark")
	bool bEnableWatermarkUIInEngine = false;
	
	UPROPERTY(Config, EditAnywhere, Category = "UI Watermark")
	TArray<FUIWatermarkText> WatermarkTexts;

	//TODO: add image
	//TODO: try to add multiple text
};