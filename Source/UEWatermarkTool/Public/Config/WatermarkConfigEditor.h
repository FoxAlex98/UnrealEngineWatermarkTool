// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "WatermarkConfigEditor.generated.h"

/**
 * 
 */
UCLASS(Config = Game, defaultconfig, BlueprintType)
class UEWATERMARKTOOL_API UWatermarkConfigEditor : public UDeveloperSettings
{
	GENERATED_BODY()

	static const UWatermarkConfigEditor* Get() { return GetDefault<UWatermarkConfigEditor>(); }

	virtual FName GetContainerName() const override { return TEXT("Project"); }
	virtual FName GetSectionName() const override { return TEXT("Watermark Editor Config"); }

public:
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Metadata", DisplayName="Automatically Add Metadata On Import Asset")
	bool bAutoAddMetadataOnImport = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Metadata", DisplayName="Folder to exclude",
		meta=(EditCondition="bAutoAddMetadataOnImport"))
	TArray<FDirectoryPath> FoldersToExclude = {};

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Metadata", DisplayName="Should Add Company Name in metadata",
		meta=(EditCondition="bAutoAddMetadataOnImport"))
	bool bShouldAddCompanyName = false;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Metadata", DisplayName="Company Name",
		meta=(EditCondition="bAutoAddMetadataOnImport && bShouldAddCompanyName"))
	FName CompanyName;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Metadata", DisplayName="Additional Metadata",
		meta=(EditCondition="bAutoAddMetadataOnImport"))
	TMap<FString, FString> AdditionalMetadata = {};

};
