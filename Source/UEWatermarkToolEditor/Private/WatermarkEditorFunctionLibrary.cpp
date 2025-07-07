// Fill out your copyright notice in the Description page of Project Settings.

#include "WatermarkEditorFunctionLibrary.h"

#include "StaticMeshAttributes.h"
#include "UEWatermarkToolEditor.h"
#include "UObject/SavePackage.h"
#include "Utility/WatermarkAlgorithmLibrary.h"
#include "Utility/WatermarkAssetFunctionLibrary.h"

#pragma region TextureWatermark

UTexture2D* UWatermarkEditorFunctionLibrary::CreateTransientTextureFromPixels(const TArray<FColor>& Pixels, int32 Width, int32 Height)
{
	UTexture2D* OutTex = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	if (!OutTex)
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("UWatermarkEditorFunctionLibrary::CreateTransientTextureFromPixels - Failed to create texture"));
		return nullptr;
	}

	OutTex->MipGenSettings = TMGS_NoMipmaps;
	OutTex->SRGB = true;
	OutTex->CompressionSettings = TC_Default;
	OutTex->UpdateResource();

	FTexture2DMipMap& Mip = OutTex->GetPlatformData()->Mips[0];
	void* Dest = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(Dest, Pixels.GetData(), Pixels.Num() * sizeof(FColor));
	Mip.BulkData.Unlock();

	OutTex->UpdateResource();

	return OutTex;
}

void UWatermarkEditorFunctionLibrary::ProcessTextureWatermarkEmbedding(UTexture2D* HostTexture, UTexture2D* WatermarkTexture,
    TFunction<void(uint8*, int32, int32, const TArray<FColor>&, int32, int32)> EmbedLogic)
{
    if (!HostTexture || !WatermarkTexture)
    {
        UE_LOG(LogWatermarkEditor, Error, TEXT("UWatermarkEditorFunctionLibrary::ProcessTextureWatermarkEmbedding - Invalid textures"));
        return;
    }

    FTexture2DMipMap& HostMip = HostTexture->GetPlatformData()->Mips[0];
    const int32 HostWidth = HostMip.SizeX;
    const int32 HostHeight = HostMip.SizeY;
	
    if (HostWidth <= 0 || HostHeight <= 0)
    {
        UE_LOG(LogWatermarkEditor, Error, TEXT("UWatermarkEditorFunctionLibrary::ProcessTextureWatermarkEmbedding - Invalid host texture dimensions: %dx%d"), 
            HostWidth, HostHeight);
        return;
    }

    TArray<uint8> HostData;
    HostData.SetNum(HostWidth * HostHeight * 4); // 4 bytes per pixel (RGBA)
    
    {
        uint8* HostPixels = static_cast<uint8*>(HostMip.BulkData.Lock(LOCK_READ_WRITE));
        if (!HostPixels)
        {
            UE_LOG(LogWatermarkEditor, Error, TEXT("UWatermarkEditorFunctionLibrary::ProcessTextureWatermarkEmbedding - Failed to lock host texture"));
            return;
        }
        FMemory::Memcpy(HostData.GetData(), HostPixels, HostData.Num());
        HostMip.BulkData.Unlock();
    }

    TArray<FColor> WmColors;
    int32 WmWidth, WmHeight;
    if (!UWatermarkAssetFunctionLibrary::ReadTexturePixels(WatermarkTexture, WmColors, WmWidth, WmHeight))
    {
        UE_LOG(LogWatermarkEditor, Error, TEXT("UWatermarkEditorFunctionLibrary::ProcessTextureWatermarkEmbedding - Failed to read watermark pixels"));
        return;
    }

    if (WmWidth <= 0 || WmHeight <= 0)
    {
        UE_LOG(LogWatermarkEditor, Error, TEXT("UWatermarkEditorFunctionLibrary::ProcessTextureWatermarkEmbedding - Invalid watermark dimensions: %dx%d"), 
            WmWidth, WmHeight);
        return;
    }

    int32 TargetWidth = FMath::Min(WmWidth, HostWidth);
    int32 TargetHeight = FMath::Min(WmHeight, HostHeight);
    TArray<FColor> ResizedColors;

    if (WmWidth > HostWidth || WmHeight > HostHeight)
    {
        UWatermarkAssetFunctionLibrary::ResizePixels(WmColors, WmWidth, WmHeight, 
            TargetWidth, TargetHeight, ResizedColors);
    }
    else
    {
        ResizedColors = MoveTemp(WmColors);
    }

    EmbedLogic(HostData.GetData(), HostWidth, HostHeight, ResizedColors, TargetWidth, TargetHeight);

    {
        uint8* DestPixels = static_cast<uint8*>(HostMip.BulkData.Lock(LOCK_READ_WRITE));
        if (DestPixels)
        {
            FMemory::Memcpy(DestPixels, HostData.GetData(), HostData.Num());
            HostMip.BulkData.Unlock();

            HostTexture->Source.Init(
                HostWidth,
                HostHeight,
                1,
                1,
                TSF_BGRA8,
                HostData.GetData()
            );

            HostTexture->UpdateResource();
            SaveAsset(HostTexture);
        }
        else
        {
            UE_LOG(LogWatermarkEditor, Error, TEXT("UWatermarkEditorFunctionLibrary::ProcessTextureWatermarkEmbedding - Failed to lock texture for writing"));
        }
    }
}

