// Fill out your copyright notice in the Description page of Project Settings.

#include "Utility/WatermarkAssetFunctionLibrary.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "StaticMeshAttributes.h"
#include "UEWatermarkTool.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Slate/WidgetRenderer.h"
#include "Sound/SoundWave.h"


#include "Misc/CString.h"

/*
UTexture2D* UWatermarkAssetFunctionLibrary::CreateBitmaskTexture(const FString& BitString)
{
    int32 Width  = BitString.Len();
    int32 Height = 1;

    UTexture2D* Tex = UTexture2D::CreateTransient(Width, Height, PF_R8);
    if (!Tex)
    {
        UE_LOG(LogTemp, Warning, TEXT("CreateBitmaskTexture: creazione della texture fallita"));
        return nullptr;
    }

    Tex->MipGenSettings      = TMGS_NoMipmaps;
    Tex->SRGB                = false;
    Tex->CompressionSettings = TC_Default;
    Tex->AddToRoot();
    Tex->UpdateResource();

    TArray<uint8> RawData;
    RawData.AddUninitialized(Width * Height);
    for (int32 i = 0; i < Width; ++i)
    {
        RawData[i] = (BitString[i] == '1') ? 0xFF : 0x00;
    }

    FTexture2DMipMap& Mip = Tex->GetPlatformData()->Mips[0];
    void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
    FMemory::Memcpy(TextureData, RawData.GetData(), RawData.Num() * sizeof(uint8));
    Mip.BulkData.Unlock();

    Tex->UpdateResource();

    return Tex;
}
*/

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

#if WITH_EDITOR
void UWatermarkAssetFunctionLibrary::EmbedLSBWatermark(UTexture2D* Texture, const FString& Message)
{
	if (!Texture) return;

	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	FByteBulkData* RawImageData = &Mip.BulkData;

	uint8* Pixels = static_cast<uint8*>(RawImageData->Lock(LOCK_READ_WRITE));
	int32 Width = Mip.SizeX;
	int32 Height = Mip.SizeY;

	TArray<uint8> MessageBits;
	for (TCHAR C : Message)
	{
		for (int i = 0; i < 8; i++)
		{
			MessageBits.Add((C >> i) & 1);
		}
	}

	int32 BitIndex = 0;
	int32 TotalBits = MessageBits.Num();

	for (int32 i = 0; i < Width * Height * 4 && BitIndex < TotalBits; i += 4)
	{
		uint8& Blue = Pixels[i + 0];
		Blue = (Blue & ~1) | MessageBits[BitIndex];
		BitIndex++;
	}

	RawImageData->Unlock();
	Texture->UpdateResource();
}

FString UWatermarkAssetFunctionLibrary::ExtractLSBWatermark(UTexture2D* Texture, int32 MessageLength)
{
	if (!Texture) return "";

	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	FByteBulkData* RawImageData = &Mip.BulkData;

	uint8* Pixels = static_cast<uint8*>(RawImageData->Lock(LOCK_READ_ONLY));
	int32 Width = Mip.SizeX;
	int32 Height = Mip.SizeY;

	TArray<uint8> MessageBits;
	int32 TotalBits = MessageLength * 8;
	int32 BitIndex = 0;

	for (int32 i = 0; i < Width * Height * 4 && BitIndex < TotalBits; i += 4)
	{
		uint8 Blue = Pixels[i + 0];
		MessageBits.Add(Blue & 1);
		BitIndex++;
	}

	RawImageData->Unlock();

	FString Result;
	for (int32 i = 0; i < MessageLength; i++)
	{
		uint8 CharValue = 0;
		for (int j = 0; j < 8; j++)
		{
			CharValue |= (MessageBits[i * 8 + j] << j);
		}
		Result.AppendChar((TCHAR)CharValue);
	}

	return Result;
}

float C(int u)
{
    return (u == 0) ? sqrt(1.0f / 8.0f) : sqrt(2.0f / 8.0f);
}

void DCT8x8(const float InBlock[8][8], float OutBlock[8][8])
{
    for (int u = 0; u < 8; u++)
    {
        for (int v = 0; v < 8; v++)
        {
            float sum = 0.0f;
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    sum += InBlock[x][y] *
                        cos(((2 * x + 1) * u * PI) / 16.0f) *
                        cos(((2 * y + 1) * v * PI) / 16.0f);
                }
            }
            OutBlock[u][v] = C(u) * C(v) * sum;
        }
    }
}

