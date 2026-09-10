// Fill out your copyright notice in the Description page of Project Settings.


#include "MyWeaponBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "MyCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProjectileBase.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AMyWeaponBase::AMyWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	bReplicates = true;
}

// Called when the game starts or when spawned
void AMyWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!HasAuthority()) return;

	CoolDown = FMath::Max(CoolDown - DeltaTime, 0);

	if (bCanFire)
	{
		if (CoolDown <= 0.f)
		{
			Fire();
			MulticastMakeMuzzleFlash();
			CoolDown = FiringRate;
			//UE_LOG(LogTemp, Warning, TEXT("Fire"));
		}
	}

}


void AMyWeaponBase::StartFire()
{
	//UE_LOG(LogTemp, Warning, TEXT("Start Fire"));

	bCanFire = true;
	FiringRate = FireRate;
}


void AMyWeaponBase::StopFire()
{
	//UE_LOG(LogTemp, Warning, TEXT("Stop Fire"));

	bCanFire = false;
}

void AMyWeaponBase::Fire()
{
	FHitResult OutResult;
	
	if (LineTrace(OutResult))
	{

	}
	else
	{

	}

}

void AMyWeaponBase::MakeMuzzleFlash()
{
	UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(),
		MuzzleFlash,
		Mesh->GetSocketLocation(TEXT("Muzzle"))
	);
}

void AMyWeaponBase::MulticastMakeMuzzleFlash_Implementation()
{
	MakeMuzzleFlash();
}

bool AMyWeaponBase::LineTrace(FHitResult& OutResult)
{
	bool bResult = false;

	AMyCharacter* Pawn = Cast<AMyCharacter>(GetOwner());
	if (!Pawn)
	{
		return bResult;
	}

	// Dedicated servers have no viewport. Aim from the controller rotation and
	// perform the authoritative trace and projectile spawn here.
	if (HasAuthority())
	{
		const float Range = 99999.f;
		const FVector CameraStart = Pawn->Camera ? Pawn->Camera->GetComponentLocation() : Mesh->GetSocketLocation(TEXT("Muzzle"));
		const FVector CameraEnd = CameraStart + (Pawn->GetControlRotation().Vector() * Range);
		TArray<AActor*> IgnoreActors;
		IgnoreActors.Add(Pawn);
		IgnoreActors.Add(this);

		bResult = UKismetSystemLibrary::LineTraceSingle(GetWorld(), CameraStart, CameraEnd,
			UEngineTypes::ConvertToTraceType(ECC_Visibility), true, IgnoreActors, EDrawDebugTrace::None,
			OutResult, true, FLinearColor::Red, FLinearColor::Green, 3.0f);

		const FVector MuzzleStart = Mesh->GetSocketLocation(TEXT("Muzzle"));
		const FVector Target = bResult ? OutResult.ImpactPoint : CameraEnd;
		const FRotator SpawnRotation = UKismetMathLibrary::FindLookAtRotation(MuzzleStart, Target + (UKismetMathLibrary::RandomUnitVector() * 0.3f));
		if (ProjectileTemplate)
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Owner = Pawn;
			SpawnParameters.Instigator = Pawn;
			GetWorld()->SpawnActor<AProjectileBase>(ProjectileTemplate, MuzzleStart, SpawnRotation, SpawnParameters);
		}
		return bResult;
	}

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC)
	{
		return bResult;
	}

	int ScreenSizeX = 0;
	int ScreenSizeY = 0;
	int CenterX = 0;
	int CenterY = 0;
	float Range = 99999.f;

	FVector WorldLocation;
	FVector WorldDirection;

	PC->GetViewportSize(ScreenSizeX, ScreenSizeY);

	CenterX = ScreenSizeX / 2;
	CenterY = ScreenSizeY / 2;

	//2D -> 3D
	PC->DeprojectScreenPositionToWorld(CenterX,
		CenterY,
		WorldLocation,
		WorldDirection
	);


	//FVector Start = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation();
	//WorldDirection = UKismetMathLibrary::GetForwardVector(UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraRotation());
	FVector Start = Pawn->Camera->GetComponentLocation();
	FVector End = Start + (WorldDirection * Range);

	TArray<AActor*> IgnoreActors;



	//Camera LineTrace
	bResult = UKismetSystemLibrary::LineTraceSingle(GetWorld(),
		Start,
		End,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		true,
		IgnoreActors,
		EDrawDebugTrace::None,
		OutResult,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		3.0f
	);


	Start = Mesh->GetSocketLocation(TEXT("Muzzle"));
	FVector ForwardVector = OutResult.ImpactPoint - Start;
	ForwardVector.Normalize();
	End = Start + (ForwardVector * Range);

	End = OutResult.bBlockingHit ? End : OutResult.TraceEnd;

	FRotator SpawnRotator = UKismetMathLibrary::FindLookAtRotation(
		Start, End + (UKismetMathLibrary::RandomUnitVector() * 0.3f)
	);

	GetWorld()->SpawnActor<AProjectileBase>(ProjectileTemplate,
		Start,
		SpawnRotator
	);





	return bResult;
}

void AMyWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMyWeaponBase, CurrentBulletCount);
}
