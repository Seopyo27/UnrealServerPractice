#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoginWidget.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;

/** C++ login logic. WBP_Login is used only for the visual layout. */
UCLASS(Abstract, BlueprintType)
class L20260713_DAY03_API ULoginWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> UserIdInput;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UEditableTextBox> PasswordInput;

	// Backwards compatibility with an earlier WBP_Login layout.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UEditableTextBox> TokenInput;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> LoginButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> StatusText;

private:
	UFUNCTION()
	void HandleLoginClicked();

	UFUNCTION()
	void HandleLoginResult(bool bSuccess, const FString& ServerAddress, const FString& Message);

	void ConnectAfterDelay();

	void SetStatus(const FString& Message);

	FString ResolvedServerAddress;
	FTimerHandle ConnectTimerHandle;
};
