// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionSubsystem.h"

#include "OnlineSubsystem.h"

UMultiplayerSessionSubsystem::UMultiplayerSessionSubsystem():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		SessionInterface = Subsystem->GetSessionInterface();
	}
	
}

void UMultiplayerSessionSubsystem::CreateSession(int32 NumPublicConnection, FString MatchType)
{
	
}

void UMultiplayerSessionSubsystem::FindSession(int32 MaxSearchResult)
{
	
}

void UMultiplayerSessionSubsystem::JoinSession(const FOnlineSessionSearchResult& Result)
{
	
}

void UMultiplayerSessionSubsystem::DestroySession()
{
	
}

void UMultiplayerSessionSubsystem::StartSession()
{
	
}

void UMultiplayerSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	
}

void UMultiplayerSessionSubsystem::OnFindSessionComplete(bool bWasSuccessful)
{
	
}

void UMultiplayerSessionSubsystem::OnJoinSessionComplete(FName SessionName,
	EOnJoinSessionCompleteResult::Type Result)
{
	
}

void UMultiplayerSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	
}

void UMultiplayerSessionSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	
}
