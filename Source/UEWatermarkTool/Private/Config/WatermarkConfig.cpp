#include "Config/WatermarkConfig.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SWindow.h"
#include "UIWatermark/UIWatermarkCompoundWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "UEWatermarkTool.h"
#include "GameVersion/GameVersionFunctionLibrary.h"

FString UWatermarkConfig::GetBuildIdString() const
{
	FString BuildIdString;
	if (bFormatBuildIdManually)
	{
		BuildIdString = UGameVersionFunctionLibrary::FormatBuildIdFromTemplate(BuildIdFormat);
	}
	else
	{
		BuildIdString = BuildFinalBuildIdWithMap();
	}
	UE_LOG(LogWatermark, Log, TEXT("UWatermarkConfig::GetBuildIdString - New Build ID: %s"), *BuildIdString);

	return BuildIdString;
}

bool UWatermarkConfig::IsUIWatermarkWidgetValid(EWidgetWatermarkType WidgetWatermarkTypeToCheck) const
{
	switch (WidgetWatermarkTypeToCheck)
	{
	case EWidgetWatermarkType::SlateWatermark:
		return !WatermarkSlateWidgets.IsEmpty();
	case EWidgetWatermarkType::UserWidgetWatermark:
		return !WatermarkUserWidgetClass.IsNull();
	default:
		return false;
	}
}

#if WITH_EDITOR

void UWatermarkConfig::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);


	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UWatermarkConfig, EnableLightWatermark)) {
		//UWatermarkFunctionLibrary::UpdateWatermarkGymEnableStatus(this);
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UWatermarkConfig, SelectedBuildIdTokens)
	|| PropertyName == GET_MEMBER_NAME_CHECKED(UWatermarkConfig, bFormatBuildIdManually)
	|| PropertyName == GET_MEMBER_NAME_CHECKED(UWatermarkConfig, BuildIdSeparator)
	|| PropertyName == GET_MEMBER_NAME_CHECKED(UWatermarkConfig, BuildIdFormat))
	{
		BuildIdPreview = GetBuildIdString();
	}

}

void UWatermarkConfig::PostInitProperties()
{
	Super::PostInitProperties();
	
	BuildIdPreview = GetBuildIdString();
}

TWeakPtr<SWindow> UWatermarkConfig::WatermarkPreviewWindow;

void UWatermarkConfig::ShowSlateWatermarkPreview_Internal()
{
	TSharedPtr<SWindow> ExistingWindow = WatermarkPreviewWindow.Pin();

	if (ExistingWindow.IsValid())
	{
		ExistingWindow->SetContent(
			SNew(SBox)
			[
				SNew(SUIWatermarkCompoundWidget)
			]
		);
		ExistingWindow->BringToFront();
		UE_LOG(LogWatermark, Log, TEXT("UWatermarkConfig::ShowSlateWatermarkPreview_Internal - Updated existing window"));
		return;
	}

	TSharedRef<SWindow> NewWindow = SNew(SWindow)
		.Title(FText::FromString("Slate Watermark Preview"))
		.ClientSize(PreviewWindowSize)
		.SupportsMaximize(true)
		.SupportsMinimize(true)
		[
			SNew(SBox)
			[
				SNew(SUIWatermarkCompoundWidget)
			]
		];

	WatermarkPreviewWindow = NewWindow;
	FSlateApplication::Get().AddWindow(NewWindow);

	UE_LOG(LogWatermark, Log, TEXT("UWatermarkConfig::ShowSlateWatermarkPreview_Internal - Created new window"));
}

void UWatermarkConfig::ShowUMGWatermarkPreview_Internal(TSharedRef<SWidget> WidgetSlate)
{
	TSharedPtr<SWindow> ExistingWindow = WatermarkPreviewWindow.Pin();

	if (ExistingWindow.IsValid())
	{
		ExistingWindow->SetContent(
			SNew(SBox)
			[
				WidgetSlate
			]
		);
		ExistingWindow->BringToFront();
		UE_LOG(LogWatermark, Log, TEXT("FWatermarkDetailsCustomization::ShowUMGWatermarkPreview_Internal - Updated existing window"));
		return;
	}

	TSharedRef<SWindow> NewWindow = SNew(SWindow)
		.Title(FText::FromString("UMG Watermark Preview"))
		.ClientSize(PreviewWindowSize)
		.SupportsMaximize(true)
		.SupportsMinimize(true)
		[
			SNew(SBox)
			[
				WidgetSlate
			]
		];

	WatermarkPreviewWindow = NewWindow;
	FSlateApplication::Get().AddWindow(NewWindow);

	UE_LOG(LogWatermark, Log, TEXT("FWatermarkDetailsCustomization::ShowUMGWatermarkPreview_Internal - Created new window"));
}

#endif

FString UWatermarkConfig::BuildFinalBuildIdWithMap() const
{
	const TMap<FName, TFunction<FString()>>& TokenMap = UGameVersionFunctionLibrary::GetTokenMap();
	TArray<FString> Parts;

	for (const FName& Token : SelectedBuildIdTokens)
	{
		if (const TFunction<FString()>* Func = TokenMap.Find(Token))
		{
			Parts.Add((*Func)());
		}
		else
		{
			Parts.Add(Token.ToString());
		}
	}

	return FString::Join(Parts, *BuildIdSeparator);
}

TArray<FName> UWatermarkConfig::GetAvailableBuildIdTokens()
{
	TArray<FName> Keys;
	UGameVersionFunctionLibrary::GetTokenMap().GetKeys(Keys);
	return Keys;
}