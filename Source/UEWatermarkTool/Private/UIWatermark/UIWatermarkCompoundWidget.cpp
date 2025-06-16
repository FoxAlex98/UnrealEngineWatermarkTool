// Fill out your copyright notice in the Description page of Project Settings.

#include "UIWatermark/UIWatermarkCompoundWidget.h"
#include "Config/WatermarkConfig.h"

void SUIWatermarkCompoundWidget::Construct(const FArguments& InArgs)
{
	const TArray<FWatermarkSlateWidgetData>& WatermarkWidgets = UWatermarkConfig::Get()->WatermarkSlateWidgets;
    
	TSharedPtr<SOverlay> OverlayWidget;
    
	ChildSlot
	[
		SAssignNew(OverlayWidget, SOverlay)
	];
    
	for (const FWatermarkSlateWidgetData& WatermarkData : WatermarkWidgets)
	{
		if (WatermarkData.Type == EWatermarkType::TextWatermark)
		{
			OverlayWidget->AddSlot()
			.Padding(WatermarkData.CommonData.Padding.X, WatermarkData.CommonData.Padding.Y)
			.VAlign(WatermarkData.CommonData.VerticalAlignment)
			.HAlign(WatermarkData.CommonData.HorizontalAlignment)
			[
				SNew(STextBlock)
				.Visibility(WatermarkData.bIsEnabled ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
				.Font(WatermarkData.TextData.GetFontInfo())
				.ColorAndOpacity(WatermarkData.CommonData.Color)
				.ShadowColorAndOpacity(WatermarkData.CommonData.ShadowColor)
				.ShadowOffset(WatermarkData.CommonData.ShadowOffset)
				.Text(WatermarkData.TextData.Text)
				.TransformPolicy(WatermarkData.TextData.TransformPolicy)
			];
		}
		else if (WatermarkData.Type == EWatermarkType::ImageWatermark)
		{
			if (WatermarkData.ImageData.HasValidImage())
			{
				OverlayWidget->AddSlot()
				.Padding(WatermarkData.CommonData.Padding.X, WatermarkData.CommonData.Padding.Y)
				.VAlign(WatermarkData.CommonData.VerticalAlignment)
				.HAlign(WatermarkData.CommonData.HorizontalAlignment)
				[
					SNew(SImage)
					.Visibility(WatermarkData.bIsEnabled ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
					.Image(new FSlateImageBrush(
						WatermarkData.ImageData.Image,
						WatermarkData.ImageData.ImageSize,
						WatermarkData.CommonData.Color))
				];
			}
		}
	}
}
