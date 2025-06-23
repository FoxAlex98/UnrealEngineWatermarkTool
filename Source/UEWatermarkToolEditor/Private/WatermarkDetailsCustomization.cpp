#include "WatermarkDetailsCustomization.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "UEWatermarkToolEditor.h"
#include "Blueprint/UserWidget.h"
#include "Config/WatermarkConfig.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "FWatermarkDetailsCustomization"

TSharedRef<IDetailCustomization> FWatermarkDetailsCustomization::MakeInstance()
{
	return MakeShareable(new FWatermarkDetailsCustomization);
}

void FWatermarkDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);

	if (Objects.Num() == 1)
	{
		UWatermarkConfig* Settings = Cast<UWatermarkConfig>(Objects[0].Get());
		if (Settings)
		{
			IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("UI Watermark");
			Category.AddCustomRow(LOCTEXT("UIWatermark", "UI Watermark"))
			.ValueContent()
			[
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .Padding(2.0f)
                .AutoWidth()
                [
                    SNew(SButton)
                    .Text(LOCTEXT("ShowPreview", "Show Preview"))
	                .ToolTipText(LOCTEXT("ShowPreviewTooltip", "Show a Preview Of Watermark Widget"))
                    .OnClicked_Lambda([this, Settings]()
                    {
                        ShowWatermarkPreview(Settings);
                        return FReply::Handled();
                    })
                ]
			];
		}
	}
}

void FWatermarkDetailsCustomization::ShowWatermarkPreview(UWatermarkConfig* Config)
{
	switch (Config->WidgetWatermarkType)
	{
	case EWidgetWatermarkType::SlateWatermark:
		Config->ShowSlateWatermarkPreview_Internal();
		break;
	case EWidgetWatermarkType::UserWidgetWatermark:
		OpenUserWidgetInEditor(Config->WatermarkUserWidgetClass);
		break;
	default:
		UE_LOG(LogWatermarkEditor, Warning, TEXT("FWatermarkDetailsCustomization::ShowWatermarkPreview - Unknown watermark type"));
		break;
	}
	
}

void FWatermarkDetailsCustomization::OpenUserWidgetInEditor(const TSoftClassPtr<UUserWidget> SoftClass)
{
	if (!SoftClass.IsValid())
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("FWatermarkDetailsCustomization::OpenUserWidgetInEditor - SoftClass not valid or null"));
		return;
	}

	UClass* WidgetClass = SoftClass.LoadSynchronous();
	if (!WidgetClass)
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("FWatermarkDetailsCustomization::OpenUserWidgetInEditor - LoadSynchronous returned nullptr"));
		return;
	}

	UBlueprint* BP = Cast<UBlueprint>(WidgetClass->ClassGeneratedBy);
	if (!BP)
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("FWatermarkDetailsCustomization::OpenUserWidgetInEditor - ClassGeneratedBy is not a valid Blueprint"));
		return;
	}

	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(BP);
	UE_LOG(LogWatermarkEditor, Log, TEXT("FWatermarkDetailsCustomization::OpenUserWidgetInEditor - Open Editor For %s"), *BP->GetName());
}

#undef LOCTEXT_NAMESPACE