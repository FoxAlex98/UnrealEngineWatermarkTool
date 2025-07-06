// Fill out your copyright notice in the Description page of Project Settings.


#include "UIWatermark/BuildIdentifierWidgetBase.h"

#include "Utility/WatermarkFunctionLibrary.h"

void UBuildIdentifierWidgetBase::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (IsDesignTime())
	{
		BuildIdentifierText->SetText(GetBuildIdentifierAsText());
	}
}

void UBuildIdentifierWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	BuildIdentifierText->SetText(GetBuildIdentifierAsText());
}

FText UBuildIdentifierWidgetBase::GetBuildIdentifierAsText()
{
	return FText::FromString(UWatermarkFunctionLibrary::GetBuildIdString());
}