void IDCT8x8(const float InBlock[8][8], float OutBlock[8][8])
{
    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            float sum = 0.0f;
            for (int u = 0; u < 8; u++)
            {
                for (int v = 0; v < 8; v++)
                {
                    sum += C(u) * C(v) * InBlock[u][v] *
                        cos(((2 * x + 1) * u * PI) / 16.0f) *
                        cos(((2 * y + 1) * v * PI) / 16.0f);
                }
            }
            OutBlock[x][y] = sum;
        }
    }
}

float GetLuminance(uint8 R, uint8 G, uint8 B)
{
    return 0.299f * R + 0.587f * G + 0.114f * B;
}

void SetRGBFromLuminance(float Y, uint8& R, uint8& G, uint8& B)
{
    R = G = B = FMath::Clamp(Y, 0.0f, 255.0f);
}

void UWatermarkAssetFunctionLibrary::EmbedDCTWatermark(UTexture2D* Texture, const FString& Message)
{
    if (!Texture) return;

    FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
    FByteBulkData* RawImageData = &Mip.BulkData;

    uint8* Pixels = static_cast<uint8*>(RawImageData->Lock(LOCK_READ_WRITE));
    int32 Width = Mip.SizeX;
    int32 Height = Mip.SizeY;

    TArray<uint8> MessageBits;
    for (TCHAR C : Message)
    {
        for (int i = 0; i < 8; i++)
        {
            MessageBits.Add((C >> i) & 1);
        }
    }

    int32 BitIndex = 0;
    int32 TotalBits = MessageBits.Num();

    for (int32 blockY = 0; blockY + 8 <= Height && BitIndex < TotalBits; blockY += 8)
    {
        for (int32 blockX = 0; blockX + 8 <= Width && BitIndex < TotalBits; blockX += 8)
        {
            float InBlock[8][8];
            float OutBlock[8][8];

            for (int y = 0; y < 8; y++)
            {
                for (int x = 0; x < 8; x++)
                {
                    int32 PixelIndex = ((blockY + y) * Width + (blockX + x)) * 4;
                    uint8 R = Pixels[PixelIndex + 2];
                    uint8 G = Pixels[PixelIndex + 1];
                    uint8 B = Pixels[PixelIndex + 0];
                    InBlock[x][y] = GetLuminance(R, G, B);
                }
            }

            DCT8x8(InBlock, OutBlock);

            float Strength = 10.0f; // TODO: Tune the strength
            if (MessageBits[BitIndex] == 1)
                OutBlock[1][2] = fabs(OutBlock[1][2]) + Strength;
            else
                OutBlock[1][2] = -fabs(OutBlock[1][2]) - Strength;

            IDCT8x8(OutBlock, InBlock);

            for (int y = 0; y < 8; y++)
            {
                for (int x = 0; x < 8; x++)
                {
                    int32 PixelIndex = ((blockY + y) * Width + (blockX + x)) * 4;
                    uint8& R = Pixels[PixelIndex + 2];
                    uint8& G = Pixels[PixelIndex + 1];
                    uint8& B = Pixels[PixelIndex + 0];
                    SetRGBFromLuminance(InBlock[x][y], R, G, B);
                }
            }

            BitIndex++;
        }
    }

    RawImageData->Unlock();
    Texture->UpdateResource();
}

