// Fill out your copyright notice in the Description page of Project Settings.

#include "WatermarkEditorFunctionLibrary.h"
#include "ImageUtils.h"
#include "UEWatermarkToolEditor.h"
#include "AssetRegistry/AssetRegistryModule.h"

UTexture2D* UWatermarkEditorFunctionLibrary::CreateDebugVisibleWatermarkedTexture(UTexture2D* Host, UTexture2D* Watermark, const FString& InPackagePath, const FString& InAssetName, bool bOverwriteOriginal)
{
	if (!Host || !Watermark)
	{
		UE_LOG(LogWatermarkEditor, Error, TEXT("CreateDebugVisibleWatermarkedTexture - Invalid textures"));
		return nullptr;
	}

	// Path fallback
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

	// Read and optionally resize watermark
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

	// Read host
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

	// Asset creation
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

	// ...dopo avere settato PlatformData, Mips, ecc

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
