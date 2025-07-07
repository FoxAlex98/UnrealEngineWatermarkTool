// Fill out your copyright notice in the Description page of Project Settings.

#include "Utility/WatermarkFunctionLibrary.h"
#include "UEWatermarkTool.h"
#include "Blueprint/UserWidget.h"
#include "Config/WatermarkConfig.h"
#include "Config/WatermarkConfigEditor.h"
#include "UIWatermark/UIWatermarkCompoundWidget.h"

bool UWatermarkFunctionLibrary::ShouldShowWatermarkUI()
{
	return UWatermarkConfig::Get()->EnableUIWatermark.ShouldBeEnabled();
}
/*
bool UWatermarkFunctionLibrary::ShouldShowLightWatermark()
{
	return UWatermarkConfig::Get()->EnableLightWatermark.ShouldBeEnabled();
}
*/
FString UWatermarkFunctionLibrary::GetBuildIdString()
{
	return UWatermarkConfig::Get()->GetBuildIdString();
}

UTexture2D* UWatermarkFunctionLibrary::GetDefaultWatermarkTexture()
{
	return UWatermarkConfigEditor::Get()->AssetWatermarkTexture.LoadSynchronous();
}

/*
void UWatermarkFunctionLibrary::SetGymMpcScalarValue(FName ParamName, float Value, UObject* WorldContextObject)
{
	if (!UWatermarkConfig::Get()->GymMPC.IsValid()) return;

	UMaterialParameterCollection* MPC = Cast<UMaterialParameterCollection>(UWatermarkConfig::Get()->GymMPC.ResolveObject());
	UKismetMaterialLibrary::SetScalarParameterValue(WorldContextObject, MPC, ParamName, Value);
}
void UWatermarkFunctionLibrary::UpdateWatermarkGymEnableStatus(UObject* WorldContextObject)
{
	SetGymMpcScalarValue(TEXT("IsEnabled"), ShouldShowLightWatermark() ? 1.0f : 0.0f, WorldContextObject);
}
*/

UUserWidget* UWatermarkFunctionLibrary::CreateWatermarkUserWidgetFromConfig(APlayerController* PC, bool& bHasSucceeded)
{
	bHasSucceeded = false;
	TSoftClassPtr<UUserWidget> WatermarkClass = UWatermarkConfig::Get()->WatermarkUserWidgetClass;

	/*
	if (!WatermarkClass.IsPending())
	{
		UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem:AddUMGWatermark - WatermarkUserWidgetClass is invalid"));
		bHasSucceeded = true;
		return nullptr;
	}
	*/

	UClass* WidgetClass = WatermarkClass.LoadSynchronous();
	if (!WidgetClass)
	{
		UE_LOG(LogWatermark, Error, TEXT("WatermarkSubsystem:AddUMGWatermark - LoadSynchronous() returned nullptr"));
		bHasSucceeded = true;
		return nullptr;
	}

	UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem:AddUMGWatermark - Widget class loaded successfully, creating widget"));

	UUserWidget* WatermarkUserWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
	if (!WatermarkUserWidget)
	{
		UE_LOG(LogWatermark, Error, TEXT("WatermarkSubsystem:AddUMGWatermark - CreateWidget returned nullptr"));
		bHasSucceeded = true;
		return nullptr;
	}

	return WatermarkUserWidget;
}

TSharedPtr<SWidget> UWatermarkFunctionLibrary::CreateWatermarkSlateWidget()
{
	return SNew(SUIWatermarkCompoundWidget);
}
