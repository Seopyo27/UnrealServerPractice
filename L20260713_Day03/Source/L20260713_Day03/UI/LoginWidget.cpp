#include "LoginWidget.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Network/ServerDirectorySubsystem.h"
#include "TimerManager.h"

void ULoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (LoginButton)
	{
		LoginButton->OnClicked.AddDynamic(this, &ULoginWidget::HandleLoginClicked);
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UServerDirectorySubsystem* Directory = GI->GetSubsystem<UServerDirectorySubsystem>())
		{
			Directory->OnLoginAndServerResolved.AddDynamic(this, &ULoginWidget::HandleLoginResult);
		}
	}
}

void ULoginWidget::HandleLoginClicked()
{
	UEditableTextBox* PasswordBox = PasswordInput ? PasswordInput.Get() : TokenInput.Get();
	if (!UserIdInput || !PasswordBox)
	{
		SetStatus(TEXT("로그인 위젯 입력 컨트롤이 없습니다."));
		return;
	}

	const FString UserId = UserIdInput->GetText().ToString();
	const FString Password = PasswordBox->GetText().ToString();
	if (UserId.IsEmpty() || Password.IsEmpty())
	{
		SetStatus(TEXT("아이디와 비밀번호를 입력하세요."));
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UServerDirectorySubsystem* Directory = GI->GetSubsystem<UServerDirectorySubsystem>())
		{
			SetStatus(TEXT("로그인 중..."));
			if (LoginButton) LoginButton->SetIsEnabled(false);
			Directory->LoginAndConnect(UserId, Password);
			return;
		}
	}

	SetStatus(TEXT("ServerDirectorySubsystem을 찾을 수 없습니다."));
}

void ULoginWidget::HandleLoginResult(bool bSuccess, const FString& ServerAddress, const FString& Message)
{
	SetStatus(Message);
	if (!bSuccess)
	{
		if (LoginButton) LoginButton->SetIsEnabled(true);
		return;
	}

	ResolvedServerAddress = ServerAddress;
	GetWorld()->GetTimerManager().SetTimer(ConnectTimerHandle, this, &ULoginWidget::ConnectAfterDelay, 5.0f, false);
}

void ULoginWidget::ConnectAfterDelay()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}

	RemoveFromParent();
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UServerDirectorySubsystem* Directory = GI->GetSubsystem<UServerDirectorySubsystem>())
		{
			Directory->ConnectToServer(ResolvedServerAddress);
		}
	}
}

void ULoginWidget::SetStatus(const FString& Message)
{
	if (StatusText) StatusText->SetText(FText::FromString(Message));
}
