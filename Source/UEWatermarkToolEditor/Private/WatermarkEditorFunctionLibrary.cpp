// Fill out your copyright notice in the Description page of Project Settings.

#include "WatermarkEditorFunctionLibrary.h"

#include "EditorAssetLibrary.h"
#include "FileHelpers.h"
#include "ImageUtils.h"
#include "UEWatermarkToolEditor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"

UTexture2D* UWatermarkEditorFunctionLibrary::CreateDebugVisibleWatermarkedTexture(UTexture2D* Host, UTexture2D* Watermark, const FString& InPackagePath, const FString& InAssetName, bool bOverwriteOriginal)
{
	if (!Host || !Watermark)
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("CreateDebugVisibleWatermarkedTexture - Invalid textures"));
		return nullptr;
	}

	FString PackagePath = InPackagePath;
	FString AssetName = InAssetName;

	if (PackagePath.IsEmpty())
	{
		FString FullName = Host->GetOutermost()->GetName();
		int32 LastSlash;
		if (FullName.FindLastChar('/', LastSlash))
			PackagePath = FullName.Left(LastSlash);
		else
			PackagePath = "/Game";
	}

	if (AssetName.IsEmpty())
	{
		AssetName = Host->GetName();
	}

	FTexture2DMipMap& HostMip = Host->GetPlatformData()->Mips[0];
	FTexture2DMipMap& WmMip = Watermark->GetPlatformData()->Mips[0];

	int32 HostW = HostMip.SizeX;
	int32 HostH = HostMip.SizeY;
	int32 WmW = WmMip.SizeX;
	int32 WmH = WmMip.SizeY;

	TArray<FColor> WmPixels;
	if (WmW > HostW || WmH > HostH)
	{
		TArray<FColor> Src;
		Src.SetNum(WmW * WmH);
		FColor* RawWm = static_cast<FColor*>(WmMip.BulkData.Lock(LOCK_READ_ONLY));
		FMemory::Memcpy(Src.GetData(), RawWm, Src.Num() * sizeof(FColor));
		WmMip.BulkData.Unlock();

		int32 TargetW = FMath::Min(WmW, HostW);
		int32 TargetH = FMath::Min(WmH, HostH);
		FImageUtils::ImageResize(WmW, WmH, Src, TargetW, TargetH, WmPixels, true);

		WmW = TargetW;
		WmH = TargetH;
	}
	else
	{
		FColor* RawWm = static_cast<FColor*>(WmMip.BulkData.Lock(LOCK_READ_ONLY));
		WmPixels.SetNum(WmW * WmH);
		FMemory::Memcpy(WmPixels.GetData(), RawWm, WmW * WmH * sizeof(FColor));
		WmMip.BulkData.Unlock();
	}

	FColor* HostPixels = static_cast<FColor*>(HostMip.BulkData.Lock(LOCK_READ_ONLY));
	TArray<FColor> OutPixels;
	OutPixels.SetNum(HostW * HostH);
	FMemory::Memcpy(OutPixels.GetData(), HostPixels, HostW * HostH * sizeof(FColor));
	HostMip.BulkData.Unlock();

	const int32 StartX = HostW - WmW;
	const int32 StartY = HostH - WmH;

	for (int32 y = 0; y < WmH; ++y)
	{
		for (int32 x = 0; x < WmW; ++x)
		{
			const int32 wmIdx = y * WmW + x;
			const int32 hostIdx = (StartY + y) * HostW + (StartX + x);

			const FColor& wmColor = WmPixels[wmIdx];
			if (wmColor.R + wmColor.G + wmColor.B < 750)
			{
				OutPixels[hostIdx] = FColor::Black;
			}
		}
	}

	UPackage* Package = CreatePackage(*FPaths::Combine(PackagePath, AssetName));
	UTexture2D* NewTex = nullptr;

	if (bOverwriteOriginal)
	{
		NewTex = Host;
		NewTex->Modify();
		NewTex->GetPlatformData()->Mips.Empty();
	}
	else
	{
		NewTex = NewObject<UTexture2D>(Package, *AssetName, RF_Public | RF_Standalone);
		NewTex->AddToRoot();
		NewTex->SetPlatformData(new FTexturePlatformData());
		NewTex->GetPlatformData()->SizeX = HostW;
		NewTex->GetPlatformData()->SizeY = HostH;
		NewTex->GetPlatformData()->PixelFormat = PF_B8G8R8A8;
	}

	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	Mip->SizeX = HostW;
	Mip->SizeY = HostH;
	Mip->BulkData.Lock(LOCK_READ_WRITE);
	void* Data = Mip->BulkData.Realloc(HostW * HostH * sizeof(FColor));
	FMemory::Memcpy(Data, OutPixels.GetData(), HostW * HostH * sizeof(FColor));
	Mip->BulkData.Unlock();

	NewTex->Source.Init(
		HostW,
		HostH,
		1,
		1,
		TSF_BGRA8,
		(uint8*)OutPixels.GetData()
	);

	NewTex->GetPlatformData()->Mips.Add(Mip);
	NewTex->SRGB = true;
	NewTex->MipGenSettings = TMGS_NoMipmaps;
	NewTex->CompressionSettings = TC_Default;
	NewTex->UpdateResource();

	FAssetRegistryModule::AssetCreated(NewTex);
	NewTex->MarkPackageDirty();

	FString PackageFilePath = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
	UPackage::SavePackage(Package, NewTex, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFilePath);

	UE_LOG(LogWatermarkEditor, Log, TEXT("CreateDebugVisibleWatermarkedTexture - Texture %s saved to %s"), *AssetName, *PackageFilePath);

	return NewTex;
}

