// Fill out your copyright notice in the Description page of Project Settings.


#include "UIWatermark/UIWatermarkCompoundWidget.h"

#include "Config/WatermarkConfig.h"

void SUIWatermarkCompoundWidget::Construct(const FArguments& InArgs)
{
	const TArray<FUIWatermarkText>& WatermarkTexts = GetDefault<UWatermarkConfig>()->WatermarkTexts;
    
	TSharedPtr<SOverlay> OverlayWidget;
    
	ChildSlot
	[
		SAssignNew(OverlayWidget, SOverlay)
	];
    
	for (const FUIWatermarkText& WatermarkText : WatermarkTexts)
	{
		OverlayWidget->AddSlot()
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
		];
	}
}
