// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameVersionFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class UEWATERMARKTOOL_API UGameVersionFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game Version")
	static FString GetGameVersion();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game Version")
	static FString GetChangelistNumber();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game Version")
	static FString GetBuildDate();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game Version")
	static FString GetEngineVersion();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game Version")
	static FString GetBuildUniqueIdentifier(bool bIncludeEngineVersion = true);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game Version")
	static FString GetBuildNumber();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game Version")
	static FString GetBuildMachineID();
    
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game Version")
	static FString GetBuildPlatform();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game Version")
	static FString GetBuildConfiguration();

	static const TMap<FName, TFunction<FString()>>& GetTokenMap()
	{
		static TMap<FName, TFunction<FString()>> TokenMap;
		if (TokenMap.Num() == 0)
		{
			TokenMap.Add("Version", []() { return GetGameVersion(); });
			TokenMap.Add("Changelist", []() { return FString::FromInt(FEngineVersion::Current().GetChangelist()); });
			TokenMap.Add("Date", []() { return GetBuildDate(); });
			TokenMap.Add("Engine", []() { return GetEngineVersion(); });
			TokenMap.Add("Build Number", []() { return GetBuildNumber(); });
			TokenMap.Add("Machine ID", []() { return GetBuildMachineID(); });
			TokenMap.Add("Platform", []() { return GetBuildPlatform(); });
			TokenMap.Add("Configuration", []() { return GetBuildConfiguration(); });
			TokenMap.Add("Unique ID", []() { return GetBuildUniqueIdentifier(); });
		}
		return TokenMap;
	}

	static FString FormatBuildIdFromTemplate(const FString& Template);

};