UTexture2D* UWatermarkEditorFunctionLibrary::BlendTextures(UTexture2D* Base, UTexture2D* Overlay, float Alpha)
{
	if (!Base || !Overlay || Alpha < 0.f || Alpha > 1.f)
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("BlendTextures - Invalid input"));
		return nullptr;
	}

	TArray<FColor> BaseColors, OverlayColors;
	int32 BaseW, BaseH, OverlayW, OverlayH;

	if (!ReadTexturePixels(Base, BaseColors, BaseW, BaseH) ||
		!ReadTexturePixels(Overlay, OverlayColors, OverlayW, OverlayH))
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("BlendTextures - Failed to read texture data"));
		return nullptr;
	}

	if (OverlayW >= BaseW || OverlayH >= BaseH)
	{
		TArray<FColor> ResizedOverlay;
		ResizePixels(OverlayColors, OverlayW, OverlayH, BaseW, BaseH, ResizedOverlay);
		OverlayColors = MoveTemp(ResizedOverlay);
	}

	TArray<FColor> ResultColors = BaseColors;

	for (int32 i = 0; i < BaseColors.Num(); ++i)
	{
		const FColor& A = BaseColors[i];
		const FColor& B = OverlayColors[i];

		uint8 R = FMath::Lerp(A.R, B.R, Alpha);
		uint8 G = FMath::Lerp(A.G, B.G, Alpha);
		uint8 Bc = FMath::Lerp(A.B, B.B, Alpha);
		uint8 AOut = FMath::Lerp(A.A, B.A, Alpha);

		ResultColors[i] = FColor(R, G, Bc, AOut);
	}

	UTexture2D* OutTex = CreateTransientTextureFromPixels(ResultColors, BaseW, BaseH);

	if (OutTex)
	{
		UE_LOG(LogWatermarkEditor, Log, TEXT("BlendTextures - Blended %dx%d overlay"), OverlayW, OverlayH);
	}

	return OutTex;
}

bool UWatermarkEditorFunctionLibrary::SaveAsset(UObject* AssetToSave)
{
	UPackage* Package = AssetToSave->GetOutermost();
	if (!Package)
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("EmbedQuantizedWatermark - Failed to get package from texture"));
		return true;
	}

	Package->Modify();
	//HostTexture->Modify();
	AssetToSave->MarkPackageDirty();

	FString PackageFilePath = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());

	if (UPackage::SavePackage(Package, AssetToSave, RF_Public | RF_Standalone, *PackageFilePath))
	{
		//UE_LOG(LogWatermarkEditor, Log, TEXT("EmbedQuantizedWatermark - Watermark embedded and saved to disk [%d x %d] using 3-bit LSB"), TargetWidth, TargetHeight);
	}
	else
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("EmbedQuantizedWatermark - Failed to save package to %s"), *PackageFilePath);
	}
	return false;
}

