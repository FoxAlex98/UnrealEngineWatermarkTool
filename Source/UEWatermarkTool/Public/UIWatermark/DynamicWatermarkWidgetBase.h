// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "DynamicWatermarkWidgetBase.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class UEWATERMARKTOOL_API UDynamicWatermarkWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintNativeEvent)
	FText GetWatermarkText();

protected:
	
	UFUNCTION()
	void UpdateWatermarkPosition() const;

	FVector2D GenerateRandomPosition() const;

	UPROPERTY(EditAnywhere, Category = "Watermark|Config")
	float UpdateInterval = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Watermark|Config")
	FMargin ScreenMargin = FMargin(100.f, 100.f, 100.f, 100.f);
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USizeBox* WatermarkSizeBox;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* WatermarkText;

private:
	FTimerHandle TimerHandle_PositionUpdate;

	void SetWatermarkText();
};
