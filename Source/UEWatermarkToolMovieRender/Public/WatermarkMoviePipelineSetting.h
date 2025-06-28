// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MoviePipelineRenderPass.h"
#include "MovieRenderPipelineDataTypes.h"
#include "CommonTypes/WatermarkCommonTypes.h"
#include "UObject/SoftObjectPath.h"
#include "WatermarkMoviePipelineSetting.generated.h"

class FWidgetRenderer;
class SVirtualWindow;
class UTextureRenderTarget2D;


/**
 * 
 */
UCLASS(Blueprintable)
class UEWATERMARKTOOLMOVIERENDER_API UWatermarkMoviePipelineSetting : public UMoviePipelineRenderPass
{
	GENERATED_BODY()

protected:
	// UMoviePipelineRenderPass Interface
	virtual void SetupImpl(const MoviePipeline::FMoviePipelineRenderPassInitSettings& InPassInitSettings) override;
	virtual void TeardownImpl() override;
	virtual void GatherOutputPassesImpl(TArray<FMoviePipelinePassIdentifier>& ExpectedRenderPasses) override;
	virtual void RenderSample_GameThreadImpl(const FMoviePipelineRenderPassMetrics& InSampleState) override;
	// ~UMoviePipelineRenderPass Interface

public:
#if WITH_EDITOR
	virtual FText GetDisplayText() const override { return NSLOCTEXT("MovieRenderPipeline", "Watermark Settings", "Watermark"); }
	virtual FText GetCategoryText() const { return NSLOCTEXT("MovieRenderPipeline", "DefaultCategoryName_Text", "Settings"); }
#endif
	virtual bool IsValidOnShots() const override { return false; }
	virtual bool IsValidOnPrimary() const override { return true; }
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget Settings")
	EWidgetWatermarkType WidgetWatermarkType;
	
	/** If true, the Burn In image will be composited into the Final Image pass. Doesn't apply to multi-layer EXR files. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget Settings")
	bool bCompositeOntoFinalImage = true;

private:
	FIntPoint OutputResolution;
	TSharedPtr<FWidgetRenderer> WidgetRenderer;
	TSharedPtr<SVirtualWindow> VirtualWindow;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UUserWidget>> BurnInUserWidgetInstances;
	
	TArray<TSharedPtr<SWidget>> BurnInSlateWidgetInstances;
};