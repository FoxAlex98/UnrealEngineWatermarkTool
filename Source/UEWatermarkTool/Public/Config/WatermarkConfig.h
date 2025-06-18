#pragma once

#include "CommonTypes/WatermarkCommonTypes.h"
#include "Engine/DeveloperSettings.h"
#include "GameVersion/GameVersionFunctionLibrary.h"
#include "WatermarkConfig.generated.h"

UCLASS(Config = Game, defaultconfig, BlueprintType)
class UEWATERMARKTOOL_API UWatermarkConfig : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	static const UWatermarkConfig* Get() { return GetDefault<UWatermarkConfig>(); }

	virtual FName GetContainerName() const override { return TEXT("Project"); }
	virtual FName GetSectionName() const override { return TEXT("Watermark Config"); }

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UI Watermark", DisplayName="Enable UI Watermark", meta=(ShowOnlyInnerProperties))
	FWatermarkEnableStatus EnableUIWatermark;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UI Watermark", DisplayName="Widget UI Watermark")
	EWidgetWatermarkType WidgetWatermarkType = EWidgetWatermarkType::SlateWatermark;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UI Watermark", DisplayName="Watermark Z Order", AdvancedDisplay, meta=(ClampMin=0))
	int32 WatermarkZOrder = INT32_MAX-10; //Unreal add 10 on add widget to viewport
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UMG UI Watermark",
		meta=(EditCondition = "WidgetWatermarkType == EWidgetWatermarkType::UserWidgetWatermark", EditConditionHides))
	TSoftClassPtr<UUserWidget> WatermarkUserWidgetClass;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Slate UI Watermark", meta = (TitleProperty = "Name",
			EditCondition = "WidgetWatermarkType == EWidgetWatermarkType::SlateWatermark", EditConditionHides))
	TArray<FWatermarkSlateWidgetData> WatermarkSlateWidgets;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Light Watermark", DisplayName="Enable Light Watermark", meta=(ShowOnlyInnerProperties))
	FWatermarkEnableStatus EnableLightWatermark;
	
	UPROPERTY(Config, EditAnywhere, Category="Build ID", meta=(GetOptions="GetAvailableBuildIdTokens"))
	TArray<FName> SelectedBuildIdTokens;

	UPROPERTY(Config, EditAnywhere, Category="Build ID", meta=(ToolTip="Text to insert between tokens"))
	FString BuildIdSeparator = TEXT("-");

	UPROPERTY(VisibleAnywhere, Transient, Category="Build ID", meta=(DisplayName="Build ID Preview"))
	FString BuildIdPreview;

	// Build ID Formatter - visible in Project Settings
	UPROPERTY(Config, EditAnywhere, Category="Build ID", meta=(DisplayName="Build ID Format", ToolTip="Use placeholders like {Version}, {Changelist}, {Date}, {Engine}"))
	FString BuildIdFormat = TEXT("{Version}-{Changelist}@{Date}");

	// Build ID Preview - NOT editable
	UPROPERTY(VisibleAnywhere, Transient, Category="Build ID", meta=(DisplayName="Build ID Preview"))
	FString BuildIdFormatPreview;


	/*
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Material Watermark",
		DisplayName="Gym Watermark Material Parameter Collection",
		meta=(AllowedClasses="/Script/Engine.MaterialParameterCollection"))
	FSoftObjectPath GymMPC = nullptr;
	*/
	
	UPROPERTY(EditAnywhere, config, Category = "Watermark")
	EScreenshotWatermarkMode WatermarkMode = EScreenshotWatermarkMode::None;

	UPROPERTY(EditAnywhere, config, Category = "TextOverlay")
	FString OverlayText;

	UPROPERTY(EditAnywhere, config, Category = "TextOverlay")
	FColor BackgroundColor = FColor::Red;

	UPROPERTY(EditAnywhere, config, Category = "ImageOverlay")
	TSoftObjectPtr<UTexture2D> ImageOverlayTexture;

	UPROPERTY(EditAnywhere, config, Category = "FontRasterOverlay")
	FString FontText;

	UPROPERTY(EditAnywhere, Category = "FontRasterOverlay")
	UFont* Font;

#if WITH_EDITOR	
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void PostInitProperties() override;
#endif

	UFUNCTION()
	FString BuildFinalBuildId() const;

	UFUNCTION()
	static TArray<FName> GetAvailableBuildIdTokens();

};