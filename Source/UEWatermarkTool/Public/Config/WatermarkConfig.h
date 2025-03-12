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

UCLASS(config = Game, defaultconfig)
class UEWATERMARKTOOL_API UWatermarkConfig : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	static const UWatermarkConfig* Get() { return GetDefault<UWatermarkConfig>(); }

	virtual FName GetContainerName() const override { return TEXT("Project"); }
	virtual FName GetSectionName() const override { return TEXT("Watermark Config"); }

	UPROPERTY(Config, EditAnywhere, Category = "UI Watermark", DisplayName="Enable In Build")
	bool bEnableWatermarkUI = false;
	
	UPROPERTY(Config, EditAnywhere, Category = "UI Watermark", DisplayName="Enable In Editor")
	bool bEnableWatermarkUIInEditor = false;
	
	UPROPERTY(Config, EditAnywhere, Category = "UI Watermark", meta = (TitleProperty = "Name"))
	TArray<FWatermarkSlateWidgetData> WatermarkSlateWidgets;
	
	UPROPERTY(Config, EditAnywhere, Category = "Material Watermark", DisplayName="Enable In Build")
	bool bEnableWatermarkGym = false;
	
	UPROPERTY(Config, EditAnywhere, Category = "Material Watermark", DisplayName="Enable In Editor")
	bool bEnableWatermarkGymInEditor = false;
	
	UPROPERTY(Config, EditDefaultsOnly, Category = "Material Watermark",
		DisplayName="Gym Watermark Material Parameter Collection",
		meta=(AllowedClasses="/Script/Engine.MaterialParameterCollection"))
	FSoftObjectPath GymMPC = nullptr;

#if WITH_EDITOR	
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void PostInitProperties() override;
#endif
};
