// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "WatermarkConfigEditor.generated.h"

USTRUCT(BlueprintType)
struct FStaticMeshWatermarkInfo
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watermark Asset|Static Mesh")
	FString Seed = FString("Test");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watermark Asset|Static Mesh")
	FString Pattern = FString("12345");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watermark Asset|Static Mesh", meta=(ClampMin="0", UIMin="0"))
	int32 VertexCount = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watermark Asset|Static Mesh", DisplayName="Confidence Threshold (Verify Only)",
		meta=(ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float ConfidenceThreshold = 0.8f;
};

/**
 * 
 */
UCLASS(Config = Game, defaultconfig, BlueprintType)
class UEWATERMARKTOOL_API UWatermarkConfigEditor : public UDeveloperSettings
{
	GENERATED_BODY()

	virtual FName GetContainerName() const override { return TEXT("Project"); }
	virtual FName GetSectionName() const override { return TEXT("Watermark Editor Config"); }

public:
	
	static const UWatermarkConfigEditor* Get() { return GetDefault<UWatermarkConfigEditor>(); }
	
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

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Asset Watermark|Texture")
	TSoftObjectPtr<UTexture2D> AssetWatermarkTexture;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Asset Watermark|Static Mesh")
	FStaticMeshWatermarkInfo AssetWatermarkStaticMeshInfo;
};
