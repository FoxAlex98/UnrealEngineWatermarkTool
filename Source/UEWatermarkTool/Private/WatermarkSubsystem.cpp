#include "WatermarkSubsystem.h"

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "UEWatermarkTool.h"
#include "WatermarkFunctionLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Config/WatermarkConfig.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/SWeakWidget.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Engine/World.h"
#include "UIWatermark/UIWatermarkCompoundWidget.h"
#include "HighResScreenshot.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "HAL/FileManagerGeneric.h"
#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

TSharedPtr<SConstraintCanvas> RootCanvas;
TSharedPtr<SUIWatermarkCompoundWidget> SlateWatermarkWidget;
UUserWidget* UMGWatermarkWidget;

void UWatermarkSubsystem::OnSeamlessTravelStart(UWorld* World, const FString& URL)
{
	UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem OnSeamlessTravelStart"));
	AddWatermarkToViewport();
}

void UWatermarkSubsystem::OnPostWorldCreation(UWorld* World)
{
	UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem OnPostWorldCreation"));
}

void UWatermarkSubsystem::OnPostWorldInitialization(UWorld* World, FWorldInitializationValues InitializationValue)
{
	UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem OnPostWorldInitialization"));
	//TODO: check if is not in editor first time
	AddWatermarkToViewport();
}

void UWatermarkSubsystem::OnScreenshotCaptured(int Width, int Height, const TArray<FColor>& InBitmap)
{
	UE_LOG(LogTemp, Log, TEXT("Watermark Screenshot: OnScreenshotCaptured"));
	UE_LOG(LogTemp, Log, TEXT("Screenshot captured %dx%d"), Width, Height);

	//Test rotated image
	TArray<FColor> RotatedBitmap;
	RotatedBitmap.SetNum(Width * Height);

	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			int32 SrcIndex = Y * Width + X;
			int32 DestX = Height - 1 - Y;
			int32 DestY = X;
			int32 DestIndex = DestY * Height + DestX;

			RotatedBitmap[DestIndex] = InBitmap[SrcIndex];
		}
	}

	int32 NewWidth = Height;
	int32 NewHeight = Width;

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	ImageWrapper->SetRaw(RotatedBitmap.GetData(), RotatedBitmap.Num() * sizeof(FColor), NewWidth, NewHeight, ERGBFormat::BGRA, 8);

	TArray64<uint8> PngData = ImageWrapper->GetCompressed();

	FString ScreenshotPath = FPaths::ProjectSavedDir() / TEXT("Screenshots") / TEXT("WindowsEditor") / TEXT("RotatedScreenshot.png");

	FFileHelper::SaveArrayToFile(PngData, *ScreenshotPath);

	UE_LOG(LogTemp, Log, TEXT("Rotated Screenshot Saved: %s"), *ScreenshotPath);
}

void UWatermarkSubsystem::OnViewportResizedEvent(FViewport* Viewport, unsigned I)
{
	UE_LOG(LogTemp, Log, TEXT("Watermark Screenshot: ViewportResizedEvent"));
}

void UWatermarkSubsystem::OnScreenshotRequestProcessed()
{
	UE_LOG(LogTemp, Log, TEXT("Watermark Screenshot: OnScreenshotRequestProcessed"));
}

void UWatermarkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FWorldDelegates::OnStartGameInstance.AddUObject(this, &UWatermarkSubsystem::OnGameStart);

	FWorldDelegates::OnSeamlessTravelStart.AddUObject(this, &UWatermarkSubsystem::OnSeamlessTravelStart);

	FWorldDelegates::OnPostWorldCreation.AddUObject(this, &UWatermarkSubsystem::OnPostWorldCreation);
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UWatermarkSubsystem::OnPostWorldInitialization);

#if WITH_EDITOR
	FWorldDelegates::OnPIEEnded.AddUObject(this, &UWatermarkSubsystem::OnGameEnd);
#endif

	FViewport::ViewportResizedEvent.AddStatic(&UWatermarkSubsystem::OnViewportResizedEvent);
	UGameViewportClient::OnScreenshotCaptured().AddStatic(&UWatermarkSubsystem::OnScreenshotCaptured);
	FScreenshotRequest::OnScreenshotRequestProcessed().AddStatic(&UWatermarkSubsystem::OnScreenshotRequestProcessed);
	
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
		if (UWatermarkConfig::Get()->bUseUMGWatermark)
		{
			AddUMGWatermark();
			return;
		}
		AddSlateWatermark();
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
	if (IsValid(UMGWatermarkWidget) && UMGWatermarkWidget->IsInViewport())
	{
		UE_LOG(LogWatermark, Warning, TEXT("WatermarkSubsystem:AddUMGWatermark - UMGWatermarkWidget is already in viewport"));
		return;
	}
	
	UWorld* World = GetGameWorldContextless();
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

	TSoftClassPtr<UUserWidget> WatermarkClass = UWatermarkConfig::Get()->WatermarkUserWidgetClass;
	if (!WatermarkClass.IsValid())
	{
		UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem:AddUMGWatermark - WatermarkUserWidgetClass is invalid"));
		return;
	}

	UClass* WidgetClass = WatermarkClass.LoadSynchronous();
	if (!WidgetClass)
	{
		UE_LOG(LogWatermark, Error, TEXT("WatermarkSubsystem:AddUMGWatermark - LoadSynchronous() returned nullptr"));
		return;
	}

	UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem:AddUMGWatermark - Widget class loaded successfully, creating widget"));

	UMGWatermarkWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
	if (!UMGWatermarkWidget)
	{
		UE_LOG(LogWatermark, Error, TEXT("WatermarkSubsystem:AddUMGWatermark - CreateWidget returned nullptr"));
		return;
	}

	UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem:AddUMGWatermark - Widget created successfully, adding to viewport"));
	UMGWatermarkWidget->AddToViewport(UWatermarkConfig::Get()->WatermarkZOrder);
}

FString UWatermarkSubsystem::FindLatestScreenshot()
{
	FString ScreenshotDir = FPaths::ProjectSavedDir() / TEXT("Screenshots/");

#if WITH_EDITOR
	ScreenshotDir.Append(TEXT("WindowsEditor/"));
#else
	ScreenshotDir.Append(TEXT("Windows/"));
#endif
	
	FString SearchPattern = ScreenshotDir / TEXT("*.png");

	TArray<FString> FoundFiles;
	IFileManager::Get().FindFiles(FoundFiles, *SearchPattern, true, false);

	FDateTime LatestTime = FDateTime::MinValue();
	FString LatestFilePath;

	for (const FString& FileName : FoundFiles)
	{
		FString FullPath = ScreenshotDir / FileName;
		FDateTime FileTime = IFileManager::Get().GetTimeStamp(*FullPath);

		if (FileTime > LatestTime)
		{
			LatestTime = FileTime;
			LatestFilePath = FullPath;
		}
	}

	return LatestFilePath;
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
	//Watermark UI
	AddWatermarkToViewport();

	//Watermark Gym
	UWatermarkFunctionLibrary::UpdateWatermarkGymEnableStatus(this);
}

void UWatermarkSubsystem::OnGameEnd(UGameInstance* GameInstance)
{
	UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem ended in Editor"));
}

void UWatermarkSubsystem::OnLevelChange(ULevel* NewLevel, ULevel* OldLevel, UWorld* World)
{
	UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem OnLevelChange"));
}