FString UWatermarkAssetFunctionLibrary::ExtractDCTWatermark(UTexture2D* Texture, int32 MessageLength)
{
    if (!Texture) return "";

    FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
    FByteBulkData* RawImageData = &Mip.BulkData;

    uint8* Pixels = static_cast<uint8*>(RawImageData->Lock(LOCK_READ_ONLY));
    int32 Width = Mip.SizeX;
    int32 Height = Mip.SizeY;

    TArray<uint8> MessageBits;
    int32 TotalBits = MessageLength * 8;
    int32 BitIndex = 0;

    for (int32 blockY = 0; blockY + 8 <= Height && BitIndex < TotalBits; blockY += 8)
    {
        for (int32 blockX = 0; blockX + 8 <= Width && BitIndex < TotalBits; blockX += 8)
        {
            float InBlock[8][8];
            float OutBlock[8][8];

            for (int y = 0; y < 8; y++)
            {
                for (int x = 0; x < 8; x++)
                {
                    int32 PixelIndex = ((blockY + y) * Width + (blockX + x)) * 4;
                    uint8 R = Pixels[PixelIndex + 2];
                    uint8 G = Pixels[PixelIndex + 1];
                    uint8 B = Pixels[PixelIndex + 0];
                    InBlock[x][y] = GetLuminance(R, G, B);
                }
            }
            DCT8x8(InBlock, OutBlock);

            if (OutBlock[1][2] > 0)
                MessageBits.Add(1);
            else
                MessageBits.Add(0);

            BitIndex++;
        }
    }

    RawImageData->Unlock();

    FString Result;
    for (int32 i = 0; i < MessageLength; i++)
    {
        uint8 CharValue = 0;
        for (int j = 0; j < 8; j++)
        {
            CharValue |= (MessageBits[i * 8 + j] << j);
        }
        Result.AppendChar((TCHAR)CharValue);
    }

    return Result;
}

#define LOCTEXT_NAMESPACE "AudioWatermark"

void GeneratePNSequence(TArray<float>& OutPNSequence, int32 Length, int32 Seed)
{
    FRandomStream RandStream(Seed);
    OutPNSequence.SetNum(Length);

    for (int32 i = 0; i < Length; i++)
    {
        OutPNSequence[i] = RandStream.FRandRange(0, 1) > 0.5f ? 1.0f : -1.0f;
    }
}

void UWatermarkAssetFunctionLibrary::EmbedSpreadSpectrumWatermark(USoundWave* SoundWave, const FString& Message)
{
    if (!SoundWave) return;

    FByteBulkData* RawData = SoundWave->GetCompressedData("OGG");
    if (!RawData) return;

    int32 DataSize = RawData->GetBulkDataSize();
    int32 NumSamples = DataSize / sizeof(int16);
    int16* Samples = static_cast<int16*>(RawData->Lock(LOCK_READ_WRITE));

    TArray<int16> AudioSamples;
    AudioSamples.SetNum(NumSamples);
    FMemory::Memcpy(AudioSamples.GetData(), Samples, DataSize);

    int32 ChipLength = 1000;
    float Alpha = 0.001f;
    int32 Seed = 12345;

    TArray<uint8> MessageBits;
    for (TCHAR C : Message)
    {
        for (int i = 0; i < 8; i++)
        {
            MessageBits.Add((C >> i) & 1);
        }
    }

    TArray<float> PNSequence;
    GeneratePNSequence(PNSequence, ChipLength, Seed);

    int32 BitIndex = 0;
    int32 SampleIndex = 0;

    while (BitIndex < MessageBits.Num() && (SampleIndex + ChipLength) < AudioSamples.Num())
    {
        float BitValue = (MessageBits[BitIndex] == 1) ? 1.0f : -1.0f;

        for (int32 i = 0; i < ChipLength; i++)
        {
            float PNValue = PNSequence[i];
            float OriginalSample = static_cast<float>(AudioSamples[SampleIndex + i]);
            float WatermarkedSample = OriginalSample + Alpha * BitValue * PNValue * 32767.0f;

            AudioSamples[SampleIndex + i] = FMath::Clamp(
                static_cast<int32>(WatermarkedSample),
                -32768,
                32767
            );
        }

        SampleIndex += ChipLength;
        BitIndex++;
    }

    FMemory::Memcpy(Samples, AudioSamples.GetData(), DataSize);
    RawData->Unlock();

    SoundWave->MarkPackageDirty();
    SoundWave->Modify();
}

uint8 DetectBit(const TArray<int16>& AudioSamples, int32 StartIndex, const TArray<float>& PNSequence)
{
    float Sum = 0.0f;
    int32 ChipLength = PNSequence.Num();

    for (int32 i = 0; i < ChipLength; i++)
    {
        Sum += static_cast<float>(AudioSamples[StartIndex + i]) * PNSequence[i];
    }

    return (Sum > 0) ? 1 : 0;
}

