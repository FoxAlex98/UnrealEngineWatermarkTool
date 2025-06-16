#include "Config/WatermarkConfig.h"
#include "Utility/WatermarkFunctionLibrary.h"

#if WITH_EDITOR

void UWatermarkConfig::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);


	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UWatermarkConfig, bEnableWatermarkGymInEditor)) {
		UWatermarkFunctionLibrary::UpdateWatermarkGymEnableStatus(this);
	}
}

void UWatermarkConfig::PostInitProperties()
{
	Super::PostInitProperties();

	
}

#endif