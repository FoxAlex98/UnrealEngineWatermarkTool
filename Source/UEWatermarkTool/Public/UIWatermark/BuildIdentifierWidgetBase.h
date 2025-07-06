// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "BuildIdentifierWidgetBase.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class UEWATERMARKTOOL_API UBuildIdentifierWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* BuildIdentifierText;

	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

protected:

	UFUNCTION(Blueprintpure, Category = "Build Identifier")
	FText GetBuildIdentifierAsText();
};
