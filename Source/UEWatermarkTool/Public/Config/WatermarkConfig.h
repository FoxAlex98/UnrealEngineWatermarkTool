#pragma once

#include "Engine/DeveloperSettings.h"
#include "UIWatermark/UIWatermarkSlate.h"
#include "WatermarkConfig.generated.h"

UENUM(BlueprintType)
enum class EWatermarkType : uint8
{
	TextWatermark,
	ImageWatermark
};

USTRUCT(BlueprintType)
struct FWatermarkSlateWidgetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watermark", DisplayName="Watermark Name")
	FName Name = FName("Watermark Name");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watermark")
	EWatermarkType Type = EWatermarkType::TextWatermark;
		
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watermark")
	bool bIsEnabled = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watermark", meta=(EditCondition = "bIsEnabled && Type == EWatermarkType::TextWatermark", EditConditionHides))
	FUIWatermarkText TextData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watermark", meta=(EditCondition = "bIsEnabled && Type == EWatermarkType::ImageWatermark", EditConditionHides))
	FUIWatermarkImage ImageData;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watermark", meta=(EditCondition = "bIsEnabled", EditConditionHides))
	FUIWatermarkBase CommonData;
	
};

UCLASS(config = Game, defaultconfig, BlueprintType)
class UEWATERMARKTOOL_API UWatermarkConfig : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	static const UWatermarkConfig* Get() { return GetDefault<UWatermarkConfig>(); }

	virtual FName GetContainerName() const override { return TEXT("Project"); }
	virtual FName GetSectionName() const override { return TEXT("Watermark Config"); }

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UI Watermark", DisplayName="Enable In Build")
	bool bEnableWatermarkUI = false;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UI Watermark", DisplayName="Enable In Editor")
	bool bEnableWatermarkUIInEditor = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UI Watermark", DisplayName="Use UMG UI Watermark")
	bool bUseUMGWatermark = false;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UMG UI Watermark", meta=(EditCondition = "bUseUMGWatermark", EditConditionHides))
	TSoftClassPtr<UUserWidget> WatermarkUserWidgetClass;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Slate UI Watermark", meta = (TitleProperty = "Name", EditCondition = "!bUseUMGWatermark", EditConditionHides))
	TArray<FWatermarkSlateWidgetData> WatermarkSlateWidgets;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Material Watermark", DisplayName="Enable In Build")
	bool bEnableWatermarkGym = false;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Material Watermark", DisplayName="Enable In Editor")
	bool bEnableWatermarkGymInEditor = false;
	
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Material Watermark",
		DisplayName="Gym Watermark Material Parameter Collection",
		meta=(AllowedClasses="/Script/Engine.MaterialParameterCollection"))
	FSoftObjectPath GymMPC = nullptr;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Metadata", DisplayName="Automatically Add Metadata On Import Asset")
	bool bAutoAddMetadataOnImport = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Metadata", DisplayName="Folder to exclude",
		meta=(EditCondition="bAutoAddMetadataOnImport", EditConditionHides))
	TArray<FDirectoryPath> FoldersToExclude = {};

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Metadata", DisplayName="Should Add Company Name in metadata",
		meta=(EditCondition="bAutoAddMetadataOnImport", EditConditionHides))
	bool bShouldAddCompanyName = false;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Metadata", DisplayName="Company Name",
		meta=(EditCondition="bAutoAddMetadataOnImport && bShouldAddCompanyName", EditConditionHides))
	FName CompanyName;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Metadata", DisplayName="Additional Metadata",
		meta=(EditCondition="bAutoAddMetadataOnImport", EditConditionHides))
	TMap<FString, FString> AdditionalMetadata = {};

#if WITH_EDITOR	
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void PostInitProperties() override;
#endif
};
