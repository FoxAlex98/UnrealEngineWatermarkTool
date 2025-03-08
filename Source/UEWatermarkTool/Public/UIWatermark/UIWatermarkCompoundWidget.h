// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class SUIWatermarkCompoundWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUIWatermarkCompoundWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
