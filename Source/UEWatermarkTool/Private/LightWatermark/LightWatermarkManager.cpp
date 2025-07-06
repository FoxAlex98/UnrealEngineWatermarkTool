// Fill out your copyright notice in the Description page of Project Settings.

#include "LightWatermark/LightWatermarkManager.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "UEWatermarkTool.h"

ALightWatermarkManager::ALightWatermarkManager()
{
	PrimaryActorTick.bCanEverTick = false;
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	RootComponent = BoxComponent;
	BoxComponent->SetBoxExtent(FVector(1000, 1000, 500));
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BoxComponent->SetHiddenInGame(true);
}

void ALightWatermarkManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (bEnableInEditor && ShouldSpawn())
	{
		ClearSpawnedLights();
		SpawnLights();
	}
}

void ALightWatermarkManager::BeginPlay()
{
	Super::BeginPlay();

	if (!bEnableInBuild || !ShouldSpawn())
	{
		return;
	}

	ClearSpawnedLights();
	SpawnLights();
}

void ALightWatermarkManager::ClearSpawnedLights()
{
	for (AActor* Actor : SpawnedLights)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	SpawnedLights.Empty();
}

void ALightWatermarkManager::SpawnLights()
{
	if (!LightClass)
	{
		UE_LOG(LogWatermark, Warning, TEXT("ALightWatermarkManager::SpawnLights - Light Class is not valid"));
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector Extent = BoxComponent->GetUnscaledBoxExtent();
	const FVector Origin = GetActorLocation();

	int32 LightsX = NumLightsX;
	int32 LightsY = NumLightsY;

	if (PlacementMode == ELightPlacementMode::ByLightSize)
	{
		AActor* Temp = GetWorld()->SpawnActor<AActor>(LightClass, FVector::ZeroVector, FRotator::ZeroRotator);
		ALightWatermarkActor* TestLight = Cast<ALightWatermarkActor>(Temp);

		FVector2D LightSize = FVector2D(100, 100); // fallback
		if (TestLight)
		{
			LightSize = TestLight->GetLightSize();
			TestLight->Destroy();
		}

		LightsX = FMath::Max(2, FMath::CeilToInt((2 * Extent.X) / (LightSize.X * LightSpacingMultiplier)));
		LightsY = FMath::Max(2, FMath::CeilToInt((2 * Extent.Y) / (LightSize.Y * LightSpacingMultiplier)));

		UE_LOG(LogWatermark, Log, TEXT("ALightWatermarkManager::SpawnLights - Auto mode: LightSize %s, Grid %d x %d"),
			*LightSize.ToString(), LightsX, LightsY);
	}

	for (int32 i = 0; i < LightsX; ++i)
	for (int32 j = 0; j < LightsY; ++j)
	{
		const float RatioX = (LightsX == 1) ? 0.f : static_cast<float>(i) / (LightsX - 1);
		const float RatioY = (LightsY == 1) ? 0.f : static_cast<float>(j) / (LightsY - 1);

		const float X = FMath::Lerp(-Extent.X, Extent.X, RatioX);
		const float Y = FMath::Lerp(-Extent.Y, Extent.Y, RatioY);
		const FVector Location = Origin + FVector(X, Y, 0.f);

		// Direction
		FVector Normal = FVector::ZeroVector;
		const float Epsilon = KINDA_SMALL_NUMBER;

		if (FMath::IsNearlyEqual(FMath::Abs(X), Extent.X, Epsilon))
			Normal.X = FMath::Sign(X);
		else if (FMath::IsNearlyEqual(FMath::Abs(Y), Extent.Y, Epsilon))
			Normal.Y = FMath::Sign(Y);

		FRotator LightRotation = Normal.IsZero()
			? FRotator(-90.f, 0.f, 0.f)
			: Normal.Rotation();

		ALightWatermarkActor* LightActor = GetWorld()->SpawnActor<ALightWatermarkActor>(LightClass, Location, LightRotation, Params);
		if (LightActor)
		{
			LightActor->SetFlags(RF_Transient);
			LightActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
			SpawnedLights.Add(LightActor);

			UE_LOG(LogWatermark, Log, TEXT("ALightWatermarkManager::SpawnLights - Spawned at %s, rotation %s"), *Location.ToString(), *LightRotation.ToString());
		}
	}
}

bool ALightWatermarkManager::ShouldSpawn() const
{
	return GetWorld()
		&& (GetWorld()->WorldType == EWorldType::Editor ||
			GetWorld()->WorldType == EWorldType::Game ||
			GetWorld()->WorldType == EWorldType::PIE);
}

void ALightWatermarkManager::Destroyed()
{
	Super::Destroyed();
	
	UE_LOG(LogWatermark, Log, TEXT("ALightWatermarkManager::Destroyed - Cleaning up lights"));

	for (AActor* Light : SpawnedLights)
	{
		if (IsValid(Light))
		{
			Light->Destroy();
		}
	}

	SpawnedLights.Empty();
	
	UE_LOG(LogWatermark, Log, TEXT("ALightWatermarkManager::Destroyed - Light Watermark Manager destroyed"));
}


#if WITH_EDITOR
void ALightWatermarkManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	//BoxExtent and RelativeScale3D are inaccessible in BoxComponent
	static const FName LightsRotatorName = GET_MEMBER_NAME_CHECKED(ALightWatermarkManager, SpawnedLightsRotator);

	if (PropertyChangedEvent.Property &&
	   (PropertyChangedEvent.Property->GetFName() == FName(L"BoxExtent") ||
		PropertyChangedEvent.Property->GetFName() == FName(L"RelativeScale3D") ||
		PropertyChangedEvent.Property->GetFName() == LightsRotatorName))
	{
		ClearSpawnedLights();
		SpawnLights();
	}
}
#endif
