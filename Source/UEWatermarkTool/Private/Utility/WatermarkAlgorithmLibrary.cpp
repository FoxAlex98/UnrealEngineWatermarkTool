// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/WatermarkAlgorithmLibrary.h"

void UWatermarkAlgorithmLibrary::QuantizedLSBEmbed(uint8* HostPixels, int32 HostWidth, int32 HostHeight, const TArray<FColor>& ResizedColors, int32 TargetWidth, int32 TargetHeight)
{
	const int32 StartX = HostWidth - TargetWidth;
	const int32 StartY = HostHeight - TargetHeight;

	for (int32 y = 0; y < TargetHeight; ++y)
	{
		for (int32 x = 0; x < TargetWidth; ++x)
		{
			const int32 HostIdx = (StartY + y) * HostWidth + (StartX + x);
			const int32 WmIdx = y * TargetWidth + x;

			uint8 f = HostPixels[HostIdx];
			uint8 w = ResizedColors[WmIdx].R;
			uint8 scaledW = FMath::Clamp(w >> 5, 0, 7);
			uint8 fw = 8 * (f / 8) + scaledW;

			HostPixels[HostIdx] = fw;
		}
	}
}

void UWatermarkAlgorithmLibrary::QuantizedLSBExtract(uint8* Pixels, int32 Width, int32 Height, TArray<FColor>& OutPixels)
{
	for (int32 i = 0; i < Width * Height; ++i)
	{
		uint8 fw = Pixels[i];
		uint8 w = (fw & 0x07) * 36;
		OutPixels[i] = FColor(w, w, w, 255);
	}
}

void UWatermarkAlgorithmLibrary::RGBThresholdLSBEmbed(uint8* HostPixels, int32 HostWidth, int32 HostHeight, const TArray<FColor>& ResizedColors, int32 TargetWidth, int32 TargetHeight)
{
	const int32 StartX = HostWidth - TargetWidth;
	const int32 StartY = HostHeight - TargetHeight;

	for (int32 y = 0; y < TargetHeight; ++y)
	{
		for (int32 x = 0; x < TargetWidth; ++x)
		{
			const int32 HostIdx = (StartY + y) * HostWidth + (StartX + x);
			const int32 WmIdx = y * TargetWidth + x;

			const FColor& wm = ResizedColors[WmIdx];
			uint8 Rbit = wm.R > 127 ? 1 : 0;
			uint8 Gbit = wm.G > 127 ? 1 : 0;
			uint8 Bbit = wm.B > 127 ? 1 : 0;

			uint8& R = HostPixels[HostIdx * 4 + 2];
			uint8& G = HostPixels[HostIdx * 4 + 1];
			uint8& B = HostPixels[HostIdx * 4 + 0];

			R = (R & ~1) | Rbit;
			G = (G & ~1) | Gbit;
			B = (B & ~1) | Bbit;
		}
	}
}

void UWatermarkAlgorithmLibrary::RGBThresholdLSBExtract(uint8* Pixels, int32 Width, int32 Height, TArray<FColor>& OutPixels)
{
	for (int32 i = 0; i < Width * Height; ++i)
	{
		const int32 Index = i * 4;
		uint8 Rbit = Pixels[Index + 2] & 1;
		uint8 Gbit = Pixels[Index + 1] & 1;
		uint8 Bbit = Pixels[Index + 0] & 1;
		OutPixels[i] = FColor(Rbit * 255, Gbit * 255, Bbit * 255, 255);
	}
}
