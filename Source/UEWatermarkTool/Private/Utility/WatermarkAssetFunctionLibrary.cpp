// Fill out your copyright notice in the Description page of Project Settings.

#include "Utility/WatermarkAssetFunctionLibrary.h"
#include "ImageUtils.h"
#include "UEWatermarkTool.h"
#include "Engine/Texture2D.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Slate/WidgetRenderer.h"

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

    UE_LOG(LogWatermark, Log, TEXT("UWatermarkAssetFunctionLibrary::RenderSlateWidgetToBitmap - Widget rendered to texture"));
}

//Texture

bool UWatermarkAssetFunctionLibrary::ValidateWatermarkParameters(const uint8* HostPixels, const int32 HostWidth, const int32 HostHeight,
	const TArray<FColor>& WatermarkColors, const int32 TargetWidth, const int32 TargetHeight, EWatermarkValidationResult& OutResult)
{
	if (!HostPixels)
	{
		UE_LOG(LogWatermark, Error, TEXT("UWatermarkAssetFunctionLibrary::ValidateWatermarkParameters - Host pixels pointer is null"));
		OutResult = EWatermarkValidationResult::InvalidPointer;
		return false;
	}

	if (HostWidth <= 0 || HostHeight <= 0 || TargetWidth <= 0 || TargetHeight <= 0)
	{
		UE_LOG(LogWatermark, Error, TEXT("UWatermarkAssetFunctionLibrary::ValidateWatermarkParameters - Invalid dimensions: Host(%d,%d), Target(%d,%d)"),
			HostWidth, HostHeight, TargetWidth, TargetHeight);
		OutResult = EWatermarkValidationResult::InvalidDimensions;
		return false;
	}

	if (TargetWidth > HostWidth || TargetHeight > HostHeight)
	{
		UE_LOG(LogWatermark, Error, TEXT("UWatermarkAssetFunctionLibrary::ValidateWatermarkParameters - Watermark size (%d,%d) exceeds host image size (%d,%d)"),
			TargetWidth, TargetHeight, HostWidth, HostHeight);
		OutResult = EWatermarkValidationResult::WatermarkTooLarge;
		return false;
	}

	if (WatermarkColors.Num() < (TargetWidth * TargetHeight))
	{
		UE_LOG(LogWatermark, Error, TEXT("UWatermarkAssetFunctionLibrary::ValidateWatermarkParameters - Watermark colors array size (%d) is smaller than required (%d)"),
			WatermarkColors.Num(), TargetWidth * TargetHeight);
		OutResult = EWatermarkValidationResult::InvalidArraySize;
		return false;
	}

	OutResult = EWatermarkValidationResult::Valid;
	return true;
}


bool UWatermarkAssetFunctionLibrary::ReadTexturePixels(UTexture2D* Texture, TArray<FColor>& OutPixels, int32& OutWidth, int32& OutHeight)
{
    if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0)
    {
        UE_LOG(LogWatermark, Error, TEXT("ReadTexturePixels - Invalid texture or platform data"));
        return false;
    }

    FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
    OutWidth = Mip.SizeX;
    OutHeight = Mip.SizeY;

    FColor* Src = static_cast<FColor*>(Mip.BulkData.Lock(LOCK_READ_ONLY));
    if (!Src)
    {
        UE_LOG(LogWatermark, Error, TEXT("ReadTexturePixels - Failed to lock mip data"));
        return false;
    }

    OutPixels.SetNum(OutWidth * OutHeight);
    FMemory::Memcpy(OutPixels.GetData(), Src, OutWidth * OutHeight * sizeof(FColor));
    Mip.BulkData.Unlock();

    return true;
}

void UWatermarkAssetFunctionLibrary::ResizePixels(const TArray<FColor>& Src, int32 SrcW, int32 SrcH, int32 DestW, int32 DestH, TArray<FColor>& Out)
{
    FImageUtils::ImageResize(SrcW, SrcH, Src, DestW, DestH, Out, true);
}


