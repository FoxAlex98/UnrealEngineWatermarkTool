// Fill out your copyright notice in the Description page of Project Settings.


#include "WatermarkFunctionLibrary.h"
#include "Config/WatermarkConfig.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

bool UWatermarkFunctionLibrary::ShouldShowWatermarkUI()
{
#if WITH_EDITOR
	return UWatermarkConfig::Get()->bEnableWatermarkUIInEditor;
#else
	return UWatermarkConfig::Get()->bEnableWatermarkUI;
#endif
}

bool UWatermarkFunctionLibrary::ShouldShowWatermarkGym()
{
#if WITH_EDITOR
	return UWatermarkConfig::Get()->bEnableWatermarkUIInEditor;
#else
	return UWatermarkConfig::Get()->bEnableWatermarkUI;
#endif
}

void UWatermarkFunctionLibrary::SetGymMpcScalarValue(FName ParamName, float Value, UObject* WorldContextObject)
{
	if (!UWatermarkConfig::Get()->GymMPC.IsValid()) return;

	UMaterialParameterCollection* MPC = Cast<UMaterialParameterCollection>(UWatermarkConfig::Get()->GymMPC.ResolveObject());
	UKismetMaterialLibrary::SetScalarParameterValue(WorldContextObject, MPC, ParamName, Value);
}

void UWatermarkFunctionLibrary::UpdateWatermarkGymEnableStatus(UObject* WorldContextObject)
{
	SetGymMpcScalarValue(TEXT("IsEnabled"), ShouldShowWatermarkGym() ? 1.0f : 0.0f, WorldContextObject);
}
