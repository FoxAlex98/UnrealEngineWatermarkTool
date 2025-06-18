#include "Config/WatermarkConfig.h"

#if WITH_EDITOR

void UWatermarkConfig::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);


	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UWatermarkConfig, EnableLightWatermark)) {
		//UWatermarkFunctionLibrary::UpdateWatermarkGymEnableStatus(this);
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UWatermarkConfig, SelectedBuildIdTokens)
	|| PropertyName == GET_MEMBER_NAME_CHECKED(UWatermarkConfig, BuildIdSeparator))
	{
		BuildIdPreview = BuildFinalBuildId();
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UWatermarkConfig, BuildIdFormat))
	{
		BuildIdFormatPreview = UGameVersionFunctionLibrary::FormatBuildIdFromTemplate(BuildIdFormat);
	}

}

void UWatermarkConfig::PostInitProperties()
{
	Super::PostInitProperties();
	
	BuildIdPreview = BuildFinalBuildId();
	BuildIdFormatPreview = UGameVersionFunctionLibrary::FormatBuildIdFromTemplate(BuildIdFormat);
}

#endif

FString UWatermarkConfig::BuildFinalBuildId() const
{
	const TMap<FName, TFunction<FString()>>& TokenMap = UGameVersionFunctionLibrary::GetTokenMap();
	TArray<FString> Parts;

	for (const FName& Token : SelectedBuildIdTokens)
	{
		if (const TFunction<FString()>* Func = TokenMap.Find(Token))
		{
			Parts.Add((*Func)());
		}
		else
		{
			Parts.Add(Token.ToString());
		}
	}

	return FString::Join(Parts, *BuildIdSeparator);
}

TArray<FName> UWatermarkConfig::GetAvailableBuildIdTokens()
{
	TArray<FName> Keys;
	UGameVersionFunctionLibrary::GetTokenMap().GetKeys(Keys);
	return Keys;
}