// Fill out your copyright notice in the Description page of Project Settings.


#include "EasterEggUtility/EasterEggFunctionLibrary.h"

TArray<TCHAR> UEasterEggFunctionLibrary::ConvertToMorse(const FString& InMessage)
{
	static const TMap<TCHAR, FString> MorseMap = {
		{ 'A', TEXT(".-") }, { 'B', TEXT("-...") }, { 'C', TEXT("-.-.") }, { 'D', TEXT("-..") },
		{ 'E', TEXT(".") }, { 'F', TEXT("..-.") }, { 'G', TEXT("--.") }, { 'H', TEXT("....") },
		{ 'I', TEXT("..") }, { 'J', TEXT(".---") }, { 'K', TEXT("-.-") }, { 'L', TEXT(".-..") },
		{ 'M', TEXT("--") }, { 'N', TEXT("-.") }, { 'O', TEXT("---") }, { 'P', TEXT(".--.") },
		{ 'Q', TEXT("--.-") }, { 'R', TEXT(".-.") }, { 'S', TEXT("...") }, { 'T', TEXT("-") },
		{ 'U', TEXT("..-") }, { 'V', TEXT("...-") }, { 'W', TEXT(".--") }, { 'X', TEXT("-..-") },
		{ 'Y', TEXT("-.--") }, { 'Z', TEXT("--..") }, { ' ', TEXT("/") }
	};

	TArray<TCHAR> Output;
	for (TCHAR Char : InMessage.ToUpper())
	{
		if (const FString* Morse = MorseMap.Find(Char))
		{
			for (TCHAR Symbol : *Morse)
			{
				Output.Add(Symbol);
			}
			Output.Add(' '); // It need a space between characters
		}
	}
	return Output;
}
