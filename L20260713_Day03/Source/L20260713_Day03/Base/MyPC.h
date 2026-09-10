// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPC.generated.h"


class UInputMappingContext;
class ULoginWidget;
/**
 * 
 */
UCLASS()
class L20260713_DAY03_API AMyPC : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMyPC();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> InputMapping;

	/** Set this in BP_MyPC to the WBP_Login child of ULoginWidget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Login")
	TSubclassOf<ULoginWidget> LoginWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Login")
	TObjectPtr<ULoginWidget> LoginWidget;
};
