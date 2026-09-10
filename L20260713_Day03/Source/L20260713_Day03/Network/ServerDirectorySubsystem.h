#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "ServerDirectorySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FServerDirectorySimpleResult, bool, bSuccess, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FServerDirectoryLoginResult, bool, bSuccess, const FString&, ServerAddress, const FString&, Message);

UCLASS(Config=GameInstance, BlueprintType)
class L20260713_DAY03_API UServerDirectorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Server Directory") FString BaseUrl = TEXT("http://127.0.0.1:8080");
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Server Directory") FString RegisterEndpoint = TEXT("/api/servers/register");
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Server Directory") FString LoginEndpoint = TEXT("/api/login");
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Server Directory") FString ServerId = TEXT("default-server");
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Server Directory") int32 ServerPort = 7777;

	UPROPERTY(BlueprintAssignable, Category="Server Directory") FServerDirectorySimpleResult OnServerRegistered;
	UPROPERTY(BlueprintAssignable, Category="Server Directory") FServerDirectoryLoginResult OnLoginAndServerResolved;

	UFUNCTION(BlueprintCallable, Category="Server Directory") void RegisterServer();
	UFUNCTION(BlueprintCallable, Category="Server Directory") void LoginAndConnect(const FString& UserId, const FString& Password);
	void ConnectToServer(const FString& ServerAddress);
	bool HasAuthenticatedSession() const { return bHasAuthenticatedSession; }

private:
	void HandleRegisterResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void HandleLoginResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void BroadcastRegisterFailure(const FString& Message);
	void BroadcastLoginFailure(const FString& Message);
	FString MakeUrl(const FString& Endpoint) const;

	bool bHasAuthenticatedSession = false;
};
