// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class ABlasterCharacter;
class ABlasterPlayerController;

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector BoxLocation;

	UPROPERTY()
	FRotator BoxRotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	TObjectPtr<ABlasterCharacter> Character;
	
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TObjectPtr<ABlasterCharacter>, uint32> HeadShots;

	UPROPERTY()
	TMap<TObjectPtr<ABlasterCharacter>, uint32> BodyShots;

};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TEST_MULTISHOOTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	friend class ABlasterCharacter;
	ULagCompensationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

	/** 
	* Hitscan
	*/
	// 服务器进行滞后补偿
	FServerSideRewindResult ServerSideRewind(
		ABlasterCharacter* HitCharacter, 
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize& HitLocation, 
		float HitTime);

	/** 
	* Shotgun
	*/
	// 用于霰弹枪
	FShotgunServerSideRewindResult ShotgunServerSideRewind(
		const TArray<ABlasterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);
	
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime
	);

	FFramePackage GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime);

	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(
		const TArray<ABlasterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime
	);
	
protected:
	virtual void BeginPlay() override;

	// 保存当前帧的角色所有 Box 的位置，旋转，缩放
	void SaveFramePackage(FFramePackage& Package);

	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

	void CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	
	void SaveFramePackage();

	/** 
	* Hitscan
	*/
	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package,
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation);

	/** 
	* Shotgun
	*/

	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations
	);
	
private:
	UPROPERTY()
	TObjectPtr<ABlasterCharacter> Character;

	UPROPERTY()
	TObjectPtr<ABlasterPlayerController> Controller;

	// 用双向链表保存每帧的历史数据
	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;
	
};
