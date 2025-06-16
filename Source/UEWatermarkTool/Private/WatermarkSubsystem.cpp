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

void UWatermarkSubsystem::OnPostLoadMapWithWorld(UWorld* World)
{
	CurrentGameWorldRef = World;
	UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem OnPostLoadMapWithWorld"));
}

void UWatermarkSubsystem::OnPostWorldInitialization(UWorld* World, FWorldInitializationValues InitializationValue)
{
	UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem OnPostWorldInitialization"));
	//TODO: check if is not in editor first time
	AddWatermarkToViewport();
}

void UWatermarkSubsystem::OnScreenshotCaptured(int32 Width, int32 Height, const TArray<FColor>& InBitmap)
{
	UE_LOG(LogTemp, Log, TEXT("UWatermarkSubsystem::OnScreenshotCaptured - Handling screenshot"));

	const UWatermarkConfig* Settings = UWatermarkConfig::Get();
	if (!Settings || Settings->WatermarkMode == EScreenshotWatermarkMode::None)
	{
		UE_LOG(LogTemp, Log, TEXT("UWatermarkSubsystem::OnScreenshotCaptured - No watermark mode set"));
		return;
	}

	switch (Settings->WatermarkMode)
	{
	case EScreenshotWatermarkMode::TextOverlay:
		ApplyTextOverlayWatermark(Width, Height, InBitmap, Settings->OverlayText, Settings->BackgroundColor);
		break;

	case EScreenshotWatermarkMode::ImageOverlay:
		if (!Settings->ImageOverlayTexture.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("Image Overlay Texture not valid"));
		}
		ApplyImageOverlayWatermark(Width, Height, InBitmap, Settings->ImageOverlayTexture.LoadSynchronous());
		break;

	case EScreenshotWatermarkMode::FontRasterOverlay:
		ApplyFontRasterWatermark(Width, Height, InBitmap, Settings->FontText, Settings->Font);
		break;
	case EScreenshotWatermarkMode::Slate:
		ApplySlateWidgetWatermark(Width, Height, InBitmap);
		break;
	case EScreenshotWatermarkMode::UMG:
		ApplyWidgetOverlayWatermark(Width, Height, InBitmap, CurrentGameWorldRef);
		break;
	default: break;
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	ImageWrapper->SetRaw(InBitmap.GetData(), InBitmap.Num() * sizeof(FColor), Width, Height, ERGBFormat::BGRA, 8);
	TArray64<uint8> PngData = ImageWrapper->GetCompressed();
	FString ScreenshotPath = FPaths::ProjectSavedDir() / TEXT("Screenshots") / TEXT("WindowsEditor") / TEXT("Test.png");
	FFileHelper::SaveArrayToFile(PngData, *ScreenshotPath);
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

void UWatermarkSubsystem::ApplyTextOverlayWatermark(int32 Width, int32 Height, const TArray<FColor>& InBitmap, const FString& Text, const FColor& RectColor)
{
	TArray<FColor>& Bitmap = const_cast<TArray<FColor>&>(InBitmap);

	const int32 RectWidth = 200;
	const int32 RectHeight = 50;

	int32 StartX = Width - RectWidth - 10;
	int32 StartY = Height - RectHeight - 10;

	for (int32 Y = 0; Y < RectHeight; ++Y)
	{
		for (int32 X = 0; X < RectWidth; ++X)
		{
			int32 Index = (StartY + Y) * Width + (StartX + X);
			if (Bitmap.IsValidIndex(Index))
			{
				Bitmap[Index] = RectColor;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("UWatermarkSubsystem::ApplyTextOverlayWatermark - Watermark rectangle drawn"));
}

void UWatermarkSubsystem::ApplyImageOverlayWatermark(int32 Width, int32 Height, const TArray<FColor>& InBitmap, UTexture2D* WatermarkTexture)
{
	TArray<FColor>& Bitmap = const_cast<TArray<FColor>&>(InBitmap);
	
	FTexture2DMipMap& Mip = WatermarkTexture->GetPlatformData()->Mips[0];
	FColor* SourceData = static_cast<FColor*>(Mip.BulkData.Lock(LOCK_READ_ONLY));
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

	UE_LOG(LogTemp, Log, TEXT("UWatermarkSubsystem::ApplyImageOverlayWatermark - Watermark image blended"));
}

void UWatermarkSubsystem::ApplyFontRasterWatermark(int32 Width, int32 Height, const TArray<FColor>& InBitmap, const FString& Text, UFont* Font)
{
	// Versione placeholder: da implementare con render to texture o font bitmap lookup
	UE_LOG(LogTemp, Warning, TEXT("UWatermarkSubsystem::ApplyFontRasterWatermark - Not implemented yet"));
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

void UWatermarkSubsystem::ApplyWidgetOverlayWatermark(int32 Width, int32 Height, const TArray<FColor>& InBitmap, UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject)) return;
	APlayerController* PC = WorldContextObject->GetWorld()->GetFirstPlayerController();
	bool bHasSucceeded = false;
	UUserWidget* WatermarkWidget = UWatermarkFunctionLibrary::CreateWatermarkUserWidgetFromConfig(PC, bHasSucceeded);
	if (!WatermarkWidget) return;

	TArray<FColor> WatermarkPixels;
	UWatermarkAssetFunctionLibrary::RenderUserWidgetToBitmap(WatermarkWidget, Width, Height, WatermarkPixels);

	TArray<FColor>& Bitmap = const_cast<TArray<FColor>&>(InBitmap);

	int32 StartX = 0;
	int32 StartY = 0;

	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			int32 SrcIndex = Y * Width + X;
			int32 DstIndex = (StartY + Y) * Width + (StartX + X);

			if (WatermarkPixels.IsValidIndex(SrcIndex) && Bitmap.IsValidIndex(DstIndex))
			{
				Bitmap[DstIndex] = AlphaBlend(WatermarkPixels[SrcIndex], Bitmap[DstIndex]);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("UWatermarkSubsystem::ApplyWidgetOverlayWatermark - Widget blended"));
}

void UWatermarkSubsystem::ApplySlateWidgetWatermark(int32 Width, int32 Height, const TArray<FColor>& InBitmap)
{
	TSharedRef<SWidget> ScreenshotSlateWatermark =
		SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0, 0, 0, 0))
			.Padding(FMargin(10))
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("DEBUG BUILD")))
				.ColorAndOpacity(FSlateColor(FLinearColor::Red))
				.Font(FSlateFontInfo("Arial", 24))
			];

	TArray<FColor> WatermarkPixels;
	UWatermarkAssetFunctionLibrary::RenderSlateWidgetToBitmap(ScreenshotSlateWatermark, Width, Height, WatermarkPixels);

	TArray<FColor>& Bitmap = const_cast<TArray<FColor>&>(InBitmap);

	int32 StartX = 0;
	int32 StartY = 0;

	/*
	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			int32 SrcIndex = Y * Width + X;
			int32 DstIndex = (StartY + Y) * Width + (StartX + X);

			if (WatermarkPixels.IsValidIndex(SrcIndex) && Bitmap.IsValidIndex(DstIndex))
			{
				Bitmap[DstIndex] = AlphaBlend(WatermarkPixels[SrcIndex], Bitmap[DstIndex]);
			}
		}
	}
	*/
	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			int32 SrcIdx = X + Y * Width;
			int32 DstIdx = (StartX + X) + (StartY + Y) * Width;

			if (DstIdx >= 0 && DstIdx < Bitmap.Num() && SrcIdx < WatermarkPixels.Num())
			{
				FColor Src = WatermarkPixels[SrcIdx];
				FColor& Dst = Bitmap[DstIdx];

				// Alpha blend
				float SrcAlpha = Src.A / 255.0f;
				Dst.R = FMath::Lerp(Dst.R, Src.R, SrcAlpha);
				Dst.G = FMath::Lerp(Dst.G, Src.G, SrcAlpha);
				Dst.B = FMath::Lerp(Dst.B, Src.B, SrcAlpha);
			}
		}
	}
	UE_LOG(LogTemp, Log, TEXT("UWatermarkSubsystem::ApplySlateWidgetWatermark - Slate widget blended"));
}
