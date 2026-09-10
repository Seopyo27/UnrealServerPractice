// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPC.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "MyPlayerCameraManager.h"
#include "UI/LoginWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Network/ServerDirectorySubsystem.h"

AMyPC::AMyPC()
{
	PlayerCameraManagerClass = AMyPlayerCameraManager::StaticClass();
}

void AMyPC::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (!InputMapping.IsNull())
			{
				InputSystem->AddMappingContext(InputMapping.LoadSynchronous(), 0);
			}
		}
	}

	bool bShouldShowLogin = true;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UServerDirectorySubsystem* Directory = GI->GetSubsystem<UServerDirectorySubsystem>())
		{
			bShouldShowLogin = !Directory->HasAuthenticatedSession();
		}
	}

	if (!IsRunningDedicatedServer() && IsLocalController() && bShouldShowLogin && LoginWidgetClass)
	{
		LoginWidget = CreateWidget<ULoginWidget>(this, LoginWidgetClass);
		if (LoginWidget)
		{
			LoginWidget->AddToViewport(100);
			FInputModeUIOnly InputMode;
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
	}
}
