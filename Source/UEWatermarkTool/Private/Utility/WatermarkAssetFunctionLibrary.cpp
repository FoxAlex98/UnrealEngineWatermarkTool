// Fill out your copyright notice in the Description page of Project Settings.

#include "Utility/WatermarkAssetFunctionLibrary.h"
#include "Engine/Texture2D.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Slate/WidgetRenderer.h"
#include "Sound/SoundWave.h"
#include "Misc/CString.h"

FString UWatermarkAssetFunctionLibrary::StringToBitString(const FString& Input)
{
    FTCHARToUTF8 Utf8Converter(*Input);
    const uint8* Data   = reinterpret_cast<const uint8*>(Utf8Converter.Get());
    int32       Length  = Utf8Converter.Length();

    FString BitString;
    BitString.Reserve(Length * 8);

    for (int32 i = 0; i < Length; ++i)
    {
        uint8 Byte = Data[i];
        for (int bit = 7; bit >= 0; --bit)
        {
            bool bIsOne = ((Byte >> bit) & 0x1) != 0;
            BitString.AppendChar(bIsOne ? '1' : '0');
        }
    }

    return BitString;
}

void UWatermarkAssetFunctionLibrary::RenderUserWidgetToBitmap(UUserWidget* Widget, int32 TargetWidth, int32 TargetHeight, TArray<FColor>& OutPixels)
{
    UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
    RenderTarget->InitCustomFormat(TargetWidth, TargetHeight, PF_B8G8R8A8, false);
    RenderTarget->ClearColor = FLinearColor::Transparent;
    RenderTarget->UpdateResourceImmediate();

    FWidgetRenderer Renderer(true, false);
    Renderer.DrawWidget(RenderTarget, Widget->TakeWidget(), FVector2D(TargetWidth, TargetHeight), 0.f);

    FTextureRenderTargetResource* RTResource = RenderTarget->GameThread_GetRenderTargetResource();

    OutPixels.SetNum(TargetWidth * TargetHeight);
    RTResource->ReadPixels(OutPixels);
}

void UWatermarkAssetFunctionLibrary::RenderSlateWidgetToBitmap(TSharedRef<SWidget> SlateWidget, int32 TargetWidth,
    int32 TargetHeight, TArray<FColor>& OutPixels)
{
    UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
    RenderTarget->InitCustomFormat(TargetWidth, TargetHeight, PF_B8G8R8A8, false);
    RenderTarget->ClearColor = FLinearColor(0, 0, 0, 0);
    RenderTarget->UpdateResourceImmediate();

    FWidgetRenderer Renderer(true);
    Renderer.DrawWidget(RenderTarget, SlateWidget, FVector2D(TargetWidth, TargetHeight), 0.f);

    FTextureRenderTargetResource* RTResource = RenderTarget->GameThread_GetRenderTargetResource();

    OutPixels.SetNum(TargetWidth * TargetHeight);
    RTResource->ReadPixels(OutPixels);

    UE_LOG(LogTemp, Log, TEXT("UWatermarkSubsystem::RenderSlateWidgetToBitmap - Widget rendered to texture"));
}