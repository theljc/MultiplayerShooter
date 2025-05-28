// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/BlasterCharacter.h"

#include "EnhancedInputComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Blaster/BlasterComponent/BuffComponent.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Blaster/BlasterComponent/LagCompensationComponent.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "Test_MultiShooter/Test_MultiShooter.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(GetMesh());
	SpringArm->TargetArmLength = 600.f;
	SpringArm->bUsePawnControlRotation = true;
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	OverHeadWidget->SetupAttachment(RootComponent);

	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	BuffComponent->SetIsReplicated(true);
	
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	// Component 不需要被注册，不需要添加到 GetLifetimeReplicatedProps 中
	CombatComponent->SetIsReplicated(true);

	LagCompensationComponent = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComponent"));
	

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);
	
	TurnInPlace = ETurnInPlace::ETIP_NotTurn;

	// 设置 Actor 每秒更新的频率
	NetUpdateFrequency = 66.f;
	// 当需要复制的属性不经常更改时，降低更新的频率
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	AttachGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttachGrenade"));
	AttachGrenade->SetupAttachment(GetMesh(), TEXT("GrenadeSocket"));
	AttachGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/**
	* Hit boxes for server-side rewind
	*/

	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("blanket"), blanket);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), backpack);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 复制条件为 COND_OwnerOnly，表示仅拥有这个 OverlappingWeapon 的 Character 所在的客户端才会被复制
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, Shield);
	DOREPLIFETIME(ABlasterCharacter, bDisableGameplay);
	
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComponent)
	{
		CombatComponent->Character = this;
	}

	if (BuffComponent)
	{
		BuffComponent->BlasterCharacter = this;
		BuffComponent->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		BuffComponent->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}

	if (LagCompensationComponent)
	{
		LagCompensationComponent->Character = this;
		if (Controller)
		{
			LagCompensationComponent->Controller = Cast<ABlasterPlayerController>(Controller);
		}
	}
	
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::SpawnDefaultWeapon()
{
	BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if (BlasterGameMode and World and !bElimed and DefaultWeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		DefaultWeapon->bDestroyWeapon = true;
		if (CombatComponent)
		{
			CombatComponent->EquippedWeapon(DefaultWeapon);
		}
	}
}

void ABlasterCharacter::ServerLeaveGame_Implementation()
{
	if (BlasterGameMode)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			BlasterGameMode->PlayerLeftGame(BlasterPlayerState);
		}
	}
}

void ABlasterCharacter::MulticastGainedTheLead_Implementation()
{
	if (CrownSystem == nullptr) return;
	if (CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetCapsuleComponent(),
			FName(),
			GetActorLocation() + FVector(0.f, 0.f, 110.f),
			GetActorRotation(),
			EAttachLocation::Type::KeepWorldPosition,
			false);
	}

	if (CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void ABlasterCharacter::MulticastLostTheLead_Implementation()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	SpawnDefaultWeapon();
	
	UpdateHUDHealth();
	UpdateHUDShield();
	UpdateHUDAmmo();
	
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}

	if (AttachGrenade)
	{
		AttachGrenade->SetVisibility(false);
	}
	
}

void ABlasterCharacter::MoveForward(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;

	if (Controller != nullptr)
	{
		const FRotator Rotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector MoveDirection(FRotationMatrix(Rotation).GetUnitAxis(EAxis::X));
		AddMovementInput(MoveDirection, Value.GetMagnitude());
	}
}

