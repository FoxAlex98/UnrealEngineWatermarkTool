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

void UWatermarkAssetFunctionLibrary::EmbedWatermarkInStaticMesh(UStaticMesh* StaticMesh, const FString& NumericPattern)
{
    if (!StaticMesh || NumericPattern.IsEmpty())
    {
        UE_LOG(LogWatermark, Error, TEXT("EmbedWatermarkInStaticMesh - Invalid parameters"));
        return;
    }

    StaticMesh->Modify();

    FMeshDescription* MeshDesc = StaticMesh->GetMeshDescription(0);
    if (!MeshDesc)
    {
        UE_LOG(LogWatermark, Error, TEXT("EmbedWatermarkInStaticMesh - MeshDescription missing"));
        return;
    }

    FStaticMeshAttributes Attributes(*MeshDesc);
    TVertexAttributesRef<FVector3f> Positions = Attributes.GetVertexPositions();

    TArray<FVertexID> VertexIDs;
    FVertexArray Vertices = MeshDesc->Vertices();
    for (auto VertexID : Vertices.GetElementIDs())
    {
        VertexIDs.Add(VertexID);
        if (VertexIDs.Num() >= 3)
            break;
    }

    if (VertexIDs.Num() < 3)
    {
        UE_LOG(LogWatermark, Error, TEXT("EmbedWatermarkInStaticMesh - Not enough vertices"));
        return;
    }

    float EncodedValue = FCString::Atof(*FString::Printf(TEXT("0.%s"), *NumericPattern));

    FVector3f Pos0 = Positions[VertexIDs[0]];
    FVector3f Pos1 = Positions[VertexIDs[1]];
    FVector3f Pos2 = Positions[VertexIDs[2]];

    Pos0.X += EncodedValue;
    Pos1.Y += EncodedValue;
    Pos2.Z += EncodedValue;

    Positions[VertexIDs[0]] = Pos0;
    Positions[VertexIDs[1]] = Pos1;
    Positions[VertexIDs[2]] = Pos2;

    StaticMesh->CommitMeshDescription(0);
    StaticMesh->Build(false);
    StaticMesh->MarkPackageDirty();

    UE_LOG(LogWatermark, Log, TEXT("EmbedWatermarkInStaticMesh - Watermark %s embedded"), *NumericPattern);
}

FString UWatermarkAssetFunctionLibrary::ExtractWatermarkFromStaticMesh(UStaticMesh* StaticMesh, int32 DecimalDigits)
{
    if (!StaticMesh || DecimalDigits <= 0)
    {
        UE_LOG(LogWatermark, Error, TEXT("ExtractWatermarkFromStaticMesh - Invalid parameters"));
        return FString();
    }

    const FMeshDescription* MeshDesc = StaticMesh->GetMeshDescription(0);
    if (!MeshDesc)
    {
        UE_LOG(LogWatermark, Error, TEXT("ExtractWatermarkFromStaticMesh - MeshDescription not found"));
        return FString();
    }

    FMeshDescription Attributes(*MeshDesc);
    TVertexAttributesConstRef<FVector3f> Positions = Attributes.GetVertexPositions();

    TArray<FVertexID> VertexIDs;
    FVertexArray Vertices = MeshDesc->Vertices();
    for (auto VertexID : Vertices.GetElementIDs())
    {
        VertexIDs.Add(VertexID);
        if (VertexIDs.Num() >= 3)
            break;
    }

    if (VertexIDs.Num() < 3)
    {
        UE_LOG(LogWatermark, Error, TEXT("ExtractWatermarkFromStaticMesh - Not enough vertices"));
        return FString();
    }

    FVector3f Pos0 = Positions[VertexIDs[0]];
    FVector3f Pos1 = Positions[VertexIDs[1]];
    FVector3f Pos2 = Positions[VertexIDs[2]];

    auto ExtractDecimal = [DecimalDigits](float Value) -> FString
    {
        float Fraction = FMath::Abs(Value - FMath::RoundToFloat(Value));
        FString AsString = FString::Printf(TEXT("%.10f"), Fraction);
        int32 DotIndex;
        if (AsString.FindChar('.', DotIndex))
        {
            FString Decimals = AsString.Mid(DotIndex + 1, DecimalDigits);
            return Decimals;
        }
        return FString();
    };

    FString Out = ExtractDecimal(Pos0.X);
    Out = Out.IsEmpty() ? ExtractDecimal(Pos1.Y) : Out;
    Out = Out.IsEmpty() ? ExtractDecimal(Pos2.Z) : Out;

    UE_LOG(LogWatermark, Log, TEXT("Extracted watermark: %s"), *Out);
    return Out;
}
#endif