UTexture2D* UWatermarkEditorFunctionLibrary::ProcessTextureWatermarkExtraction(UTexture2D* WatermarkedTexture,
	TFunction<void(uint8*, int32, int32, TArray<FColor>&)> ExtractLogic)
{
	if (!WatermarkedTexture)
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("UWatermarkEditorFunctionLibrary::ProcessTextureWatermarkExtraction - Invalid texture"));
		return nullptr;
	}

	FTexture2DMipMap& Mip = WatermarkedTexture->GetPlatformData()->Mips[0];
	const int32 Width = Mip.SizeX;
	const int32 Height = Mip.SizeY;

	uint8* Pixels = static_cast<uint8*>(Mip.BulkData.Lock(LOCK_READ_ONLY));
	if (!Pixels)
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("UWatermarkEditorFunctionLibrary::ProcessTextureWatermarkExtraction - Failed to lock mip data"));
		return nullptr;
	}

	TArray<FColor> OutPixels;
	OutPixels.SetNum(Width * Height);

	ExtractLogic(Pixels, Width, Height, OutPixels);

	Mip.BulkData.Unlock();

	return CreateTransientTextureFromPixels(OutPixels, Width, Height);
}

// BlueprintCallable wrappers

void UWatermarkEditorFunctionLibrary::EmbedQuantizedTextureWatermark(UTexture2D* HostTexture, UTexture2D* WatermarkTexture)
{
	ProcessTextureWatermarkEmbedding(HostTexture, WatermarkTexture, UWatermarkAlgorithmLibrary::QuantizedLSBEmbed);
}

UTexture2D* UWatermarkEditorFunctionLibrary::ExtractQuantizedTextureWatermark(UTexture2D* WatermarkedTexture)
{
	return ProcessTextureWatermarkExtraction(WatermarkedTexture, UWatermarkAlgorithmLibrary::QuantizedLSBExtract);
}

void UWatermarkEditorFunctionLibrary::EmbedTextureWatermarkRGBThreshold(UTexture2D* HostTexture, UTexture2D* WatermarkTexture)
{
	ProcessTextureWatermarkEmbedding(HostTexture, WatermarkTexture, UWatermarkAlgorithmLibrary::RGBThresholdLSBEmbed);
}

UTexture2D* UWatermarkEditorFunctionLibrary::ExtractTextureWatermarkRGBThreshold(UTexture2D* WatermarkedTexture)
{
	return ProcessTextureWatermarkExtraction(WatermarkedTexture, UWatermarkAlgorithmLibrary::RGBThresholdLSBExtract);
}

#pragma endregion TextureWatermark

#pragma region StaticMeshWatermark

