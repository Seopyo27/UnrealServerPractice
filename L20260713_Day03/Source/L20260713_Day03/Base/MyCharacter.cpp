// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/ChildActorComponent.h"
#include "MyWeaponBase.h"
#include "GameFramework/DamageType.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AMyCharacter::AMyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom);

	GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()),
		FRotator(0, -90, 0));

	Weapon = CreateDefaultSubobject<UChildActorComponent>(TEXT("Weapon"));
	Weapon->SetupAttachment(GetMesh(), FName(TEXT("HandGrip_R")));

	bArmed = true;
	bReplicates = true;
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* UIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (UIC)
	{
		UIC->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &AMyCharacter::Jump);
		UIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AMyCharacter::Look);
		UIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AMyCharacter::Move);

		UIC->BindAction(IA_Zoom, ETriggerEvent::Triggered, this, &AMyCharacter::StartZoom);
		UIC->BindAction(IA_Zoom, ETriggerEvent::Canceled, this, &AMyCharacter::StopZoom);
		UIC->BindAction(IA_Zoom, ETriggerEvent::Completed, this, &AMyCharacter::StopZoom);

		UIC->BindAction(IA_Lean, ETriggerEvent::Triggered, this, &AMyCharacter::Lean);
		UIC->BindAction(IA_Lean, ETriggerEvent::Canceled, this, &AMyCharacter::Lean);
		UIC->BindAction(IA_Lean, ETriggerEvent::Completed, this, &AMyCharacter::Lean);

		UIC->BindAction(IA_Fire, ETriggerEvent::Started, this, &AMyCharacter::StartFire);
		UIC->BindAction(IA_Fire, ETriggerEvent::Canceled, this, &AMyCharacter::StopFire);
		UIC->BindAction(IA_Fire, ETriggerEvent::Completed, this, &AMyCharacter::StopFire);

		UIC->BindAction(IA_Reload, ETriggerEvent::Triggered, this, &AMyCharacter::Reload);
	}
}

void AMyCharacter::StartFire()
{
	if (bArmed) ServerSetFiring(true);
}

void AMyCharacter::StopFire()
{
	ServerSetFiring(false);
}

void AMyCharacter::ServerSetFiring_Implementation(bool bNewFiring)
{
	if (!bArmed || bIsDead) return;
	bFire = bNewFiring;
	if (AMyWeaponBase* ChildWeapon = Cast<AMyWeaponBase>(Weapon->GetChildActor()))
	{
		if (bNewFiring) ChildWeapon->StartFire();
		else ChildWeapon->StopFire();
	}
}

void AMyCharacter::Look(const FInputActionValue& Value)
{
	FVector2D Direction = Value.Get<FVector2D>();

	AddControllerPitchInput(Direction.Y);
	AddControllerYawInput(Direction.X);
//	AddControllerRollInput(Direction.Y);
}

void AMyCharacter::Move(const FInputActionValue& Value)
{
	FVector2D Direction = Value.Get<FVector2D>();

	FRotator CameraRotaion = GetControlRotation();

	FVector ForawrdVector =  UKismetMathLibrary::GetForwardVector( FRotator(0, CameraRotaion.Yaw, 0));
	FVector RightVector =  UKismetMathLibrary::GetRightVector(FRotator(0, CameraRotaion.Yaw, CameraRotaion.Roll));

	
	AddMovementInput(ForawrdVector * Direction.X);
	AddMovementInput(RightVector * Direction.Y);
}

void AMyCharacter::StartZoom()
{
	ServerSetZoom(true);
	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
}

void AMyCharacter::StopZoom()
{
	ServerSetZoom(false);
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
}

void AMyCharacter::ServerSetZoom_Implementation(bool bNewZoom)
{
	bZoom = bNewZoom;
	GetCharacterMovement()->MaxWalkSpeed = bNewZoom ? 300.0f : 600.0f;
}

void AMyCharacter::Lean(const FInputActionValue& Value)
{
	LeanValue = Value.Get<float>();
}

float AMyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority() || bIsDead) return 0.0f;
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);


	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		FPointDamageEvent* Event = (FPointDamageEvent*)(&DamageEvent);
		if (Event)
		{
			SpawnHitEffect(Event->HitInfo);

			if (CurrentHP <= 0)
			{
				return 0;
			}

			if (Event->HitInfo.BoneName == TEXT("head"))
			{
				CurrentHP = 0;
				DamageAmount = 100.f;
			}
			else
			{
				int32 MontageType = FMath::RandRange(1, 8);
				FString SectionName = FString::Printf(TEXT("%d"), MontageType);
				MulticastPlayHitReaction(FName(*SectionName));
				CurrentHP -= DamageAmount;
			}
		

			CurrentHP = FMath::Clamp(CurrentHP, 0, MaxHP);

			if (CurrentHP == 0) ApplyDeathState();


		}
	}
	if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{

	}



	return DamageAmount;
}

void AMyCharacter::SpawnHitEffect(const FHitResult& InResult)
{
	UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(),
		HitEffect,
		InResult.ImpactPoint
	);
}

void AMyCharacter::Reload()
{
	ServerReload();
}

void AMyCharacter::ServerReload_Implementation()
{
	if (!bIsDead) MulticastPlayReload();
}

void AMyCharacter::MulticastPlayReload_Implementation()
{
	PlayAnimMontage(ReloadAnimMontage);
}

void AMyCharacter::MulticastPlayHitReaction_Implementation(const FName& SectionName)
{
	PlayAnimMontage(HitReactionAnimMontage, 1.0f, SectionName);
}

void AMyCharacter::OnRep_CurrentHP()
{
	if (CurrentHP <= 0.0f) ApplyDeathState();
}

void AMyCharacter::OnRep_Dead()
{
	if (bIsDead) ApplyDeathState();
}

void AMyCharacter::ApplyDeathState()
{
	if (bIsDead && GetMesh()->IsSimulatingPhysics()) return;
	bIsDead = true;
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
}

void AMyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMyCharacter, bArmed);
	DOREPLIFETIME(AMyCharacter, bZoom);
	DOREPLIFETIME(AMyCharacter, CurrentHP);
	DOREPLIFETIME(AMyCharacter, bIsDead);
}


