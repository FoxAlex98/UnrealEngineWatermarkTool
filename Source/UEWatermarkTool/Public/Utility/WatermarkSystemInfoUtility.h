// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WatermarkSystemInfoUtility.generated.h"

/**
 * 
 */
UCLASS()
class UEWATERMARKTOOL_API UWatermarkSystemInfoUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintPure, Category = "System Info")
	static FString GetUserName();
	
	UFUNCTION(BlueprintPure, Category = "System Info")
	static FString GetMacAddress();
	
	UFUNCTION(BlueprintPure, Category = "System Info")
	static FString GetCPUBrand();
	
	UFUNCTION(BlueprintPure, Category = "System Info")
	static FString GetCPUVendor();

	UFUNCTION(BlueprintPure, Category = "System Info")
	static FString GetPrimaryGPUBrand();

	UFUNCTION(BlueprintPure, Category = "System Info")
	static FString GetGPUDriverVersion();

	UFUNCTION(BlueprintPure, Category = "System Info")
	static FString GetEpicAccountId();

	UFUNCTION(BlueprintPure, Category = "System Info")
	static FString GetDeviceId();

	UFUNCTION(BlueprintPure, Category = "System Info")
	static FString GetRAMInfo();
	
};