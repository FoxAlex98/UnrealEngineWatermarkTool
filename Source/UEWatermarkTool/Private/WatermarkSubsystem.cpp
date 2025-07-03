#include "WatermarkSubsystem.h"

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "UEWatermarkTool.h"
#include "Blueprint/UserWidget.h"
#include "Config/WatermarkConfig.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/SWeakWidget.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Engine/World.h"
#include "UIWatermark/UIWatermarkCompoundWidget.h"
#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Utility/WatermarkAssetFunctionLibrary.h"
#include "Utility/WatermarkFunctionLibrary.h"

TSharedPtr<SConstraintCanvas> RootCanvas;
TSharedPtr<SWidget> SlateWatermarkWidget;
UUserWidget* UMGWatermarkWidget;

void UWatermarkSubsystem::OnSeamlessTravelStart(UWorld* World, const FString& URL)
{
	UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem OnSeamlessTravelStart"));
	AddWatermarkToViewport();
}

void UWatermarkSubsystem::OnPostLoadMapWithWorld(UWorld* World)
{
	CurrentGameWorldRef = World;
	UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem OnPostLoadMapWithWorld"));
}

void UWatermarkSubsystem::OnPostWorldInitialization(UWorld* World, FWorldInitializationValues InitializationValue)
{
	UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem OnPostWorldInitialization"));
	
	if (World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE))
	{
		CurrentGameWorldRef = World;
		AddWatermarkToViewport();
	}
}

void UWatermarkSubsystem::ApplyWatermarkToScreenshot(int32 Width, int32 Height, const TArray<FColor>& InBitmap)
{
	const UWatermarkConfig* Settings = UWatermarkConfig::Get();
	if (Settings && Settings->bApplyWatermarkInScreenshot)
	{
		if (Settings->ImageOverlayTexture.IsValid())
		{
			UE_LOG(LogWatermark, Log, TEXT("UWatermarkSubsystem::OnScreenshotCaptured - Try to apply watermark in image"));

			if (Settings->bUseInvisibleWatermark)
			{
				UWatermarkAssetFunctionLibrary::ApplyInvisibleTextureWatermark(Width, Height, InBitmap, Settings->ImageOverlayTexture.LoadSynchronous());
			}
			else
			{
				UWatermarkAssetFunctionLibrary::ApplyImageOverlayWatermark(Width, Height, InBitmap, Settings->ImageOverlayTexture.LoadSynchronous());
			}
		}
		else
		{
			UE_LOG(LogWatermark, Error, TEXT("UWatermarkSubsystem::OnScreenshotCaptured - Image Overlay Texture not valid, saving a normal screenshot"));
		}
	}	
}

void UWatermarkSubsystem::OnScreenshotCaptured(int32 Width, int32 Height, const TArray<FColor>& InBitmap)
{
	UE_LOG(LogWatermark, Log, TEXT("UWatermarkSubsystem::OnScreenshotCaptured - Handling screenshot"));

	ApplyWatermarkToScreenshot(Width, Height, InBitmap);

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	ImageWrapper->SetRaw(InBitmap.GetData(), InBitmap.Num() * sizeof(FColor), Width, Height, ERGBFormat::BGRA, 8);
	TArray64<uint8> PngData = ImageWrapper->GetCompressed();
	
	FString ScreenshotDir = FPaths::ProjectSavedDir() / TEXT("Screenshots/");

#if WITH_EDITOR
	ScreenshotDir.Append(TEXT("WindowsEditor/"));
#else
	ScreenshotDir.Append(TEXT("Windows/"));
#endif
	FString TimestampString = FDateTime::UtcNow().ToString();
	FString ScreenshotPath = ScreenshotDir / TEXT("HighResScreenshot") + TimestampString + TEXT(".png");
	FFileHelper::SaveArrayToFile(PngData, *ScreenshotPath);
}

void UWatermarkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FWorldDelegates::OnStartGameInstance.AddUObject(this, &UWatermarkSubsystem::OnGameStart);
	FWorldDelegates::OnSeamlessTravelStart.AddUObject(this, &UWatermarkSubsystem::OnSeamlessTravelStart);
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UWatermarkSubsystem::OnPostWorldInitialization);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UWatermarkSubsystem::OnPostLoadMapWithWorld);

	UGameViewportClient::OnScreenshotCaptured().AddUObject(this, &UWatermarkSubsystem::OnScreenshotCaptured);

	
	UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem Initialized (Editor or Game)"));
}

void UWatermarkSubsystem::Deinitialize()
{
	FWorldDelegates::OnStartGameInstance.RemoveAll(this);

	if (GEngine && GEngine->GameViewport && RootCanvas.IsValid())
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(RootCanvas.ToSharedRef());
		RootCanvas.Reset();
		SlateWatermarkWidget.Reset();
	}

	Super::Deinitialize();
}

