// Fill out your copyright notice in the Description page of Project Settings.

#include "UIWatermark/DynamicWatermarkWidgetBase.h"
#include "Components/CanvasPanelSlot.h"
#include "TimerManager.h"
#include "UEWatermarkTool.h"
#include "Utility/WatermarkSystemInfoUtility.h"

void UDynamicWatermarkWidgetBase::SetWatermarkText()
{
	if (IsValid(WatermarkText))
	{
		WatermarkText->SetText(GetWatermarkText());
	}
}

void UDynamicWatermarkWidgetBase::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (IsDesignTime())
	{
		SetWatermarkText();
	}
}

void UDynamicWatermarkWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	UE_LOG(LogWatermark, Log, TEXT("UDynamicWatermarkWidgetBase::NativeConstruct - Starting watermark timer"));

	SetWatermarkText();

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_PositionUpdate,
		this,
		&UDynamicWatermarkWidgetBase::UpdateWatermarkPosition,
		UpdateInterval,
		true
	);

	UpdateWatermarkPosition();
}

void UDynamicWatermarkWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_PositionUpdate);
}

FText UDynamicWatermarkWidgetBase::GetWatermarkText_Implementation()
{
	return FText::FromString(UWatermarkSystemInfoUtility::GetUserName());
}

void UDynamicWatermarkWidgetBase::UpdateWatermarkPosition() const
{
	if (!WatermarkSizeBox)
	{
		UE_LOG(LogWatermark, Warning, TEXT("UDynamicWatermarkWidgetBase::UpdateWatermarkPosition - WatermarkCanvasPanel is null"));
		return;
	}

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(WatermarkSizeBox->Slot))
	{
		FVector2D NewPosition = GenerateRandomPosition();
		CanvasSlot->SetPosition(NewPosition);

		UE_LOG(LogWatermark, Log, TEXT("UDynamicWatermarkWidgetBase::UpdateWatermarkPosition - Moved to %s"), *NewPosition.ToString());
	}
}

FVector2D UDynamicWatermarkWidgetBase::GenerateRandomPosition() const
{
	FVector2D ViewportSize;
	if (!GEngine || !GEngine->GameViewport)
	{
		UE_LOG(LogWatermark, Log, TEXT("UDynamicWatermarkWidgetBase::UpdateWatermarkPosition - GEngine or GameViewport are not valid"));
		return FVector2D::ZeroVector;
	}
	GEngine->GameViewport->GetViewportSize(ViewportSize);

	FVector2D WidgetSize = FVector2D(0.f, 0.f);

	if (WatermarkSizeBox && WatermarkSizeBox->IsVisible())
	{
		WidgetSize = FVector2D(WatermarkSizeBox->GetWidthOverride(), WatermarkSizeBox->GetHeightOverride());
	}

	float MinX = ScreenMargin.Left;
	float MaxX = ViewportSize.X - ScreenMargin.Right - WidgetSize.X;

	float MinY = ScreenMargin.Top;
	float MaxY = ViewportSize.Y - ScreenMargin.Bottom - WidgetSize.Y;

	MaxX = FMath::Max(MinX, MaxX);
	MaxY = FMath::Max(MinY, MaxY);

	float X = FMath::FRandRange(MinX, MaxX);
	float Y = FMath::FRandRange(MinY, MaxY);

	UE_LOG(LogWatermark, Log, TEXT("UWatermarkWidget::GenerateRandomPosition - WidgetSize: %s, Viewport: %s"),
	*WidgetSize.ToString(), *ViewportSize.ToString());

	return FVector2D(X, Y);
}
