// MorseFlickerComponent.cpp

#include "EasterEggUtility/MorseFlickerComponent.h"
#include "Components/LightComponent.h"
#include "EasterEggUtility/EasterEggFunctionLibrary.h"
#include "Engine/World.h"

UMorseFlickerComponent::UMorseFlickerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMorseFlickerComponent::BeginPlay()
{
	Super::BeginPlay();

	InitTargetLight();
	if (bAutoStart && TargetLight)
	{
		MorseSequence = UEasterEggFunctionLibrary::ConvertToMorse(Message);
		StartFlickering();
	}
}

void UMorseFlickerComponent::InitTargetLight()
{
	if (AActor* Owner = GetOwner())
	{
		TArray<UActorComponent*> Components = Owner->GetComponentsByTag(ULightComponent::StaticClass(), LightComponentTag);
		if (Components.Num() > 0)
		{
			TargetLight = Cast<ULightComponent>(Components[0]);
		}
	}

	if (!TargetLight)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMorseFlickerComponent::InitTargetLight - No valid light found with tag %s"), *LightComponentTag.ToString());
	}
}

void UMorseFlickerComponent::StartFlickering()
{
	CurrentIndex = 0;
	HandleNextSymbol();
}

void UMorseFlickerComponent::HandleNextSymbol()
{
	if (!TargetLight || CurrentIndex >= MorseSequence.Num())
	{
		SetLightState(false);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Flicker, this, &UMorseFlickerComponent::StartFlickering, 2.0f, false);
		return;
	}

	TCHAR Symbol = MorseSequence[CurrentIndex];
	float Duration;

	if (bIsLightOnPhase)
	{
		if (Symbol == '.')
		{
			SetLightState(true);
			Duration = GetDuration(EMorseDurationUnit::Dot);
		}
		else if (Symbol == '-')
		{
			SetLightState(true);
			Duration = GetDuration(EMorseDurationUnit::Dash);
		}
		else
		{
			SetLightState(false);
			Duration = (Symbol == '/') ? GetDuration(EMorseDurationUnit::NextWord) : GetDuration(EMorseDurationUnit::NextLetter);
			CurrentIndex++;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_Flicker, this, &UMorseFlickerComponent::HandleNextSymbol, Duration, false);
			return;
		}

		bIsLightOnPhase = false;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Flicker, this, &UMorseFlickerComponent::HandleNextSymbol, Duration, false);
	}
	else
	{
		SetLightState(false);
		bIsLightOnPhase = true;
		CurrentIndex++;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Flicker, this, &UMorseFlickerComponent::HandleNextSymbol, GetDuration(EMorseDurationUnit::NextCharacter), false);
	}
}

void UMorseFlickerComponent::SetLightState(bool bIsEnabled)
{
	if (TargetLight)
	{
		TargetLight->SetVisibility(bIsEnabled);
	}
}

float UMorseFlickerComponent::GetDuration(EMorseDurationUnit DurationUnit)
{
	float Velocity = 1.2f / static_cast<float>(WordsPerMinute);
	return MorseSymbolDurationMap[DurationUnit] * Velocity;
}