void UWatermarkSubsystem::AddWatermarkToViewport()
{
	if (UWatermarkFunctionLibrary::ShouldShowWatermarkUI())
	{
		switch (UWatermarkConfig::Get()->WidgetWatermarkType)
		{
		case EWidgetWatermarkType::SlateWatermark:
			AddSlateWatermark();
			break;
		case EWidgetWatermarkType::UserWidgetWatermark:
			AddUMGWatermark();
			break;
		}
	}
}

void UWatermarkSubsystem::ForceAddWatermarkToViewport(EWidgetWatermarkType WidgetWatermarkTypeToShow)
{
	switch (WidgetWatermarkTypeToShow)
	{
	case EWidgetWatermarkType::SlateWatermark:
		AddSlateWatermark();
		break;
	case EWidgetWatermarkType::UserWidgetWatermark:
		AddUMGWatermark();
	}
}

void UWatermarkSubsystem::ForceRemoveWatermarkToViewport(EWidgetWatermarkType WidgetWatermarkTypeToShow)
{
	switch (WidgetWatermarkTypeToShow)
	{
	case EWidgetWatermarkType::SlateWatermark:
		RemoveSlateWatermark();
		break;
	case EWidgetWatermarkType::UserWidgetWatermark:
		RemoveUMGWatermark();
	}
}

void UWatermarkSubsystem::AddSlateWatermark()
{
	if (GEngine && GEngine->GameViewport)
	{
		RootCanvas = SNew(SConstraintCanvas);

		SlateWatermarkWidget = SNew(SUIWatermarkCompoundWidget);

		RootCanvas->AddSlot()
		          .Anchors(FAnchors(0, 0, 1, 1))
		          .Offset(FMargin(0, 0, 0, 0))
		          .Alignment(FVector2D(0, 0))
		[
			SlateWatermarkWidget.ToSharedRef()
		];

		GEngine->GameViewport->AddViewportWidgetContent(
			SNew(SWeakWidget)
			.PossiblyNullContent(RootCanvas.ToSharedRef()),
			UWatermarkConfig::Get()->WatermarkZOrder
		);

		UE_LOG(LogWatermark, Log, TEXT("WatermarkWidget added to viewport"));
	}
}

void UWatermarkSubsystem::AddUMGWatermark()
{
	UWorld* World = CurrentGameWorldRef;
		
	if (IsValid(UMGWatermarkWidget) && UMGWatermarkWidget->IsInViewport())
	{
		UE_LOG(LogWatermark, Warning, TEXT("WatermarkSubsystem:AddUMGWatermark - UMGWatermarkWidget is already in viewport"));
		return;
	}
	
	if (!World)
	{
		UE_LOG(LogWatermark, Warning, TEXT("WatermarkSubsystem:AddUMGWatermark - World is null, retrying next tick"));
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float DeltaTime) -> bool
		{
			AddUMGWatermark();
			return false;
		}), 0.5f);
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogWatermark, Warning, TEXT("WatermarkSubsystem:AddUMGWatermark - PlayerController is not ready, retrying next tick"));
		World->GetTimerManager().SetTimerForNextTick(this, &UWatermarkSubsystem::AddUMGWatermark);
		return;
	}

	UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem:AddUMGWatermark - PlayerController found, proceeding with widget creation"));

	bool Returns;
	UMGWatermarkWidget = UWatermarkFunctionLibrary::CreateWatermarkUserWidgetFromConfig(PC, Returns);
	if (Returns) return;

	UE_LOG(LogWatermark, Log,
	       TEXT("WatermarkSubsystem:AddUMGWatermark - Widget created successfully, adding to viewport"));
	UMGWatermarkWidget->AddToViewport(UWatermarkConfig::Get()->WatermarkZOrder);
}

void UWatermarkSubsystem::RemoveSlateWatermark()
{
	if (RootCanvas.IsValid())
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(RootCanvas.ToSharedRef());
		RootCanvas.Reset();
		SlateWatermarkWidget.Reset();
	}
}

void UWatermarkSubsystem::RemoveUMGWatermark()
{
	if (UMGWatermarkWidget)
	{
		UMGWatermarkWidget->RemoveFromParent();
		UMGWatermarkWidget = nullptr;
	}
}

UWorld* UWatermarkSubsystem::GetGameWorldContextless()
{
	TIndirectArray<FWorldContext> Contexts = GEngine->GetWorldContexts();

	if (Contexts.Num())
		for (auto Context : Contexts)
			if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
				return Context.World();

	return nullptr;
}

void UWatermarkSubsystem::OnGameStart(UGameInstance* GameInstance)
{
	CurrentGameWorldRef = GameInstance->GetWorld();
	
	//Watermark UI
	AddWatermarkToViewport();
}