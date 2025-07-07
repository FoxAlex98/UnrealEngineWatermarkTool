// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MorseFlickerComponent.generated.h"

UENUM(BlueprintType)
enum class EMorseDurationUnit : uint8
{
	Dot,
	Dash,
	NextSymbol,
	NextLetter,
	NextWord,
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UEWATERMARKTOOL_API UMorseFlickerComponent : public UActorComponent
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMorseFlickerStateChanged, bool, bIsEnabled);

public:
	UMorseFlickerComponent();

	UFUNCTION(BlueprintCallable, Category = "Morse")
	void StartFlickering();
	
	UPROPERTY(BlueprintAssignable, Category = "Morse")
	FOnMorseFlickerStateChanged OnMorseFlickerStateChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Morse")
	FString Message = TEXT("HELLO");

	UPROPERTY(EditAnywhere, Category = "Morse")
	FName LightComponentTag = TEXT("MorseLight");

	UPROPERTY(EditAnywhere, Category = "Morse")
	bool bAutoStart = true;

	UPROPERTY(EditAnywhere, Category = "Morse", DisplayName="Words Per Minute (WPM)")
	int32 WordsPerMinute = 20;

	UFUNCTION(BlueprintNativeEvent, Category = "Morse")
	void SetLightState(bool bIsEnabled) const;

private:

	bool bIsLightOnPhase = true;

	TMap<EMorseDurationUnit, int32> MorseSymbolDurationMap = {
		{EMorseDurationUnit::Dot, 1},
		{EMorseDurationUnit::Dash, 3},
		{EMorseDurationUnit::NextSymbol, 1},
		{EMorseDurationUnit::NextLetter, 3},
		{EMorseDurationUnit::NextWord, 7}
	};
	
	TArray<TCHAR> MorseSequence;
	
	int32 CurrentIndex = 0;

	FTimerHandle TimerHandle_Flicker;

	UPROPERTY(Transient)
	ULightComponent* TargetLight = nullptr;

	void InitTargetLight();
	void HandleNextSymbol();
	
	void UpdateLightState(bool bIsEnabled) const;
	void SetLightVisibility(bool bIsEnabled) const;
	float GetDuration(EMorseDurationUnit DurationUnit);
};
