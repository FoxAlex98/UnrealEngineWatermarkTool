#pragma once

#include "IDetailCustomization.h"
#include "Config/WatermarkConfig.h"

class FWatermarkDetailsCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	void ShowWatermarkPreview(UWatermarkConfig* Config);
	
	void OpenUserWidgetInEditor(const TSoftClassPtr<UUserWidget> SoftClass);
};
