// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/WatermarkSystemInfoUtility.h"
#include "HAL/PlatformProcess.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Runtime/Core/Public/Misc/Paths.h"
#include "Misc/EngineVersion.h"
#include "RHI.h"
#include "Engine/Engine.h"
#include "GenericPlatform/GenericPlatformDriver.h"

FString UWatermarkSystemInfoUtility::GetUserName()
{
	return FPlatformProcess::UserName();
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
