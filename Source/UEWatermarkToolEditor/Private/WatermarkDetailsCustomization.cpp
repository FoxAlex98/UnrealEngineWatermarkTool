#include "WatermarkDetailsCustomization.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "UEWatermarkToolEditor.h"
#include "Blueprint/UserWidget.h"
#include "Config/WatermarkConfig.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

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
		{
			UClass* WatermarkWidgetClass = UWatermarkConfig::Get()->WatermarkUserWidgetClass.LoadSynchronous();
			UUserWidget* WatermarkUserWidget = CreateWidget<UUserWidget>(GEditor->GetEditorWorldContext().World(), WatermarkWidgetClass);
			Config->ShowUMGWatermarkPreview_Internal(WatermarkUserWidget->TakeWidget());
		}
		break;
	default:
		UE_LOG(LogWatermarkEditor, Warning, TEXT("FWatermarkDetailsCustomization::ShowWatermarkPreview - Unknown watermark type"));
		break;
	}
	
}

#undef LOCTEXT_NAMESPACE