void UWatermarkEditorFunctionLibrary::EmbedStaticMeshVertexPatternWatermark(UStaticMesh* StaticMesh,
	const FString& Seed, const FString& WatermarkPattern, int32 VertexCount)
{
	if (!StaticMesh || Seed.IsEmpty() || WatermarkPattern.IsEmpty())
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("UWatermarkEditorFunctionLibrary::EmbedStaticMeshVertexPatternWatermark: Invalid parameters"));
		return;
	}

	FString NewWatermarkPattern = FString::Printf(TEXT("%s%d"), *WatermarkPattern, 1);
	
	float WatermarkValue = FCString::Atof(*FString::Printf(TEXT("0.%s"), *NewWatermarkPattern));
	if (WatermarkValue == 0.f)
	{
		UE_LOG(LogWatermarkEditor, Warning, TEXT("UWatermarkEditorFunctionLibrary::EmbedStaticMeshVertexPatternWatermark: Watermark value evaluated to 0"));
	}

	StaticMesh->Modify();
	FMeshDescription* MeshDesc = StaticMesh->GetMeshDescription(0);
	if (!MeshDesc)
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("UWatermarkEditorFunctionLibrary::EmbedStaticMeshVertexPatternWatermark: MeshDescription is missing"));
		return;
	}

	FStaticMeshAttributes Attributes(*MeshDesc);
	TVertexAttributesRef<FVector3f> Positions = Attributes.GetVertexPositions();

	TArray<FVertexID> VertexIDs;
	for (const FVertexID& ID : MeshDesc->Vertices().GetElementIDs())
		VertexIDs.Add(ID);

	if (VertexIDs.Num() < VertexCount)
	{
		UE_LOG(LogWatermarkEditor, Warning, TEXT("UWatermarkEditorFunctionLibrary::EmbedStaticMeshVertexPatternWatermark: Not enough vertices"));
		VertexCount = VertexIDs.Num();
	}

	FRandomStream RNG(GetSeedFromString(Seed));
	for (int32 i = 0; i < VertexCount; ++i)
	{
		int32 Index = RNG.RandRange(0, VertexIDs.Num() - 1);
		FVertexID VtxID = VertexIDs[Index];
		FVector3f Pos = Positions[VtxID];

		int32 IntPart = FMath::FloorToInt(Pos.X);
		Pos.X = IntPart + WatermarkValue;

		Positions[VtxID] = Pos;
	}

	StaticMesh->CommitMeshDescription(0);
	StaticMesh->Build(false);
	bool _ = StaticMesh->MarkPackageDirty();
	SaveAsset(StaticMesh);
	
	UE_LOG(LogWatermarkEditor, Log, TEXT("UWatermarkEditorFunctionLibrary::EmbedStaticMeshVertexPatternWatermark: Watermark '%s' embedded in %d vertices"), *WatermarkPattern, VertexCount);
}

FString UWatermarkEditorFunctionLibrary::ExtractStaticMeshVertexPatternWatermark(UStaticMesh* StaticMesh,
	const FString& Seed, int32 VertexCount, int32 DecimalDigits)
{
	if (!StaticMesh || Seed.IsEmpty() || DecimalDigits <= 0)
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("UWatermarkEditorFunctionLibrary::ExtractStaticMeshVertexPatternWatermark: Invalid parameters"));
		return FString();
	}

	FMeshDescription* MeshDesc = StaticMesh->GetMeshDescription(0);
	if (!MeshDesc)
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("UWatermarkEditorFunctionLibrary::ExtractStaticMeshVertexPatternWatermark: MeshDescription missing"));
		return FString();
	}

	FStaticMeshAttributes Attributes(*MeshDesc);
	TVertexAttributesConstRef<FVector3f> Positions = Attributes.GetVertexPositions();

	TArray<FVertexID> VertexIDs;
	for (const FVertexID& ID : MeshDesc->Vertices().GetElementIDs())
		VertexIDs.Add(ID);

	if (VertexIDs.Num() < VertexCount)
	{
		VertexCount = VertexIDs.Num();
	}

	FRandomStream RNG(GetSeedFromString(Seed));
	FString Collected;

	for (int32 i = 0; i < VertexCount; ++i)
	{
		int32 Index = RNG.RandRange(0, VertexIDs.Num() - 1);
		FVertexID VtxID = VertexIDs[Index];
		FVector3f Pos = Positions[VtxID];

		float Fraction = FMath::Abs(Pos.X - FMath::FloorToFloat(Pos.X));
		FString AsString = FString::Printf(TEXT("%.10f"), Fraction);
		int32 DotIndex;
		if (AsString.FindChar('.', DotIndex))
		{
			FString Decimals = AsString.Mid(DotIndex + 1, DecimalDigits);
			Collected += FString::Printf(TEXT("\n %s"), *Decimals);
		}
	}

	UE_LOG(LogWatermarkEditor, Log, TEXT("UWatermarkEditorFunctionLibrary::ExtractStaticMeshVertexPatternWatermark: Extracted = %s"), *Collected);
	return Collected;
}

