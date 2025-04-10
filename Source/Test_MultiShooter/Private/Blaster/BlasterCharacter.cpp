// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/BlasterCharacter.h"

#include "EnhancedInputComponent.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"

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

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	// Component 不需要被注册，不需要添加到 GetLifetimeReplicatedProps 中
	CombatComponent->SetIsReplicated(true);
	
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 复制条件为 COND_OwnerOnly，表示仅拥有这个 OverlappingWeapon 的 Character 所在的客户端才会被复制
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComponent)
	{
		CombatComponent->Character = this;
	}
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABlasterCharacter::MoveForward(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		const FRotator Rotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector MoveDirection(FRotationMatrix(Rotation).GetUnitAxis(EAxis::X));
		AddMovementInput(MoveDirection, Value.GetMagnitude());
	}
}

void ABlasterCharacter::MoveRight(const FInputActionValue& Value)
{
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
	if (CombatComponent)
	{
		if (HasAuthority())
		{
			CombatComponent->EquippedWeapon(OverlappingWeapon);
		}
		else
		{
			// 客户端调用 RPC 函数装备武器
			Server_EquipButtonPressed();
		}
	}
	
}

void ABlasterCharacter::CrouchButtonPressed()
{
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
	if (CombatComponent)
	{
		CombatComponent->SetAiming(true);
	}
}

void ABlasterCharacter::AimingButtonReleased()
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(false);
	}
}

void ABlasterCharacter::Server_EquipButtonPressed_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->EquippedWeapon(OverlappingWeapon);
	}
}


// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