void UWatermarkAssetFunctionLibrary::ApplyImageOverlayWatermark(int32 Width, int32 Height, const TArray<FColor>& InBitmap, UTexture2D* WatermarkTexture)
{
	TArray<FColor>& Bitmap = const_cast<TArray<FColor>&>(InBitmap);
	
	FTexture2DMipMap& Mip = WatermarkTexture->GetPlatformData()->Mips[0];
	FColor* SourceData = static_cast<FColor*>(Mip.BulkData.Lock(LOCK_READ_ONLY));
	if (SourceData == nullptr)
	{
		UE_LOG(LogWatermark, Error, TEXT("UWatermarkAssetFunctionLibrary::ApplyImageOverlayWatermark - Can't Extract Bulk Data from Watermark Texure"))
		return;
	}
	const int32 ImgWidth = Mip.SizeX;
	const int32 ImgHeight = Mip.SizeY;

	for (int32 Y = 0; Y < ImgHeight; ++Y)
	{
		for (int32 X = 0; X < ImgWidth; ++X)
		{
			int32 TargetX = Width - ImgWidth + X - 10;
			int32 TargetY = Height - ImgHeight + Y - 10;

			int32 SrcIndex = Y * ImgWidth + X;
			int32 DstIndex = TargetY * Width + TargetX;

			if (InBitmap.IsValidIndex(DstIndex))
			{
				Bitmap[DstIndex] = AlphaBlend(SourceData[SrcIndex], Bitmap[DstIndex]);
			}
		}
	}

	Mip.BulkData.Unlock();

	UE_LOG(LogWatermark, Log, TEXT("UWatermarkAssetFunctionLibrary::ApplyImageOverlayWatermark - Watermark image blended"));
}

FColor UWatermarkAssetFunctionLibrary::AlphaBlend(const FColor& Src, const FColor& Dst)
{
	float SrcAlpha = Src.A / 255.0f;
	float DstAlpha = Dst.A / 255.0f;
	float OutAlpha = SrcAlpha + DstAlpha * (1.0f - SrcAlpha);

	if (OutAlpha == 0.0f)
	{
		return FColor(0, 0, 0, 0);
	}

	uint8 R = FMath::Clamp(int32((Src.R * SrcAlpha + Dst.R * DstAlpha * (1.0f - SrcAlpha)) / OutAlpha), 0, 255);
	uint8 G = FMath::Clamp(int32((Src.G * SrcAlpha + Dst.G * DstAlpha * (1.0f - SrcAlpha)) / OutAlpha), 0, 255);
	uint8 B = FMath::Clamp(int32((Src.B * SrcAlpha + Dst.B * DstAlpha * (1.0f - SrcAlpha)) / OutAlpha), 0, 255);
	uint8 A = FMath::Clamp(int32(OutAlpha * 255.0f), 0, 255);

	return FColor(R, G, B, A);
}

void UWatermarkAssetFunctionLibrary::ApplyInvisibleTextureWatermark(int32 Width, int32 Height, const TArray<FColor>& InBitmap, UTexture2D* WatermarkTexture)
{
    if (!WatermarkTexture)
    {
        UE_LOG(LogWatermark, Error, TEXT("UWatermarkAssetFunctionLibrary::ApplyInvisibleTextureWatermark - Not Valid Texture"));
        return;
    }

    TArray<FColor>& Bitmap = const_cast<TArray<FColor>&>(InBitmap);
    
    FTexture2DMipMap& Mip = WatermarkTexture->GetPlatformData()->Mips[0];
    FColor* WatermarkData = static_cast<FColor*>(Mip.BulkData.Lock(LOCK_READ_ONLY));
    
    if (WatermarkData == nullptr)
    {
        UE_LOG(LogWatermark, Error, TEXT("UWatermarkAssetFunctionLibrary::ApplyInvisibleTextureWatermark - Can't access to texture data"));
        return;
    }

    const int32 WatermarkWidth = Mip.SizeX;
    const int32 WatermarkHeight = Mip.SizeY;

    const int32 StartX = Width - WatermarkWidth;
    const int32 StartY = Height - WatermarkHeight;

    for (int32 Y = 0; Y < WatermarkHeight; Y++)
    {
        for (int32 X = 0; X < WatermarkWidth; X++)
        {
            const int32 TargetX = StartX + X;
            const int32 TargetY = StartY + Y;

            if (TargetX >= 0 && TargetX < Width && TargetY >= 0 && TargetY < Height)
            {
                const int32 WatermarkIndex = Y * WatermarkWidth + X;
                const int32 TargetIndex = TargetY * Width + TargetX;
                
                const FColor& WatermarkPixel = WatermarkData[WatermarkIndex];
                const float Luminance = 0.299f * WatermarkPixel.R + 0.587f * WatermarkPixel.G + 0.114f * WatermarkPixel.B;
                
                if (Luminance > 127)
                {
                    Bitmap[TargetIndex].R = (Bitmap[TargetIndex].R & 0xFE) | 1;
                    Bitmap[TargetIndex].G = (Bitmap[TargetIndex].G & 0xFE) | 1;
                    Bitmap[TargetIndex].B = (Bitmap[TargetIndex].B & 0xFE) | 1;
                }
                else
                {
                    Bitmap[TargetIndex].R &= 0xFE;
                    Bitmap[TargetIndex].G &= 0xFE;
                    Bitmap[TargetIndex].B &= 0xFE;
                }
            }
        }
    }

    Mip.BulkData.Unlock();
    
    UE_LOG(LogWatermark, Log, TEXT("UWatermarkAssetFunctionLibrary::ApplyInvisibleTextureWatermark - invisible Watermark applied"));
}