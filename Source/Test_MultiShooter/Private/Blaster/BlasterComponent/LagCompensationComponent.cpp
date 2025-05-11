// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/BlasterComponent/LagCompensationComponent.h"

#include "Blaster/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Test_MultiShooter/Test_MultiShooter.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	FFramePackage FramePackage;
	SaveFramePackage(FramePackage);
	ShowFramePackage(FramePackage, FColor::Red);
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? TObjectPtr<ABlasterCharacter>(Cast<ABlasterCharacter>(Character)) : Character;
	if (Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		Package.Character = Character;
		
		for (auto& BoxPair:Character->HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;
			BoxInformation.BoxLocation = BoxPair.Value->GetComponentLocation();
			BoxInformation.BoxRotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			
			Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);
		}
	}
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame,
	const FFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);

	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;

	// 遍历角色的每个碰撞盒
	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;
		
		// 通过 Name ，获得这个碰撞盒分别在 Younger 和 Older 上的信息
		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		// 插值
		FBoxInformation InterpBoxInfo;
		InterpBoxInfo.BoxLocation = FMath::VInterpTo(OlderBox.BoxLocation, YoungerBox.BoxLocation, 1.f, InterpFraction);
		InterpBoxInfo.BoxRotation = FMath::RInterpTo(OlderBox.BoxRotation, YoungerBox.BoxRotation, 1.f, InterpFraction);
		// Box 的缩放不会改变，所以不需要插值
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);

	}

	return InterpFramePackage;
}

void ULagCompensationComponent::CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;

	for (auto& HitPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitPair.Value != nullptr)
		{
			FBoxInformation BoxInformation;
			BoxInformation.BoxLocation = HitPair.Value->GetComponentLocation();
			BoxInformation.BoxRotation = HitPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = HitPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitPair.Key, BoxInformation);
		}
	}
}

void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitPair.Value != nullptr)
		{
			HitPair.Value->SetWorldLocation(Package.HitBoxInfo[HitPair.Key].BoxLocation);
			HitPair.Value->SetWorldRotation(Package.HitBoxInfo[HitPair.Key].BoxRotation);
			HitPair.Value->SetBoxExtent(Package.HitBoxInfo[HitPair.Key].BoxExtent);
		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitPair.Value != nullptr)
		{
			HitPair.Value->SetWorldLocation(Package.HitBoxInfo[HitPair.Key].BoxLocation);
			HitPair.Value->SetWorldRotation(Package.HitBoxInfo[HitPair.Key].BoxRotation);
			HitPair.Value->SetBoxExtent(Package.HitBoxInfo[HitPair.Key].BoxExtent);
			HitPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter,
	ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter and HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package,
                                                              ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if (HitCharacter == nullptr) return FServerSideRewindResult();

	FFramePackage CurrentFrame;
	// 缓存当前所有盒体的位置
	CacheBoxPositions(HitCharacter, CurrentFrame);
	// 移动盒体到服务器补偿的位置
	MoveBoxes(HitCharacter, Package);
	// 检测盒体碰撞时需要先关闭模型的碰撞
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	// 开启头部盒体的碰撞
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	FHitResult HitResult;
	FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
	UWorld* World = GetWorld();
	if (World)
	{
		World->LineTraceSingleByChannel(
			HitResult,
			TraceStart,
			TraceEnd,
			ECC_Visibility);
		if (HitResult.bBlockingHit) // 击中头部
		{
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::Type::QueryAndPhysics);
			return FServerSideRewindResult{ true, true};
		}
		else
		{
			// 未击中头部，则开启其它盒体的碰撞，判断是否命中其它部位
			for (auto& HitPair : HitCharacter->HitCollisionBoxes)
			{
				if (HitPair.Value != nullptr)
				{
					HitPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitPair.Value->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
				}
			}
			World->LineTraceSingleByChannel(
						HitResult,
						TraceStart,
						TraceEnd,
				ECC_Visibility);
			if (HitResult.bBlockingHit) // 击中其它部位
			{
				ResetHitBoxes(HitCharacter, CurrentFrame);
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::Type::QueryAndPhysics);
				return FServerSideRewindResult{ true, false};
			}
		}
	}

	// 未命中
	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::Type::QueryAndPhysics);
	return FServerSideRewindResult{ false, true};
	
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages,
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	for (auto& Frame : FramePackages)
	{
		if (Frame.Character == nullptr) return FShotgunServerSideRewindResult();
	}
	
	FShotgunServerSideRewindResult ShotgunResult;
	TArray<FFramePackage> CurrentFrames;
	for (auto& Frame : FramePackages)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.Character = Frame.Character;
		CacheBoxPositions(Frame.Character, CurrentFrame);
		MoveBoxes(Frame.Character, Frame);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::NoCollision);
		CurrentFrames.Add(CurrentFrame);
	}

	for (auto& Frame : FramePackages)
	{
		// Enable collision for the head first
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
	}

	UWorld* World = GetWorld();
	// check for head shots
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if (BlasterCharacter)
			{
				if (ShotgunResult.HeadShots.Contains(BlasterCharacter))
				{
					ShotgunResult.HeadShots[BlasterCharacter]++;
				}
				else
				{
					ShotgunResult.HeadShots.Emplace(BlasterCharacter, 1);
				}
			}
		}
	}

	// enable collision for all boxes, then disable for head box
	for (auto& Frame : FramePackages)
	{
		for (auto& HitBoxPair : Frame.Character->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// check for body shots
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if (BlasterCharacter)
			{
				if (ShotgunResult.BodyShots.Contains(BlasterCharacter))
				{
					ShotgunResult.BodyShots[BlasterCharacter]++;
				}
				else
				{
					ShotgunResult.BodyShots.Emplace(BlasterCharacter, 1);
				}
			}
		}
	}

	for (auto& Frame : CurrentFrames)
	{
		ResetHitBoxes(Frame.Character, Frame);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics);
	}
	
	return ShotgunResult;
}

