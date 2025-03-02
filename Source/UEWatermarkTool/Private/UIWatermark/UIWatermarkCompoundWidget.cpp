// Fill out your copyright notice in the Description page of Project Settings.


#include "UIWatermark/UIWatermarkCompoundWidget.h"

#include "Config/WatermarkConfig.h"

void SUIWatermarkCompoundWidget::Construct(const FArguments& InArgs)
{

	FUIWatermarkText WatermarkText = GetDefault<UWatermarkConfig>()->WatermarkText;
	
	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.Padding(WatermarkText.Padding.X, WatermarkText.Padding.Y)
		.VAlign(WatermarkText.VerticalAlignment)
		.HAlign(WatermarkText.HorizontalAlignment)
		[
			SNew(STextBlock)
			.Visibility(WatermarkText.bEnabled ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
			.Font(WatermarkText.GetFontInfo())
			.ColorAndOpacity(WatermarkText.Color)
			.ShadowColorAndOpacity(WatermarkText.ShadowColor)
			.ShadowOffset(WatermarkText.ShadowOffset)
			.Text(TAttribute<FText>(WatermarkText.Text))
		]
	];
}
