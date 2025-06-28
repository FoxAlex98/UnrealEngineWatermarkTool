#include "UEWatermarkToolMovieRender.h"

#define LOCTEXT_NAMESPACE "FUEWatermarkToolMovieRenderModule"

void FUEWatermarkToolMovieRenderModule::StartupModule()
{
    UE_LOG(LogTemp, Display, TEXT("StartupModule Watermark Movie Pipeline Render"));
}

void FUEWatermarkToolMovieRenderModule::ShutdownModule()
{
    UE_LOG(LogTemp, Display, TEXT("ShutdownModule Watermark Movie Pipeline Render"));
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FUEWatermarkToolMovieRenderModule, UEWatermarkToolMovieRender)