#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "UIWatermarkSlateTypes.generated.h"

USTRUCT(BlueprintType)
struct FUIWatermarkBase
{
	GENERATED_USTRUCT_BODY()

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

	FUIWatermarkBase()
		: HorizontalAlignment(HAlign_Center),
		  VerticalAlignment(VAlign_Center),
		  Padding(FIntPoint(10, 10)),
		  Color(FLinearColor(1.0, 1.0f, 1.0f, 1.0f)),
		  ShadowColor(FLinearColor(0.f, 0.f, 0.f, 0.0)),
		  ShadowOffset(FVector2D::ZeroVector)
	{
	}
};

USTRUCT(BlueprintType)
struct FUIWatermarkText
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category="UI Watermark | Text", meta = (MultiLine = "true"))
	FText Text;

	UPROPERTY(EditAnywhere, Category="UI Watermark | Text")
	ETextTransformPolicy TransformPolicy = ETextTransformPolicy::None;
	
private:
	UPROPERTY(EditAnywhere, Category="UI Watermark | Text")
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
};

USTRUCT(BlueprintType)
struct FUIWatermarkImage
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category="UI Watermark | Image")
	UTexture2D* Image;

	UPROPERTY(EditAnywhere, Category="UI Watermark | Image")
	FVector2D ImageSize;

	FUIWatermarkImage()
		: Image(nullptr),
		  ImageSize(FVector2D(100.f, 100.f))
	{
	}
	
	FORCEINLINE bool HasValidImage() const
	{
		return Image != nullptr;
	}
};

