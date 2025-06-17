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

#if WITH_EDITOR
void ALightWatermarkManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (bEnableInEditor && ShouldSpawn())
	{
		ClearSpawnedLights();
		SpawnLights();
	}
}
#endif

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
		UE_LOG(LogWatermark, Log, TEXT("ALightWatermarkManager::SpawnLights - Light Class is not valid"));
		return;
	};

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector Extent = BoxComponent->GetUnscaledBoxExtent();
	const FVector GridSpacing = BoxComponent->GetScaledBoxExtent();
	const FVector Origin = GetActorLocation();

	for (float X = -Extent.X; X <= Extent.X; X += GridSpacing.X)
	for (float Y = -Extent.Y; Y <= Extent.Y; Y += GridSpacing.Y)
	{
		FVector Location = Origin + FVector(X, Y, 0);
		ALightWatermarkActor* LightActor = GetWorld()->SpawnActor<ALightWatermarkActor>(LightClass, Location, SpawnedLightsRotator);
		if (LightActor)
		{
			//LightActor->SetIsTemporarilyHiddenInEditor(true);
			LightActor->SetFlags(RF_Transient);
			LightActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
			SpawnedLights.Add(LightActor);
			UE_LOG(LogWatermark, Log, TEXT("ALightWatermarkManager::SpawnLights - Spawned at %s"), *Location.ToString());
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

	if (PropertyChangedEvent.Property &&
	   (PropertyChangedEvent.Property->GetFName() == FName(L"BoxExtent") ||
		PropertyChangedEvent.Property->GetFName() == FName(L"RelativeScale3D")))
	{
		ClearSpawnedLights();
		SpawnLights();
	}
}
#endif
