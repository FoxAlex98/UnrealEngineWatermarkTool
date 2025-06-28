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
#include "Utility/WatermarkFunctionLibrary.h"

TSharedPtr<SConstraintCanvas> RootCanvas;
TSharedPtr<SWidget> SlateWatermarkWidget;
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

void UWatermarkSubsystem::OnScreenshotCaptured(int32 Width, int32 Height, const TArray<FColor>& InBitmap)
{
	UE_LOG(LogWatermark, Log, TEXT("UWatermarkSubsystem::OnScreenshotCaptured - Handling screenshot"));

	const UWatermarkConfig* Settings = UWatermarkConfig::Get();
	if (Settings && Settings->bApplyWatermarkInScreenshot)
	{
		if (Settings->ImageOverlayTexture.IsValid())
		{
			UE_LOG(LogWatermark, Log, TEXT("UWatermarkSubsystem::OnScreenshotCaptured - Try to apply watermark in image"));
			ApplyImageOverlayWatermark(Width, Height, InBitmap, Settings->ImageOverlayTexture.LoadSynchronous());
		}
	}
	else
	{
		UE_LOG(LogWatermark, Error, TEXT("UWatermarkSubsystem::OnScreenshotCaptured - Image Overlay Texture not valid, saving a normal screenshot"));
	}

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

void UWatermarkSubsystem::OnViewportResizedEvent(FViewport* Viewport, unsigned I)
{
	UE_LOG(LogWatermark, Log, TEXT("Watermark Screenshot: ViewportResizedEvent"));
}

void UWatermarkSubsystem::OnScreenshotRequestProcessed()
{
	UE_LOG(LogWatermark, Log, TEXT("Watermark Screenshot: OnScreenshotRequestProcessed"));
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

	//FViewport::ViewportResizedEvent.AddStatic(&UWatermarkSubsystem::OnViewportResizedEvent);
	UGameViewportClient::OnScreenshotCaptured().AddUObject(this, &UWatermarkSubsystem::OnScreenshotCaptured);
	//FScreenshotRequest::OnScreenshotRequestProcessed().AddStatic(&UWatermarkSubsystem::OnScreenshotRequestProcessed);

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UWatermarkSubsystem::OnPostLoadMapWithWorld);
	
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
	CurrentGameWorldRef = GameInstance->GetWorld();
	
	//Watermark UI
	AddWatermarkToViewport();

	//Watermark Gym
	//UWatermarkFunctionLibrary::UpdateWatermarkGymEnableStatus(this);
}

void UWatermarkSubsystem::OnGameEnd(UGameInstance* GameInstance)
{
	UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem ended in Editor"));
}

void UWatermarkSubsystem::OnLevelChange(ULevel* NewLevel, ULevel* OldLevel, UWorld* World)
{
	UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem OnLevelChange"));
}

void UWatermarkSubsystem::ApplyImageOverlayWatermark(int32 Width, int32 Height, const TArray<FColor>& InBitmap, UTexture2D* WatermarkTexture)
{
	TArray<FColor>& Bitmap = const_cast<TArray<FColor>&>(InBitmap);
	
	FTexture2DMipMap& Mip = WatermarkTexture->GetPlatformData()->Mips[0];
	FColor* SourceData = static_cast<FColor*>(Mip.BulkData.Lock(LOCK_READ_ONLY));
	if (SourceData == nullptr)
	{
		UE_LOG(LogWatermark, Error, TEXT("UWatermarkSubsystem::ApplyImageOverlayWatermark - Can't Extract Bulk Data from Watermark Texure"))
		return;
	}
	const int32 ImgWidth = Mip.SizeX;
	const int32 ImgHeight = Mip.SizeY;

	for (int32 Y = 0; Y < ImgHeight; ++Y)
	{
		for (int32 X = 0; X < ImgWidth; ++X)
		{
			int32 TargetX = Width - ImgWidth + X - 10;
			int32 TargetY = Height - ImgHeight + Y - 10;

			int32 SrcIndex = Y * ImgWidth + X;
			int32 DstIndex = TargetY * Width + TargetX;

			if (InBitmap.IsValidIndex(DstIndex))
			{
				Bitmap[DstIndex] = AlphaBlend(SourceData[SrcIndex], Bitmap[DstIndex]);
			}
		}
	}

	Mip.BulkData.Unlock();

	UE_LOG(LogWatermark, Log, TEXT("UWatermarkSubsystem::ApplyImageOverlayWatermark - Watermark image blended"));
}

FColor UWatermarkSubsystem::AlphaBlend(const FColor& Src, const FColor& Dst)
{
	float SrcAlpha = Src.A / 255.0f;
	float DstAlpha = Dst.A / 255.0f;
	float OutAlpha = SrcAlpha + DstAlpha * (1.0f - SrcAlpha);

	if (OutAlpha == 0.0f)
	{
		return FColor(0, 0, 0, 0);
	}

	uint8 R = FMath::Clamp(int32((Src.R * SrcAlpha + Dst.R * DstAlpha * (1.0f - SrcAlpha)) / OutAlpha), 0, 255);
	uint8 G = FMath::Clamp(int32((Src.G * SrcAlpha + Dst.G * DstAlpha * (1.0f - SrcAlpha)) / OutAlpha), 0, 255);
	uint8 B = FMath::Clamp(int32((Src.B * SrcAlpha + Dst.B * DstAlpha * (1.0f - SrcAlpha)) / OutAlpha), 0, 255);
	uint8 A = FMath::Clamp(int32(OutAlpha * 255.0f), 0, 255);

	return FColor(R, G, B, A);
}