FString UWatermarkAssetFunctionLibrary::ExtractSpreadSpectrumWatermark(USoundWave* SoundWave, int32 MessageLength)
{
    if (!SoundWave) return "";

    FByteBulkData* RawData = SoundWave->GetCompressedData("OGG");
    if (!RawData) return "";

    int32 DataSize = RawData->GetBulkDataSize();
    int32 NumSamples = DataSize / sizeof(int16);

    int16* Samples = static_cast<int16*>(RawData->Lock(LOCK_READ_ONLY));

    TArray<int16> AudioSamples;
    AudioSamples.SetNum(NumSamples);
    FMemory::Memcpy(AudioSamples.GetData(), Samples, DataSize);

    RawData->Unlock();

    int32 ChipLength = 1000;
    int32 Seed = 12345;

    TArray<float> PNSequence;
    GeneratePNSequence(PNSequence, ChipLength, Seed);

    int32 BitIndex = 0;
    int32 SampleIndex = 0;
    int32 TotalBits = MessageLength * 8;

    TArray<uint8> MessageBits;
    MessageBits.SetNum(TotalBits);

    while (BitIndex < TotalBits && (SampleIndex + ChipLength) < AudioSamples.Num())
    {
        uint8 Bit = DetectBit(AudioSamples, SampleIndex, PNSequence);
        MessageBits[BitIndex] = Bit;

        SampleIndex += ChipLength;
        BitIndex++;
    }

    FString Result;
    for (int32 i = 0; i < MessageLength; i++)
    {
        uint8 CharValue = 0;
        for (int j = 0; j < 8; j++)
        {
            CharValue |= (MessageBits[i * 8 + j] << j);
        }
        Result.AppendChar((TCHAR)CharValue);
    }

    return Result;
}

#undef LOCTEXT_NAMESPACE

void UWatermarkAssetFunctionLibrary::EmbedLSBOnRenderTarget(UTextureRenderTarget2D* RenderTarget, const FString& Message)
{
    if (!RenderTarget) return;

    FTextureRenderTargetResource* RTResource = RenderTarget->GameThread_GetRenderTargetResource();
    if (!RTResource) return;

    TArray<FColor> Pixels;
    RTResource->ReadPixels(Pixels);

    TArray<uint8> MessageBits;
    for (TCHAR C : Message)
    {
        for (int i = 0; i < 8; i++)
        {
            MessageBits.Add((C >> i) & 1);
        }
    }

    int32 BitIndex = 0;
    for (FColor& Pixel : Pixels)
    {
        if (BitIndex >= MessageBits.Num()) break;

        if (BitIndex < MessageBits.Num())
            Pixel.R = (Pixel.R & ~1) | MessageBits[BitIndex++];

        if (BitIndex < MessageBits.Num())
            Pixel.G = (Pixel.G & ~1) | MessageBits[BitIndex++];

        if (BitIndex < MessageBits.Num())
            Pixel.B = (Pixel.B & ~1) | MessageBits[BitIndex++];
    }

    TArray64<uint8> CompressedData;
    FImageUtils::PNGCompressImageArray(RenderTarget->SizeX, RenderTarget->SizeY, Pixels, CompressedData);
    FString Path = FPaths::ProjectSavedDir() / TEXT("LSB_Embedded_RenderTarget.png");
    FFileHelper::SaveArrayToFile(CompressedData, *Path);

    UE_LOG(LogWatermark, Log, TEXT("EmbedLSBOnRenderTarget - Data embedded and saved: %s"), *Path);
}

FString UWatermarkAssetFunctionLibrary::ExtractLSBFromRenderTarget(UTextureRenderTarget2D* RenderTarget, int32 MessageLength)
{
    if (!RenderTarget) return "";

    FTextureRenderTargetResource* RTResource = RenderTarget->GameThread_GetRenderTargetResource();
    if (!RTResource) return "";

    TArray<FColor> Pixels;
    RTResource->ReadPixels(Pixels);

    TArray<uint8> Bits;
    int32 TotalBits = MessageLength * 8;
    int32 BitIndex = 0;

    for (const FColor& Pixel : Pixels)
    {
        if (BitIndex >= TotalBits) break;
        Bits.Add(Pixel.R & 1); BitIndex++;
        if (BitIndex >= TotalBits) break;
        Bits.Add(Pixel.G & 1); BitIndex++;
        if (BitIndex >= TotalBits) break;
        Bits.Add(Pixel.B & 1); BitIndex++;
    }

    FString Result;
    for (int32 i = 0; i < MessageLength; ++i)
    {
        uint8 CharByte = 0;
        for (int b = 0; b < 8; ++b)
        {
            CharByte |= (Bits[i * 8 + b] << b);
        }
        Result.AppendChar((TCHAR)CharByte);
    }

    UE_LOG(LogWatermark, Log, TEXT("ExtractLSBFromRenderTarget - Decoded: %s"), *Result);

    return Result;
}

