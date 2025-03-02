#include "WatermarkSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/SWeakWidget.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Engine/World.h"
#include "UIWatermark/UIWatermarkCompoundWidget.h"

TSharedPtr<SUIWatermarkCompoundWidget> WatermarkWidget;
TSharedPtr<SConstraintCanvas> RootCanvas;

void UWatermarkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FWorldDelegates::OnStartGameInstance.AddUObject(this, &UWatermarkSubsystem::OnGameStart);
	FWorldDelegates::OnPIEEnded.AddUObject(this, &UWatermarkSubsystem::OnGameEnd);

	UE_LOG(LogTemp, Log, TEXT("WatermarkSubsystem Initialized (Editor or Game)"));
}

void UWatermarkSubsystem::Deinitialize()
{
	FWorldDelegates::OnStartGameInstance.RemoveAll(this);

	if (GEngine && GEngine->GameViewport && RootCanvas.IsValid())
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(RootCanvas.ToSharedRef());
		RootCanvas.Reset();
		WatermarkWidget.Reset();
	}

	Super::Deinitialize();
}

void UWatermarkSubsystem::OnGameStart(UGameInstance* GameInstance)
{
	if (GEngine && GEngine->GameViewport)
	{
		RootCanvas = SNew(SConstraintCanvas);

		WatermarkWidget = SNew(SUIWatermarkCompoundWidget);

		RootCanvas->AddSlot()
		.Anchors(FAnchors(1, 1))
		.Alignment(FVector2D(1, 1))
		.Offset(FMargin(-10, -10, 200, 50))
		[
			WatermarkWidget.ToSharedRef()
		];

		GEngine->GameViewport->AddViewportWidgetContent(
			SNew(SWeakWidget)
			.PossiblyNullContent(RootCanvas.ToSharedRef())
		);

		UE_LOG(LogTemp, Log, TEXT("WatermarkWidget added to viewport"));
	}
}

void UWatermarkSubsystem::OnGameEnd(UGameInstance* GameInstance)
{
	UE_LOG(LogTemp, Log, TEXT("WatermarkSubsystem ended in Editor"));
}
