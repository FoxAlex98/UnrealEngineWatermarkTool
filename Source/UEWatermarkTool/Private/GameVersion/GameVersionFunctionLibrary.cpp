// Fill out your copyright notice in the Description page of Project Settings.

#include "GameVersion/GameVersionFunctionLibrary.h"
#include "GameVersion/GameVersion.h"

FString UGameVersionFunctionLibrary::GetGameVersion()
{
	return FString::FromInt(GAME_MAJOR_VERSION) + "." + FString::FromInt(GAME_MINOR_VERSION) + "." + FString::FromInt(GAME_PATCH_VERSION);
}

FString UGameVersionFunctionLibrary::GetChangelistNumber()
{
	return FString(CHANGELIST_NUMBER);
}

FString UGameVersionFunctionLibrary::GetBuildDate()
{
	return FString(GAME_BUILD_DATE);
}

FString UGameVersionFunctionLibrary::GetEngineVersion()
{
	return FString(ENGINE_VERSION);
}

FString UGameVersionFunctionLibrary::GetBuildUniqueIdentifier(bool bIncludeEngineVersion)
{
#if UE_BUILD_SHIPPING
	FString BuildUniqueIdentifier = "#" + GetChangelistNumber();
#else
	FString BuildUniqueIdentifier = GetBuildDate() + "_" + "_#" + GetChangelistNumber();
	const FString EngineVersion = GetEngineVersion();
	if (bIncludeEngineVersion && !EngineVersion.IsEmpty())
	{
		BuildUniqueIdentifier = BuildUniqueIdentifier + " UE_" + EngineVersion;
	}
#endif
	return BuildUniqueIdentifier;
}

FString UGameVersionFunctionLibrary::GetBuildNumber()
{
	return FString(BUILD_NUMBER);
}

FString UGameVersionFunctionLibrary::GetBuildMachineID()
{
	return FString(BUILD_MACHINE_ID);
}
FString UGameVersionFunctionLibrary::GetBuildPlatform()
{
	return FString(BUILD_PLATFORM);
}

FString UGameVersionFunctionLibrary::GetBuildConfiguration()
{
#if UE_BUILD_SHIPPING
	return FString("Shipping")
#endif
#if UE_BUILD_TEST
	return FString("Test");
#endif
#if UE_BUILD_DEVELOPMENT
	return FString("Development");
#endif
}

FString UGameVersionFunctionLibrary::FormatBuildIdFromTemplate(const FString& Template)
{
	FString Result = Template;
	const TMap<FName, TFunction<FString()>>& TokenMap = GetTokenMap();

	for (const TPair<FName, TFunction<FString()>>& Pair : TokenMap)
	{
		const FString Placeholder = FString::Printf(TEXT("{%s}"), *Pair.Key.ToString());
		Result.ReplaceInline(*Placeholder, *Pair.Value(), ESearchCase::IgnoreCase);
	}

	return Result;
}