#endif

/*
void UWatermarkAssetFunctionLibrary::EmbedQuantizedWatermark(UTexture2D* HostTexture, UTexture2D* WatermarkTexture)
{
	if (!HostTexture || !WatermarkTexture)
	{
		UE_LOG(LogWatermark, Error, TEXT("UWatermarkFunctionLibrary::EmbedQuantizedWatermark - Invalid textures"));
		return;
	}

	FTexture2DMipMap& HostMip = HostTexture->GetPlatformData()->Mips[0];
	FTexture2DMipMap& WatermarkMip = WatermarkTexture->GetPlatformData()->Mips[0];

	if (HostMip.SizeX != WatermarkMip.SizeX || HostMip.SizeY != WatermarkMip.SizeY)
	{
		UE_LOG(LogWatermark, Error, TEXT("UWatermarkFunctionLibrary::EmbedQuantizedWatermark - Texture sizes do not match"));
		return;
	}

	uint8* HostPixels = static_cast<uint8*>(HostMip.BulkData.Lock(LOCK_READ_WRITE));
	uint8* WatermarkPixels = static_cast<uint8*>(WatermarkMip.BulkData.Lock(LOCK_READ_ONLY));

	const int32 PixelCount = HostMip.SizeX * HostMip.SizeY;

	for (int32 i = 0; i < PixelCount; ++i)
	{
		uint8 f = HostPixels[i];
		uint8 w = WatermarkPixels[i];

		uint8 fw = 4 * (f / 4) + (w / 64); // embed: quantization + scaled watermark
		HostPixels[i] = fw;
	}

	WatermarkMip.BulkData.Unlock();
	HostMip.BulkData.Unlock();

	HostTexture->UpdateResource();

	UE_LOG(LogWatermark, Log, TEXT("UWatermarkFunctionLibrary::EmbedQuantizedWatermark - Watermark embedded successfully"));
}

UTexture2D* UWatermarkAssetFunctionLibrary::ExtractQuantizedWatermark(UTexture2D* WatermarkedTexture)
{
	if (!WatermarkedTexture)
	{
		UE_LOG(LogWatermark, Error, TEXT("UWatermarkFunctionLibrary::ExtractQuantizedWatermark - Invalid texture"));
		return nullptr;
	}

	FTexture2DMipMap& Mip = WatermarkedTexture->GetPlatformData()->Mips[0];
	uint8* Pixels = static_cast<uint8*>(Mip.BulkData.Lock(LOCK_READ_ONLY));

	const int32 Width = Mip.SizeX;
	const int32 Height = Mip.SizeY;
	const int32 PixelCount = Width * Height;

	UTexture2D* OutTexture = UTexture2D::CreateTransient(Width, Height, PF_G8);
	if (!OutTexture)
	{
		Mip.BulkData.Unlock();
		UE_LOG(LogWatermark, Error, TEXT("UWatermarkFunctionLibrary::ExtractQuantizedWatermark - Failed to create output texture"));
		return nullptr;
	}
	OutTexture->MipGenSettings = TMGS_NoMipmaps;
	OutTexture->SRGB = false;
	OutTexture->CompressionSettings = TC_Grayscale;
	OutTexture->UpdateResource();

	TArray<uint8> OutData;
	OutData.AddUninitialized(PixelCount);

	for (int32 i = 0; i < PixelCount; ++i)
	{
		uint8 fw = Pixels[i];
		uint8 w = (fw % 4) * 64; // inverse operation: (fw mod 4) * 64
		OutData[i] = w;
	}

	Mip.BulkData.Unlock();

	FTexture2DMipMap& OutMip = OutTexture->GetPlatformData()->Mips[0];
	void* DestData = OutMip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(DestData, OutData.GetData(), PixelCount);
	OutMip.BulkData.Unlock();

	OutTexture->UpdateResource();

	UE_LOG(LogWatermark, Log, TEXT("UWatermarkFunctionLibrary::ExtractQuantizedWatermark - Watermark extracted successfully"));

	return OutTexture;
}
*/
/*
void UWatermarkAssetFunctionLibrary::EmbedQuantizedWatermark(UTexture2D* HostTexture, UTexture2D* WatermarkTexture)
{
	if (!HostTexture || !WatermarkTexture)
	{
		UE_LOG(LogWatermark, Error, TEXT("UWatermarkFunctionLibrary::EmbedQuantizedWatermark - Invalid textures"));
		return;
	}

	FTexture2DMipMap& HostMip = HostTexture->GetPlatformData()->Mips[0];
	FTexture2DMipMap& WatermarkMip = WatermarkTexture->GetPlatformData()->Mips[0];

	const int32 HostWidth = HostMip.SizeX;
	const int32 HostHeight = HostMip.SizeY;

	const int32 WmWidth = WatermarkMip.SizeX;
	const int32 WmHeight = WatermarkMip.SizeY;

	TArray<uint8> ResizedWmData;
	if (WmWidth > HostWidth || WmHeight > HostHeight)
	{
		TArray<FColor> SrcColors;
		SrcColors.SetNum(WmWidth * WmHeight);

		FColor* WmSrc = static_cast<FColor*>(WatermarkMip.BulkData.Lock(LOCK_READ_ONLY));
		FMemory::Memcpy(SrcColors.GetData(), WmSrc, WmWidth * WmHeight * sizeof(FColor));
		WatermarkMip.BulkData.Unlock();

		const int32 TargetWidth = FMath::Min(WmWidth, HostWidth);
		const int32 TargetHeight = FMath::Min(WmHeight, HostHeight);

		TArray<FColor> ResizedColors;
		FImageUtils::ImageResize(WmWidth, WmHeight, SrcColors, TargetWidth, TargetHeight, ResizedColors, true);

		ResizedWmData.SetNum(TargetWidth * TargetHeight);
		for (int32 i = 0; i < ResizedColors.Num(); ++i)
		{
			ResizedWmData[i] = ResizedColors[i].R;
		}
	}
	else
	{
		uint8* WmPixels = static_cast<uint8*>(WatermarkMip.BulkData.Lock(LOCK_READ_ONLY));
		ResizedWmData.SetNum(WmWidth * WmHeight);
		FMemory::Memcpy(ResizedWmData.GetData(), WmPixels, WmWidth * WmHeight);
		WatermarkMip.BulkData.Unlock();
	}

	uint8* HostPixels = static_cast<uint8*>(HostMip.BulkData.Lock(LOCK_READ_WRITE));

	const int32 StartX = HostWidth - (WmWidth > HostWidth ? HostWidth : WmWidth);
	const int32 StartY = HostHeight - (WmHeight > HostHeight ? HostHeight : WmHeight);

	int32 WmAppliedWidth = FMath::Min(WmWidth, HostWidth);
	int32 WmAppliedHeight = FMath::Min(WmHeight, HostHeight);

	for (int32 y = 0; y < WmAppliedHeight; ++y)
	{
		for (int32 x = 0; x < WmAppliedWidth; ++x)
		{
			const int32 HostIdx = (StartY + y) * HostWidth + (StartX + x);
			const int32 WmIdx = y * WmAppliedWidth + x;

			uint8 f = HostPixels[HostIdx];
			uint8 w = ResizedWmData[WmIdx];
			uint8 fw = 4 * (f / 4) + (w / 64);

			HostPixels[HostIdx] = fw;
		}
	}

	HostMip.BulkData.Unlock();
	HostTexture->UpdateResource();

	UE_LOG(LogWatermark, Log, TEXT("UWatermarkFunctionLibrary::EmbedQuantizedWatermark - Embedded at [%d x %d] bottom-right"), WmAppliedWidth, WmAppliedHeight);
}
UTexture2D* UWatermarkAssetFunctionLibrary::ExtractQuantizedWatermark(UTexture2D* WatermarkedTexture)
{
	if (!WatermarkedTexture)
	{
		UE_LOG(LogWatermark, Error, TEXT("UWatermarkFunctionLibrary::ExtractQuantizedWatermark - Invalid texture"));
		return nullptr;
	}

	constexpr int32 WmMaxWidth = 256;
	constexpr int32 WmMaxHeight = 256;

	FTexture2DMipMap& Mip = WatermarkedTexture->GetPlatformData()->Mips[0];
	const int32 Width = Mip.SizeX;
	const int32 Height = Mip.SizeY;
	const int32 StartX = Width - WmMaxWidth;
	const int32 StartY = Height - WmMaxHeight;

	uint8* Pixels = static_cast<uint8*>(Mip.BulkData.Lock(LOCK_READ_ONLY));

	const int32 FinalWidth = FMath::Min(WmMaxWidth, Width);
	const int32 FinalHeight = FMath::Min(WmMaxHeight, Height);
	TArray<uint8> OutData;
	OutData.SetNum(FinalWidth * FinalHeight);

	for (int32 y = 0; y < FinalHeight; ++y)
	{
		for (int32 x = 0; x < FinalWidth; ++x)
		{
			const int32 SrcIdx = (StartY + y) * Width + (StartX + x);
			const int32 DstIdx = y * FinalWidth + x;
			uint8 fw = Pixels[SrcIdx];
			uint8 w = (fw % 4) * 64;
			OutData[DstIdx] = w;
		}
	}

	Mip.BulkData.Unlock();

	UTexture2D* OutTex = UTexture2D::CreateTransient(FinalWidth, FinalHeight, PF_G8);
	if (!OutTex)
	{
		UE_LOG(LogWatermark, Error, TEXT("UWatermarkFunctionLibrary::ExtractQuantizedWatermark - Could not create result texture"));
		return nullptr;
	}
	OutTex->MipGenSettings = TMGS_NoMipmaps;
	OutTex->SRGB = false;
	OutTex->CompressionSettings = TC_Grayscale;
	OutTex->UpdateResource();

	FTexture2DMipMap& OutMip = OutTex->GetPlatformData()->Mips[0];
	void* Dest = OutMip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(Dest, OutData.GetData(), OutData.Num());
	OutMip.BulkData.Unlock();

	OutTex->UpdateResource();

	UE_LOG(LogWatermark, Log, TEXT("UWatermarkFunctionLibrary::ExtractQuantizedWatermark - Extracted [%d x %d] from bottom-right"), FinalWidth, FinalHeight);

	return OutTex;
}
*/

