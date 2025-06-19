#include "UEWatermarkToolEditor.h"

#include "WatermarkDetailsCustomization.h"

#define LOCTEXT_NAMESPACE "FUEWatermarkToolEditorModule"

void FUEWatermarkToolEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyEditorModule.RegisterCustomClassLayout(
		"WatermarkConfig",
		FOnGetDetailCustomizationInstance::CreateStatic(&FWatermarkDetailsCustomization::MakeInstance)
	);
}

void FUEWatermarkToolEditorModule::ShutdownModule()
{
    
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FUEWatermarkToolEditorModule, UEWatermarkToolEditor)

DEFINE_LOG_CATEGORY(LogWatermarkEditor);