void ABlasterCharacter::MoveRight(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;

	if (Controller != nullptr)
	{
		const FRotator Rotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector MoveDirection(FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(MoveDirection, Value.GetMagnitude());
	}
}

void ABlasterCharacter::Turn(const FInputActionValue& Value)
{
	AddControllerYawInput(Value.GetMagnitude());
}

void ABlasterCharacter::LookUP(const FInputActionValue& Value)
{
	AddControllerPitchInput(Value.GetMagnitude());	
}

void ABlasterCharacter::EquippedButtonPressed()
{
	if (bDisableGameplay) return;

	if (CombatComponent)
	{
		if (CombatComponent->CombatState == ECombatState::ECS_Unoccupied) Server_EquipButtonPressed();
		bool bSwap = CombatComponent->ShouldSwapWeapons() &&
			!HasAuthority() &&
			CombatComponent->CombatState == ECombatState::ECS_Unoccupied &&
			OverlappingWeapon == nullptr;
		if (bSwap)
		{
			PlaySwapMontage();
			CombatComponent->CombatState = ECombatState::ECS_SwappingWeapon;
			bFinishedSwapping = false;
		}
	}
	
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if (bDisableGameplay) return;

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlasterCharacter::AimingButtonPressed()
{
	if (bDisableGameplay) return;

	if (CombatComponent)
	{
		CombatComponent->SetAiming(true);
	}
}

void ABlasterCharacter::AimingButtonReleased()
{
	if (bDisableGameplay) return;

	if (CombatComponent)
	{
		CombatComponent->SetAiming(false);
	}
}

void ABlasterCharacter::CalcAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;

	// 由于旋转值会被限制在 0 到 360 之间（比如 -1 会变成 359 ），所以需要映射 Pitch 轴的值
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

float ABlasterCharacter::CalcSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return  Velocity.Size();
}

void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? TObjectPtr<ABlasterPlayerController>(Cast<ABlasterPlayerController>(Controller)) : BlasterPlayerController;

	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::UpdateHUDShield()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? TObjectPtr<ABlasterPlayerController>(Cast<ABlasterPlayerController>(Controller)) : BlasterPlayerController;

	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void ABlasterCharacter::UpdateHUDAmmo()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? TObjectPtr<ABlasterPlayerController>(Cast<ABlasterPlayerController>(Controller)) : BlasterPlayerController;

	if (BlasterPlayerController and CombatComponent and CombatComponent->EquipWeapon)
	{
		BlasterPlayerController->SetHUDCarriedAmmo(CombatComponent->CarriedAmmo);
		BlasterPlayerController->SetHUDWeaponAmmo(CombatComponent->EquipWeapon->GetAmmo());
	}
}

void ABlasterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;
	
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

void ABlasterCharacter::Elim(bool bPlayerLeftGame)
{
	if (CombatComponent)
	{
		if (CombatComponent->EquipWeapon)
		{
			DropOrDestroyWeapon(CombatComponent->EquipWeapon);
		}

		if (CombatComponent->SecondaryEquipWeapon)
		{
			DropOrDestroyWeapon(CombatComponent->SecondaryEquipWeapon);
		}
	}
	
	MulticastEliminate(bPlayerLeftGame);
	
}

void ABlasterCharacter::MulticastEliminate_Implementation(bool bPlayerLeftGame)
{
	bLeftGame  = bPlayerLeftGame;
	
	bElimed = true;
    PlayElimMontage();

	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 70.f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve Value"), -0.5f);
	}
	StartDissolve();

	bDisableGameplay = true;
	GetCharacterMovement()->DisableMovement();
	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(false);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (ElimBotEffect)
	{
		FVector ElimSpawnLocation(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotEffectComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
			ElimBotEffect,
			ElimSpawnLocation,
			GetActorRotation());
	}

	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this,
			ElimBotSound,
			GetActorLocation());
	}

	bool bSniperAiming = IsLocallyControlled() and CombatComponent and CombatComponent->bAiming and CombatComponent->EquipWeapon and CombatComponent->EquipWeapon->GetWeaponType() == EWeaponTypes::EWT_Sniper;
	if (bSniperAiming)
	{
		CombatComponent->CharacterHUD->SetHidden(false);
		ShowSniperScope(false);
	}

	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
	
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ABlasterCharacter::ElimTimeFinished,
		ElimDelay);
	
}

void ABlasterCharacter::ElimTimeFinished()
{
	BlasterGameMode = Cast<ABlasterGameMode>(GetWorld()->GetAuthGameMode());
	if (BlasterGameMode and !bLeftGame)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}

	if (bLeftGame and IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve Value"), DissolveValue);
	}
	
}

void ABlasterCharacter::StartDissolve()
{
	// 时间轴的通道触发时调用回调函数
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve and DissolveTimeline)
	{
		// 给时间轴绑定曲线作为通道
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ABlasterCharacter::AimingOffset(float DeltaTime)
{
	if (CombatComponent && CombatComponent->EquipWeapon == nullptr) return;
	
	float Speed = CalcSpeed();
	
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir)
	{
		bRotateRootBone = true;
		
		// 站立待机状态
		FRotator CurrentAimRotation = FRotator(0, GetBaseAimRotation().Yaw, 0);
		FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartAimRotation);
		AO_Yaw = DeltaRotation.Yaw;
		if (TurnInPlace == ETurnInPlace::ETIP_NotTurn)
		{
			Interp_AO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		SetTurnInPlace(DeltaTime);
	}

	if (Speed > 0.f || bIsInAir)
	{
		bRotateRootBone = false;

		// Running 或者 Jumping 状态
		StartAimRotation = FRotator(0, GetBaseAimRotation().Yaw, 0);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurnInPlace = ETurnInPlace::ETIP_NotTurn;
	}

	CalcAO_Pitch();
	
}

