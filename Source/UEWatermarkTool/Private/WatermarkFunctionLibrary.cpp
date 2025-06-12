// Fill out your copyright notice in the Description page of Project Settings.


#include "WatermarkFunctionLibrary.h"
#include "Config/WatermarkConfig.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "UEWatermarkTool.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Slate/WidgetRenderer.h"
#include "Sound/SoundWave.h"

bool UWatermarkFunctionLibrary::ShouldShowWatermarkUI()
{
#if WITH_EDITOR
	return UWatermarkConfig::Get()->bEnableWatermarkUIInEditor;
#else
	return UWatermarkConfig::Get()->bEnableWatermarkUI;
#endif
}

bool UWatermarkFunctionLibrary::ShouldShowWatermarkGym()
{
#if WITH_EDITOR
	return UWatermarkConfig::Get()->bEnableWatermarkUIInEditor;
#else
	return UWatermarkConfig::Get()->bEnableWatermarkUI;
#endif
}

void UWatermarkFunctionLibrary::SetGymMpcScalarValue(FName ParamName, float Value, UObject* WorldContextObject)
{
	if (!UWatermarkConfig::Get()->GymMPC.IsValid()) return;

	UMaterialParameterCollection* MPC = Cast<UMaterialParameterCollection>(UWatermarkConfig::Get()->GymMPC.ResolveObject());
	UKismetMaterialLibrary::SetScalarParameterValue(WorldContextObject, MPC, ParamName, Value);
}

void UWatermarkFunctionLibrary::UpdateWatermarkGymEnableStatus(UObject* WorldContextObject)
{
	SetGymMpcScalarValue(TEXT("IsEnabled"), ShouldShowWatermarkGym() ? 1.0f : 0.0f, WorldContextObject);
}


void UWatermarkFunctionLibrary::EmbedLSBWatermark(UTexture2D* Texture, const FString& Message)
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

FString UWatermarkFunctionLibrary::ExtractLSBWatermark(UTexture2D* Texture, int32 MessageLength)
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

void UWatermarkFunctionLibrary::EmbedDCTWatermark(UTexture2D* Texture, const FString& Message)
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

FString UWatermarkFunctionLibrary::ExtractDCTWatermark(UTexture2D* Texture, int32 MessageLength)
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

void UWatermarkFunctionLibrary::EmbedSpreadSpectrumWatermark(USoundWave* SoundWave, const FString& Message)
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

FString UWatermarkFunctionLibrary::ExtractSpreadSpectrumWatermark(USoundWave* SoundWave, int32 MessageLength)
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
/*
UTexture2D* UWatermarkFunctionLibrary::CreateBitmaskTexture(const FString& BitString)
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

#include "Misc/CString.h"

FString UWatermarkFunctionLibrary::StringToBitString(const FString& Input)
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

void UWatermarkFunctionLibrary::RenderUserWidgetToBitmap(UUserWidget* Widget, int32 TargetWidth, int32 TargetHeight, TArray<FColor>& OutPixels)
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

void UWatermarkFunctionLibrary::RenderSlateWidgetToBitmap(TSharedRef<SWidget> SlateWidget, int32 TargetWidth,
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


UUserWidget* UWatermarkFunctionLibrary::CreateWatermarkUserWidgetFromConfig(APlayerController* PC, bool& bHasSucceeded)
{
    bHasSucceeded = false;
    TSoftClassPtr<UUserWidget> WatermarkClass = UWatermarkConfig::Get()->WatermarkUserWidgetClass;
    if (!WatermarkClass.IsValid())
    {
        UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem:AddUMGWatermark - WatermarkUserWidgetClass is invalid"));
        bHasSucceeded = true;
        return nullptr;
    }

    UClass* WidgetClass = WatermarkClass.LoadSynchronous();
    if (!WidgetClass)
    {
        UE_LOG(LogWatermark, Error, TEXT("WatermarkSubsystem:AddUMGWatermark - LoadSynchronous() returned nullptr"));
        bHasSucceeded = true;
        return nullptr;
    }

    UE_LOG(LogWatermark, Log, TEXT("WatermarkSubsystem:AddUMGWatermark - Widget class loaded successfully, creating widget"));

    UUserWidget* WatermarkUserWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
    if (!WatermarkUserWidget)
    {
        UE_LOG(LogWatermark, Error, TEXT("WatermarkSubsystem:AddUMGWatermark - CreateWidget returned nullptr"));
        bHasSucceeded = true;
        return nullptr;
    }

    return WatermarkUserWidget;
}