bool UWatermarkEditorFunctionLibrary::VerifyStaticMeshVertexPatternWatermark(UStaticMesh* StaticMesh,
    const FString& Seed, const FString& ExpectedPattern, int32 VertexCount, float ConfidenceThreshold)
{
    if (!StaticMesh || Seed.IsEmpty() || ExpectedPattern.IsEmpty() || ConfidenceThreshold <= 0.f)
    {
        UE_LOG(LogWatermarkEditor, Error, TEXT("UWatermarkEditorFunctionLibrary::VerifyStaticMeshVertexPatternWatermark: Invalid parameters"));
        return false;
    }

    FMeshDescription* MeshDesc = StaticMesh->GetMeshDescription(0);
    if (!MeshDesc)
    {
        UE_LOG(LogWatermarkEditor, Error, TEXT("UWatermarkEditorFunctionLibrary::VerifyStaticMeshVertexPatternWatermark: MeshDescription missing"));
        return false;
    }

    FStaticMeshAttributes Attributes(*MeshDesc);
    TVertexAttributesConstRef<FVector3f> Positions = Attributes.GetVertexPositions();

    TArray<FVertexID> VertexIDs;
    for (const FVertexID& ID : MeshDesc->Vertices().GetElementIDs())
        VertexIDs.Add(ID);

    if (VertexIDs.Num() < VertexCount)
        VertexCount = VertexIDs.Num();

    FRandomStream RNG(GetSeedFromString(Seed));
    int32 Matches = 0;

	int32 DecimalDigits = ExpectedPattern.Len();

    const int32 Pow10 = FMath::Pow(10, static_cast<float>(DecimalDigits));

    for (int32 i = 0; i < VertexCount; i++)
    {
        int32 Index = RNG.RandRange(0, VertexIDs.Num() - 1);
        FVertexID VtxID = VertexIDs[Index];
        FVector3f Pos = Positions[VtxID];

        float DecimalPart = FMath::Abs(Pos.X - FMath::FloorToFloat(Pos.X));
        int32 Truncated = static_cast<int32>(DecimalPart * Pow10);

        FString TruncatedStr = FString::Printf(TEXT("%0*d"), DecimalDigits, Truncated);

        if (TruncatedStr == ExpectedPattern)
        {
            Matches++;
        }
    }

    float Confidence = static_cast<float>(Matches) / static_cast<float>(VertexCount);
    UE_LOG(LogWatermarkEditor, Log, TEXT("UWatermarkEditorFunctionLibrary::VerifyStaticMeshVertexPatternWatermark: Match ratio = %.2f%% (%d/%d)"), Confidence * 100.0f, Matches, VertexCount);

    return Confidence >= ConfidenceThreshold;
}

#pragma endregion StaticMeshWatermark

bool UWatermarkEditorFunctionLibrary::SaveAsset(UObject* AssetToSave)
{
	UPackage* Package = AssetToSave->GetPackage();
	if (!Package)
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("UWatermarkEditorFunctionLibrary::SaveAsset - Failed to get package from asset %s"), *AssetToSave->GetName());
		return true;
	}

	Package->Modify();
	bool _ = AssetToSave->MarkPackageDirty();

	FString PackageFilePath = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());

	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.Error = GError;

	if (UPackage::SavePackage(Package, AssetToSave, *PackageFilePath, SaveArgs))
	{
		UE_LOG(LogWatermarkEditor, Log, TEXT("UWatermarkEditorFunctionLibrary::SaveAsset - Asset %s correctly saved"), *AssetToSave->GetName());
		return true;		
	}
	
	UE_LOG(LogWatermarkEditor, Error, TEXT("UWatermarkEditorFunctionLibrary::SaveAsset - Failed to save package to %s"), *PackageFilePath);
	return false;
}

int32 UWatermarkEditorFunctionLibrary::GetSeedFromString(const FString& Seed)
{
	return static_cast<int32>(FCrc::StrCrc32(*Seed));
}