void ABlasterCharacter::SetTurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurnInPlace = ETurnInPlace::ETIP_Left;
	}
	else if (AO_Yaw < -90.f)
	{
		TurnInPlace = ETurnInPlace::ETIP_Right;
	}

	if (TurnInPlace != ETurnInPlace::ETIP_NotTurn)
	{
		Interp_AO_Yaw = FMath::FInterpTo(Interp_AO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = Interp_AO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurnInPlace = ETurnInPlace::ETIP_NotTurn;
			StartAimRotation = FRotator(0, GetBaseAimRotation().Yaw, 0);

		}
	}
	
}

void ABlasterCharacter::FireButtonPressed()
{
	if (bDisableGameplay) return;

	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if (bDisableGameplay) return;

	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(false);
	}
}

void ABlasterCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return;

	if (CombatComponent)
	{
		CombatComponent->Reload();
	}
}

void ABlasterCharacter::ThrowGrenadeButtonPressed()
{
	if (bDisableGameplay) return;

	if (CombatComponent)
	{
		CombatComponent->ThrowGrenade();
	}
}

void ABlasterCharacter::HideCameraIfCharacterClosed()
{
	if (!IsLocallyControlled()) return;

	if (( Camera->GetComponentLocation() - GetActorLocation() ).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComponent and CombatComponent->EquipWeapon and CombatComponent->EquipWeapon->GetWeaponMesh())
		{
			CombatComponent->EquipWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComponent and CombatComponent->EquipWeapon and CombatComponent->EquipWeapon->GetWeaponMesh())
		{
			CombatComponent->EquipWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
	
}

void ABlasterCharacter::SimProxiesTurn()
{
	if (CombatComponent == nullptr || CombatComponent->EquipWeapon == nullptr) return;

	bRotateRootBone = false;

	float Speed = CalcSpeed();
	if (Speed > 0.f)
	{
		TurnInPlace = ETurnInPlace::ETIP_NotTurn;
		return;
	}
	
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurnInPlace = ETurnInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurnInPlace = ETurnInPlace::ETIP_Left;
		}
		else
		{
			TurnInPlace = ETurnInPlace::ETIP_NotTurn;
		}
		return;
	}
	TurnInPlace = ETurnInPlace::ETIP_NotTurn;
	
}

void ABlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDefeats(0);
			
			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));

			if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(BlasterPlayerState))
			{
				MulticastGainedTheLead();
			}
		}
	}
	
}

void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurnInPlace = ETurnInPlace::ETIP_NotTurn;
		return;
	}
	
	// 能用大于来比较是因为 ENetRole 是枚举值
	if (GetLocalRole() > ROLE_SimulatedProxy and IsLocallyControlled())
	{
		AimingOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalcAO_Pitch();
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (CombatComponent == nullptr || CombatComponent->EquipWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireMontage_Aim_Hip)
	{
		AnimInstance->Montage_Play(FireMontage_Aim_Hip);
		FName SectionName;
		SectionName = bAiming ? FName("Fire_Aim") : FName("Fire_Aim_Hip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquipWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName = FName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
	
}

void ABlasterCharacter::PlayReloadMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		FName SectionName;
		switch (CombatComponent->EquipWeapon->GetWeaponType())
		{
		case EWeaponTypes::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponTypes::EWT_RocketLaunch:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponTypes::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponTypes::EWT_ShotGun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponTypes::EWT_Sniper:
			SectionName = FName("SniperRifle");
			break;
		case EWeaponTypes::EWT_GrenadeLaunch:
			SectionName = FName("GrenadeLauncher");
			break;
		default:
			SectionName = FName("Rifle");
			break;
		}
		
		AnimInstance->Montage_Play(ReloadMontage);
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && GrenadeMontage)
	{
		AnimInstance->Montage_Play(GrenadeMontage);
	}
}

