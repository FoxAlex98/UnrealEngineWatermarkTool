#include "WatermarkSubsystem.h"

#include "WatermarkFunctionLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Config/WatermarkConfig.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/SWeakWidget.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Engine/World.h"
#include "UIWatermark/UIWatermarkCompoundWidget.h"

TSharedPtr<SConstraintCanvas> RootCanvas;
TSharedPtr<SUIWatermarkCompoundWidget> SlateWatermarkWidget;
UUserWidget* UMGWatermarkWidget;

void UWatermarkSubsystem::OnSeamlessTravelStart(UWorld* World, const FString& URL)
{
	UE_LOG(LogTemp, Log, TEXT("WatermarkSubsystem OnSeamlessTravelStart"));
	AddWatermarkToViewport();
}

void UWatermarkSubsystem::OnPostWorldCreation(UWorld* World)
{
	UE_LOG(LogTemp, Log, TEXT("WatermarkSubsystem OnPostWorldCreation"));
}

void UWatermarkSubsystem::OnPostWorldInitialization(UWorld* World, FWorldInitializationValues InitializationValue)
{
	UE_LOG(LogTemp, Log, TEXT("WatermarkSubsystem OnPostWorldInitialization"));
	//TODO: check if is not in editor first time
	AddWatermarkToViewport();
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
	
	UE_LOG(LogTemp, Log, TEXT("WatermarkSubsystem Initialized (Editor or Game)"));
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
			.PossiblyNullContent(RootCanvas.ToSharedRef())
		);

		UE_LOG(LogTemp, Log, TEXT("WatermarkWidget added to viewport"));
	}
}

void UWatermarkSubsystem::AddUMGWatermark()
{
	if (IsValid(UMGWatermarkWidget) && UMGWatermarkWidget->IsInViewport())
	{
		UE_LOG(LogTemp, Warning, TEXT("WatermarkSubsystem:AddUMGWatermark - UMGWatermarkWidget is already in viewport"));
		return;
	}
	
	UWorld* World = GetGameWorldContextless();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("WatermarkSubsystem:AddUMGWatermark - World is null, retrying next tick"));
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
		UE_LOG(LogTemp, Warning, TEXT("WatermarkSubsystem:AddUMGWatermark - PlayerController is not ready, retrying next tick"));
		World->GetTimerManager().SetTimerForNextTick(this, &UWatermarkSubsystem::AddUMGWatermark);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("WatermarkSubsystem:AddUMGWatermark - PlayerController found, proceeding with widget creation"));

	TSoftClassPtr<UUserWidget> WatermarkClass = UWatermarkConfig::Get()->WatermarkUserWidgetClass;
	if (!WatermarkClass.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("WatermarkSubsystem:AddUMGWatermark - WatermarkUserWidgetClass is invalid"));
		return;
	}

	UClass* WidgetClass = WatermarkClass.LoadSynchronous();
	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("WatermarkSubsystem:AddUMGWatermark - LoadSynchronous() returned nullptr"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("WatermarkSubsystem:AddUMGWatermark - Widget class loaded successfully, creating widget"));

	UMGWatermarkWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
	if (!UMGWatermarkWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("WatermarkSubsystem:AddUMGWatermark - CreateWidget returned nullptr"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("WatermarkSubsystem:AddUMGWatermark - Widget created successfully, adding to viewport"));
	UMGWatermarkWidget->AddToViewport();
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
	UE_LOG(LogTemp, Log, TEXT("WatermarkSubsystem ended in Editor"));
}

