// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/WatermarkSystemInfoUtility.h"
#include "HAL/PlatformProcess.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Runtime/Core/Public/Misc/Paths.h"
#include "Misc/EngineVersion.h"
#include "RHI.h"
#include "UEWatermarkTool.h"
#include "Engine/Engine.h"
#include "GenericPlatform/GenericPlatformDriver.h"

FString UWatermarkSystemInfoUtility::GetUserName()
{
	return FPlatformProcess::UserName();
}

FString UWatermarkSystemInfoUtility::GetMacAddress()
{
	TArray<uint8> MacAddress = FPlatformMisc::GetMacAddress();

	bool bIsValid = false;
	for (int32 i = 0; i < 6; ++i)
	{
		if (MacAddress[i] != 0)
		{
			bIsValid = true;
			break;
		}
	}

	if (!bIsValid)
	{
		UE_LOG(LogWatermark, Warning, TEXT("UWatermarkSystemInfoUtility::GetMACAddress - MAC address not available or invalid"));
		return TEXT("Unavailable");
	}

	FString MACString = FString::Printf(TEXT("%02X:%02X:%02X:%02X:%02X:%02X"),
		MacAddress[0], MacAddress[1], MacAddress[2],
		MacAddress[3], MacAddress[4], MacAddress[5]);

	UE_LOG(LogWatermark, Log, TEXT("UWatermarkSystemInfoUtility::GetMACAddress - Found MAC: %s"), *MACString);

	return MACString;
}


FString UWatermarkSystemInfoUtility::GetCPUBrand()
{
	return FPlatformMisc::GetCPUBrand();
}

FString UWatermarkSystemInfoUtility::GetCPUVendor()
{
	return FPlatformMisc::GetCPUVendor();
}

FString UWatermarkSystemInfoUtility::GetPrimaryGPUBrand()
{
	return FPlatformMisc::GetPrimaryGPUBrand();
}

FString UWatermarkSystemInfoUtility::GetGPUDriverVersion()
{
	FString DeviceDescription;
	return FPlatformMisc::GetGPUDriverInfo(DeviceDescription).UserDriverVersion;
}

FString UWatermarkSystemInfoUtility::GetEpicAccountId()
{
	return FPlatformMisc::GetEpicAccountId();
}

FString UWatermarkSystemInfoUtility::GetDeviceId()
{
	return FPlatformMisc::GetDeviceId();
}

FString UWatermarkSystemInfoUtility::GetRAMInfo()
{
	const int32 TotalRAM = FPlatformMemory::GetConstants().TotalPhysical / (1024 * 1024); // In MB
	return FString::Printf(TEXT("%d MB"), TotalRAM);
}