void ABlasterCharacter::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}
}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                      class AController* InstigatorController, AActor* DamageCauser)
{
	if (bElimed) return;
	float DamageToHealth = Damage;
	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - DamageToHealth, 0, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0, Damage);
			Shield = 0.f;
		}
	}
	
	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);
	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();

	if (Health <= 0.f)
	{
		BlasterGameMode = Cast<ABlasterGameMode>(GetWorld()->GetAuthGameMode());
		if (BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? TObjectPtr<ABlasterPlayerController>(Cast<ABlasterPlayerController>(Controller)) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);

			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
	
}

void ABlasterCharacter::Jump()
{
	if (bDisableGameplay) return;

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotEffectComponent)
	{
		ElimBotEffectComponent->DestroyComponent();
	}

	BlasterGameMode = BlasterGameMode == nullptr ? TObjectPtr<ABlasterGameMode>(GetWorld()->GetAuthGameMode<ABlasterGameMode>()) : BlasterGameMode;
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;
	if (CombatComponent && CombatComponent->EquipWeapon && bMatchNotInProgress)
	{
		CombatComponent->EquipWeapon->Destroy();
	}
	
}

void ABlasterCharacter::Server_EquipButtonPressed_Implementation()
{
	if (CombatComponent)
	{
		if (OverlappingWeapon)
		{
			CombatComponent->EquippedWeapon(OverlappingWeapon);
		}
		else if (CombatComponent->ShouldSwapWeapons())
		{
			CombatComponent->SwapWeapons();
		}
	}
}

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	
	HideCameraIfCharacterClosed();

	PollInit();
	
}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	EnhancedInputComponent->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &ABlasterCharacter::MoveForward);
	EnhancedInputComponent->BindAction(IA_MoveRight, ETriggerEvent::Triggered, this, &ABlasterCharacter::MoveRight);
	EnhancedInputComponent->BindAction(IA_Turn, ETriggerEvent::Triggered, this, &ABlasterCharacter::Turn);
	EnhancedInputComponent->BindAction(IA_LookUP, ETriggerEvent::Triggered, this, &ABlasterCharacter::LookUP);
	EnhancedInputComponent->BindAction(IA_Equipped, ETriggerEvent::Started, this, &ABlasterCharacter::EquippedButtonPressed);
	EnhancedInputComponent->BindAction(IA_Crouch, ETriggerEvent::Started, this, &ABlasterCharacter::CrouchButtonPressed);
	EnhancedInputComponent->BindAction(IA_Aiming, ETriggerEvent::Started, this, &ABlasterCharacter::AimingButtonPressed);
	EnhancedInputComponent->BindAction(IA_Aiming, ETriggerEvent::Completed, this, &ABlasterCharacter::AimingButtonReleased);
	EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &ABlasterCharacter::Jump);
	EnhancedInputComponent->BindAction(IA_Fire, ETriggerEvent::Started, this, &ABlasterCharacter::FireButtonPressed);
	EnhancedInputComponent->BindAction(IA_Fire, ETriggerEvent::Completed, this, &ABlasterCharacter::FireButtonReleased);
	EnhancedInputComponent->BindAction(IA_Reload, ETriggerEvent::Completed, this, &ABlasterCharacter::ReloadButtonPressed);
	EnhancedInputComponent->BindAction(IA_ThrowGrenade, ETriggerEvent::Completed, this, &ABlasterCharacter::ThrowGrenadeButtonPressed);
	
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(true);
	}

	if (LastWeapon)
	{
		LastWeapon->ShowPickUpWidget(false);
	}
	
}



void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(false);
	}
	
	OverlappingWeapon = Weapon;

	// 本地控制器控制的 Pawn 才显示 UI
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickUpWidget(true);
		}
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquipWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (CombatComponent && CombatComponent->bAiming);
}

bool ABlasterCharacter::IsLocallyReloading()
{
	if (CombatComponent == nullptr) return false;

	return CombatComponent->bLocallyReloading;
}

ECombatState ABlasterCharacter::GetCombatState() const
{
	if (CombatComponent == nullptr) return ECombatState::ECS_Max;
	
	return CombatComponent->CombatState;
}

TObjectPtr<AWeapon> ABlasterCharacter::GetWeapon()
{
	if (CombatComponent == nullptr) return nullptr;

	return CombatComponent->EquipWeapon;
}

FVector ABlasterCharacter::GetHitTarget()
{
	if (!CombatComponent) return FVector::ZeroVector;

	return CombatComponent->HitTarget;
}
