#include "ServerDirectorySubsystem.h"

#include "Dom/JsonObject.h"
#include "GameFramework/PlayerController.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UServerDirectorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (IsRunningDedicatedServer()) RegisterServer();
}

FString UServerDirectorySubsystem::MakeUrl(const FString& Endpoint) const
{
	FString Root = BaseUrl;
	Root.RemoveFromEnd(TEXT("/"));
	FString Path = Endpoint;
	Path.RemoveFromStart(TEXT("/"));
	return Root + TEXT("/") + Path;
}

void UServerDirectorySubsystem::RegisterServer()
{
	if (BaseUrl.IsEmpty() || RegisterEndpoint.IsEmpty()) { BroadcastRegisterFailure(TEXT("Server Directory URL or register endpoint is empty.")); return; }
	TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("server_id"), ServerId);
	Json->SetNumberField(TEXT("port"), ServerPort > 0 ? ServerPort : 7777);
	Json->SetStringField(TEXT("map"), GetWorld() ? GetWorld()->GetMapName() : TEXT(""));
	Json->SetStringField(TEXT("status"), TEXT("online"));
	FString Body;
	FJsonSerializer::Serialize(Json, TJsonWriterFactory<>::Create(&Body));

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(MakeUrl(RegisterEndpoint));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(Body);
	Request->OnProcessRequestComplete().BindUObject(this, &UServerDirectorySubsystem::HandleRegisterResponse);
	Request->ProcessRequest();
}

void UServerDirectorySubsystem::LoginAndConnect(const FString& UserId, const FString& Password)
{
	if (BaseUrl.IsEmpty() || LoginEndpoint.IsEmpty()) { BroadcastLoginFailure(TEXT("Server Directory URL or login endpoint is empty.")); return; }
	TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("user_id"), UserId);
	Json->SetStringField(TEXT("password"), Password);
	FString Body;
	FJsonSerializer::Serialize(Json, TJsonWriterFactory<>::Create(&Body));

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(MakeUrl(LoginEndpoint));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(Body);
	Request->OnProcessRequestComplete().BindUObject(this, &UServerDirectorySubsystem::HandleLoginResponse);
	Request->ProcessRequest();
}

void UServerDirectorySubsystem::HandleRegisterResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	const bool bOk = bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode());
	if (!bOk) { BroadcastRegisterFailure(Response.IsValid() ? FString::Printf(TEXT("Register failed: HTTP %d"), Response->GetResponseCode()) : TEXT("Register request failed.")); return; }
	OnServerRegistered.Broadcast(true, Response->GetContentAsString());
}

void UServerDirectorySubsystem::HandleLoginResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode())) { BroadcastLoginFailure(Response.IsValid() ? FString::Printf(TEXT("Login failed: HTTP %d"), Response->GetResponseCode()) : TEXT("Login request failed.")); return; }
	TSharedPtr<FJsonObject> Json;
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Response->GetContentAsString()), Json) || !Json.IsValid()) { BroadcastLoginFailure(TEXT("Login response is not valid JSON.")); return; }

	FString Address;
	Json->TryGetStringField(TEXT("server_address"), Address);
	if (Address.IsEmpty()) Json->TryGetStringField(TEXT("server_ip"), Address);
	if (Address.IsEmpty()) Json->TryGetStringField(TEXT("ip"), Address);
	double Port = 0;
	Json->TryGetNumberField(TEXT("server_port"), Port);
	if (Port <= 0) Json->TryGetNumberField(TEXT("port"), Port);
	if (!Address.Contains(TEXT(":")) && Port > 0) Address += FString::Printf(TEXT(":%d"), FMath::RoundToInt(Port));
	if (Address.IsEmpty()) { BroadcastLoginFailure(TEXT("Login succeeded, but no server address was returned.")); return; }

	bHasAuthenticatedSession = true;
	OnLoginAndServerResolved.Broadcast(true, Address, TEXT("로그인 성공. 5초 후 게임 서버에 접속합니다."));
}

void UServerDirectorySubsystem::ConnectToServer(const FString& ServerAddress)
{
	if (APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController())
	{
		PC->ClientTravel(ServerAddress, TRAVEL_Absolute);
	}
}

void UServerDirectorySubsystem::BroadcastRegisterFailure(const FString& Message) { UE_LOG(LogTemp, Error, TEXT("%s"), *Message); OnServerRegistered.Broadcast(false, Message); }
void UServerDirectorySubsystem::BroadcastLoginFailure(const FString& Message) { UE_LOG(LogTemp, Error, TEXT("%s"), *Message); OnLoginAndServerResolved.Broadcast(false, TEXT(""), Message); }