/*
void UWatermarkAssetFunctionLibrary::EmbedQuantizedWatermark(UTexture2D* HostTexture, UTexture2D* WatermarkTexture)
{
	if (!HostTexture || !WatermarkTexture)
	{
		UE_LOG(LogWatermark, Error, TEXT("UWatermarkFunctionLibrary::EmbedQuantizedWatermark - Invalid textures"));
		return;
	}

	FTexture2DMipMap& HostMip = HostTexture->GetPlatformData()->Mips[0];
	FTexture2DMipMap& WatermarkMip = WatermarkTexture->GetPlatformData()->Mips[0];

	const int32 HostWidth = HostMip.SizeX;
	const int32 HostHeight = HostMip.SizeY;

	const int32 WmWidth = WatermarkMip.SizeX;
	const int32 WmHeight = WatermarkMip.SizeY;

	TArray<uint8> ResizedWmData;
	if (WmWidth > HostWidth || WmHeight > HostHeight)
	{
		TArray<FColor> SrcColors;
		SrcColors.SetNum(WmWidth * WmHeight);

		FColor* WmSrc = static_cast<FColor*>(WatermarkMip.BulkData.Lock(LOCK_READ_ONLY));
		FMemory::Memcpy(SrcColors.GetData(), WmSrc, WmWidth * WmHeight * sizeof(FColor));
		WatermarkMip.BulkData.Unlock();

		const int32 TargetWidth = FMath::Min(WmWidth, HostWidth);
		const int32 TargetHeight = FMath::Min(WmHeight, HostHeight);

		TArray<FColor> ResizedColors;
		FImageUtils::ImageResize(WmWidth, WmHeight, SrcColors, TargetWidth, TargetHeight, ResizedColors, true);

		ResizedWmData.SetNum(TargetWidth * TargetHeight);
		for (int32 i = 0; i < ResizedColors.Num(); ++i)
		{
			ResizedWmData[i] = ResizedColors[i].R;
		}
	}
	else
	{
		uint8* WmPixels = static_cast<uint8*>(WatermarkMip.BulkData.Lock(LOCK_READ_ONLY));
		ResizedWmData.SetNum(WmWidth * WmHeight);
		FMemory::Memcpy(ResizedWmData.GetData(), WmPixels, WmWidth * WmHeight);
		WatermarkMip.BulkData.Unlock();
	}

	uint8* HostPixels = static_cast<uint8*>(HostMip.BulkData.Lock(LOCK_READ_WRITE));

	const int32 StartX = HostWidth - (WmWidth > HostWidth ? HostWidth : WmWidth);
	const int32 StartY = HostHeight - (WmHeight > HostHeight ? HostHeight : WmHeight);

	const int32 WmAppliedWidth = FMath::Min(WmWidth, HostWidth);
	const int32 WmAppliedHeight = FMath::Min(WmHeight, HostHeight);

	for (int32 y = 0; y < WmAppliedHeight; ++y)
	{
		for (int32 x = 0; x < WmAppliedWidth; ++x)
		{
			const int32 HostIdx = (StartY + y) * HostWidth + (StartX + x);
			const int32 WmIdx = y * WmAppliedWidth + x;

			uint8 f = HostPixels[HostIdx];
			uint8 w = ResizedWmData[WmIdx];

			uint8 scaledW = FMath::Clamp(w >> 6, 0, 3); // 0..255 → 0..3
			uint8 fw = 4 * (f / 4) + scaledW;

			HostPixels[HostIdx] = fw;
		}
	}

	HostMip.BulkData.Unlock();
	HostTexture->UpdateResource();

	UE_LOG(LogWatermark, Log, TEXT("UWatermarkFunctionLibrary::EmbedQuantizedWatermark - Watermark embedded [%d x %d] at bottom-right"), WmAppliedWidth, WmAppliedHeight);
}

UTexture2D* UWatermarkAssetFunctionLibrary::ExtractQuantizedWatermark(UTexture2D* WatermarkedTexture)
{
	if (!WatermarkedTexture)
	{
		UE_LOG(LogWatermark, Error, TEXT("UWatermarkFunctionLibrary::ExtractQuantizedWatermark - Invalid texture"));
		return nullptr;
	}

	constexpr int32 WmMaxWidth = 256;
	constexpr int32 WmMaxHeight = 256;

	FTexture2DMipMap& Mip = WatermarkedTexture->GetPlatformData()->Mips[0];
	const int32 Width = Mip.SizeX;
	const int32 Height = Mip.SizeY;

	const int32 FinalWidth = FMath::Min(WmMaxWidth, Width);
	const int32 FinalHeight = FMath::Min(WmMaxHeight, Height);

	const int32 StartX = Width - FinalWidth;
	const int32 StartY = Height - FinalHeight;

	uint8* Pixels = static_cast<uint8*>(Mip.BulkData.Lock(LOCK_READ_ONLY));

	TArray<uint8> OutData;
	OutData.SetNum(FinalWidth * FinalHeight);

	for (int32 y = 0; y < FinalHeight; ++y)
	{
		for (int32 x = 0; x < FinalWidth; ++x)
		{
			const int32 SrcIdx = (StartY + y) * Width + (StartX + x);
			const int32 DstIdx = y * FinalWidth + x;

			uint8 fw = Pixels[SrcIdx];
			uint8 w = (fw & 0x03) * 85; // 0..3 → 0,85,170,255

			OutData[DstIdx] = w;
		}
	}

	Mip.BulkData.Unlock();

	UTexture2D* OutTex = UTexture2D::CreateTransient(FinalWidth, FinalHeight, PF_G8);
	if (!OutTex)
	{
		UE_LOG(LogWatermark, Error, TEXT("UWatermarkFunctionLibrary::ExtractQuantizedWatermark - Could not create result texture"));
		return nullptr;
	}
	OutTex->MipGenSettings = TMGS_NoMipmaps;
	OutTex->SRGB = false;
	OutTex->CompressionSettings = TC_Grayscale;
	OutTex->UpdateResource();

	FTexture2DMipMap& OutMip = OutTex->GetPlatformData()->Mips[0];
	void* Dest = OutMip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(Dest, OutData.GetData(), OutData.Num());
	OutMip.BulkData.Unlock();

	OutTex->UpdateResource();

	UE_LOG(LogWatermark, Log, TEXT("UWatermarkFunctionLibrary::ExtractQuantizedWatermark - Watermark extracted [%d x %d] from bottom-right"), FinalWidth, FinalHeight);

	return OutTex;
}
*/
#if WITH_EDITOR

#endif
