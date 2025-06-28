#pragma once

#include "CommonTypes/WatermarkCommonTypes.h"
#include "Engine/DeveloperSettings.h"
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
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UI Watermark", DisplayName="Watermark Z Order", meta=(ClampMin=0))
	int32 WatermarkZOrder = INT32_MAX-10; //Unreal add 10 on add widget to viewport
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UI Watermark")
	FVector2D PreviewWindowSize = FVector2D(1280, 800);
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UI Watermark",
		meta=(EditCondition = "WidgetWatermarkType == EWidgetWatermarkType::UserWidgetWatermark", EditConditionHides))
	TSoftClassPtr<UUserWidget> WatermarkUserWidgetClass;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UI Watermark", meta = (TitleProperty = "Name",
			EditCondition = "WidgetWatermarkType == EWidgetWatermarkType::SlateWatermark", EditConditionHides))
	TArray<FWatermarkSlateWidgetData> WatermarkSlateWidgets;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Light Watermark", DisplayName="Enable Light Watermark", meta=(ShowOnlyInnerProperties))
	FWatermarkEnableStatus EnableLightWatermark;

	UPROPERTY(Config, EditAnywhere, Category="Build ID", meta=(ToolTip="Format build id manually"))
	bool bFormatBuildIdManually = true;
	
	UPROPERTY(Config, EditAnywhere, Category="Build ID", meta=(GetOptions="GetAvailableBuildIdTokens",
		EditCondition="!bFormatBuildIdManually", EditConditionHides))
	TArray<FName> SelectedBuildIdTokens;

	UPROPERTY(Config, EditAnywhere, Category="Build ID", meta=(ToolTip="Text to insert between tokens",
		EditCondition="!bFormatBuildIdManually", EditConditionHides))
	FString BuildIdSeparator = TEXT("-");
	
	UPROPERTY(Config, EditAnywhere, Category="Build ID", meta=(DisplayName="Build ID Format",
		ToolTip="Available Placeholders: \n{Version} \n{Changelist} \n{Date} \n{Engine} \n{Build Number} \n{Machine ID} \n{Platform} \n{Configuration} \n{Unique ID}",
		EditCondition="bFormatBuildIdManually", EditConditionHides))
	FString BuildIdFormat = TEXT("{Version}-{Changelist}@{Date}");

	UPROPERTY(VisibleAnywhere, Transient, Category="Build ID", meta=(DisplayName="Build ID Preview"))
	FString BuildIdPreview;

	/*
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Material Watermark",
		DisplayName="Gym Watermark Material Parameter Collection",
		meta=(AllowedClasses="/Script/Engine.MaterialParameterCollection"))
	FSoftObjectPath GymMPC = nullptr;
	*/

	UPROPERTY(EditAnywhere, config, Category = "ImageOverlay")
	bool bApplyWatermarkInScreenshot;
	
	UPROPERTY(EditAnywhere, config, Category = "ImageOverlay")
	TSoftObjectPtr<UTexture2D> ImageOverlayTexture;

	UFUNCTION()                                
	FString GetBuildIdString() const;

	UFUNCTION()
	bool IsUIWatermarkWidgetValid(EWidgetWatermarkType WidgetWatermarkTypeToCheck) const;

#if WITH_EDITOR
	
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void PostInitProperties() override;

	void ShowSlateWatermarkPreview_Internal();
	void ShowUMGWatermarkPreview_Internal(TSharedRef<SWidget> WidgetSlate);

#endif
	
#if WITH_EDITORONLY_DATA
private:
	static TWeakPtr<SWindow> WatermarkPreviewWindow;
#endif

private:
	
	UFUNCTION()
	static TArray<FName> GetAvailableBuildIdTokens();

	UFUNCTION()
	FString BuildFinalBuildIdWithMap() const;
};