void ULagCompensationComponent::SaveFramePackage()
{
	if (Character == nullptr || !Character->HasAuthority()) return;
	
	// 帧数据中如果小于一条数据，则这条数据就作为链表的头节点
	if (FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		// 根据链表头和尾节点中保存的时间数据，如果超过指定的 MaxRecordTime 时间，则表示之前节点中的数据时间过长了，不需要保存了，丢弃
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
		
		ShowFramePackage(ThisFrame, FColor::Red);
	}
	
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SaveFramePackage();
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for (auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.BoxLocation,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.BoxRotation),
			Color,
			false,
			4.f
		);
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);

	// 补偿后判断是否命中
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(
	const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> FramesToCheck;
	for (ABlasterCharacter* HitCharacter : HitCharacters)
	{
		FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}

	return ShotgunConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter,
                                                                  const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);
	if (Character && HitCharacter && Character->GetWeapon() && Confirm.bHitConfirmed)
	{
		float Damage = Confirm.bHeadShot ? Character->GetWeapon()->GetHeadShotDamage() : Character->GetWeapon()->GetDamage();
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			Character->Controller,
			Character->GetWeapon(),
			UDamageType::StaticClass()
			);
	}
}

void ULagCompensationComponent::ShotgunServerScoreRequest_Implementation(
	const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	FShotgunServerSideRewindResult Confirm = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);

	for (auto& HitCharacter : HitCharacters)
	{
		if (HitCharacter == nullptr || HitCharacter->GetWeapon() == nullptr || Character == nullptr) continue;
		float TotalDamage = 0.f;
		if (Confirm.HeadShots.Contains(HitCharacter))
		{
			float HeadShotDamage = Confirm.HeadShots[HitCharacter] * Character->GetWeapon()->GetHeadShotDamage();
			TotalDamage += HeadShotDamage;
		}
		if (Confirm.BodyShots.Contains(HitCharacter))
		{
			float BodyShotDamage = Confirm.BodyShots[HitCharacter] * Character->GetWeapon()->GetDamage();
			TotalDamage += BodyShotDamage;
		}
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			TotalDamage,
			Character->Controller,
			HitCharacter->GetWeapon(),
			UDamageType::StaticClass()
		);
	}
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime)
{
	bool bReturned = HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComponent() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr;
	if (bReturned) return FFramePackage();

	// 标识是否需要在两个节点之间插值
	bool bShouldInterpolate = true;

	// FrameToCheck 是用于检查是否命中的包
	FFramePackage FrameToCheck;
	// 初始获得第一个和最后一个包的时间
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	// 第一个包的时间超过指定时间，表示这个包太久远了，不能用于补偿
	if (OldestHistoryTime > HitTime)
	{
		return FFramePackage();
	}

	
	if (OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	
	if (NewestHistoryTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;

	}

	auto Older = History.GetTail();
	auto Younger = Older;
	// 循环双向链表，找到 HitTime 左右的两个包
	while (Older->GetValue().Time > HitTime)
	{
		if (Older->GetNextNode() == nullptr) break;
		
		Older = Older->GetNextNode();
		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}

	// Older 包的时间正好是 HitTime 的时间点，就直接使用 Older 包
	if (Older->GetValue().Time == HitTime) // highly unlikely, but we found our frame to check
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}

	// 判断是否需要在左右两个包之间插值
	if (bShouldInterpolate)
	{
		// Interpolate between Younger and Older
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}
	
	FrameToCheck.Character = HitCharacter;

	return FrameToCheck;
}