void UWatermarkEditorFunctionLibrary::EmbedQuantizedWatermark(UTexture2D* HostTexture, UTexture2D* WatermarkTexture)
{
	if (!HostTexture || !WatermarkTexture)
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("EmbedQuantizedWatermark - Invalid textures"));
		return;
	}

	FTexture2DMipMap& HostMip = HostTexture->GetPlatformData()->Mips[0];
	uint8* HostPixels = static_cast<uint8*>(HostMip.BulkData.Lock(LOCK_READ_WRITE));
	const int32 HostWidth = HostMip.SizeX;
	const int32 HostHeight = HostMip.SizeY;

	TArray<FColor> WmColors;
	int32 WmWidth, WmHeight;
	if (!ReadTexturePixels(WatermarkTexture, WmColors, WmWidth, WmHeight))
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("EmbedQuantizedWatermark - Failed to read watermark pixels"));
		HostMip.BulkData.Unlock();
		return;
	}

	int32 TargetWidth = FMath::Min(WmWidth, HostWidth);
	int32 TargetHeight = FMath::Min(WmHeight, HostHeight);
	TArray<FColor> ResizedColors;

	if (WmWidth > HostWidth || WmHeight > HostHeight)
	{
		ResizePixels(WmColors, WmWidth, WmHeight, TargetWidth, TargetHeight, ResizedColors);
	}
	else
	{
		ResizedColors = MoveTemp(WmColors);
	}

	TArray<uint8> ResizedWmData;
	ResizedWmData.SetNum(TargetWidth * TargetHeight);
	for (int32 i = 0; i < ResizedColors.Num(); ++i)
	{
		ResizedWmData[i] = ResizedColors[i].R;
	}

	const int32 StartX = HostWidth - TargetWidth;
	const int32 StartY = HostHeight - TargetHeight;

	for (int32 y = 0; y < TargetHeight; ++y)
	{
		for (int32 x = 0; x < TargetWidth; ++x)
		{
			const int32 HostIdx = (StartY + y) * HostWidth + (StartX + x);
			const int32 WmIdx = y * TargetWidth + x;

			uint8 f = HostPixels[HostIdx];
			uint8 w = ResizedWmData[WmIdx];
			uint8 scaledW = FMath::Clamp(w >> 5, 0, 7); // 3-bit LSB
			uint8 fw = 8 * (f / 8) + scaledW;

			HostPixels[HostIdx] = fw;
		}
	}

	HostMip.BulkData.Unlock();

	HostTexture->Source.Init(
		HostWidth,
		HostHeight,
		1,
		1,
		TSF_BGRA8,
		HostPixels
	);

	HostTexture->UpdateResource();

	if (SaveAsset(HostTexture)) return;

	UE_LOG(LogWatermarkEditor, Log, TEXT("EmbedQuantizedWatermark - Watermark embedded [%d x %d] using 3-bit LSB"), TargetWidth, TargetHeight);
}

UTexture2D* UWatermarkEditorFunctionLibrary::ExtractQuantizedWatermark(UTexture2D* WatermarkedTexture)
{
	if (!WatermarkedTexture)
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("ExtractQuantizedWatermark - Invalid texture"));
		return nullptr;
	}

	FTexture2DMipMap& Mip = WatermarkedTexture->GetPlatformData()->Mips[0];
	const int32 Width = Mip.SizeX;
	const int32 Height = Mip.SizeY;

	uint8* Pixels = static_cast<uint8*>(Mip.BulkData.Lock(LOCK_READ_ONLY));
	if (!Pixels)
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("ExtractQuantizedWatermark - Failed to lock input texture"));
		return nullptr;
	}

	TArray<uint8> OutData;
	OutData.SetNum(Width * Height);

	for (int32 i = 0; i < Width * Height; ++i)
	{
		uint8 fw = Pixels[i];
		uint8 w = (fw & 0x07) * 36; // 3-bit LSB → 0, 36, ..., 252
		OutData[i] = w;
	}

	Mip.BulkData.Unlock();

	UTexture2D* OutTex = UTexture2D::CreateTransient(Width, Height, PF_G8);
	if (!OutTex)
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("ExtractQuantizedWatermark - Failed to create output texture"));
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

	UE_LOG(LogWatermarkEditor, Log, TEXT("ExtractQuantizedWatermark - Extracted watermark using 3-bit LSB [%d x %d]"), Width, Height);

	return OutTex;
}

bool UWatermarkEditorFunctionLibrary::ReadTexturePixels(UTexture2D* Texture, TArray<FColor>& OutPixels, int32& OutWidth, int32& OutHeight)
{
	if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0)
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("ReadTexturePixels - Invalid texture or platform data"));
		return false;
	}

	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	OutWidth = Mip.SizeX;
	OutHeight = Mip.SizeY;

	FColor* Src = static_cast<FColor*>(Mip.BulkData.Lock(LOCK_READ_ONLY));
	if (!Src)
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("ReadTexturePixels - Failed to lock mip data"));
		return false;
	}

	OutPixels.SetNum(OutWidth * OutHeight);
	FMemory::Memcpy(OutPixels.GetData(), Src, OutWidth * OutHeight * sizeof(FColor));
	Mip.BulkData.Unlock();

	return true;
}

void UWatermarkEditorFunctionLibrary::ResizePixels(const TArray<FColor>& Src, int32 SrcW, int32 SrcH, int32 DestW, int32 DestH, TArray<FColor>& Out)
{
	FImageUtils::ImageResize(SrcW, SrcH, Src, DestW, DestH, Out, true);
}

UTexture2D* UWatermarkEditorFunctionLibrary::CreateTransientTextureFromPixels(const TArray<FColor>& Pixels, int32 Width, int32 Height)
{
	UTexture2D* OutTex = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	if (!OutTex)
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("CreateTransientTextureFromPixels - Failed to create texture"));
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
