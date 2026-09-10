// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DecalComponent.h"


// Sets default values
AProjectileBase::AProjectileBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	RootComponent = Sphere;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->MaxSpeed = 10000.0f;
	Movement->InitialSpeed = 10000.0f;
	// Only the server spawns and simulates projectiles; all clients receive the replicated actor.
	bReplicates = true;
	SetReplicateMovement(true);
	InitialLifeSpan = 5.0f;
}

// Called when the game starts or when spawned
void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	Sphere->OnComponentHit.AddDynamic(this, &AProjectileBase::ProcessHit);
	
}

// Called every frame
void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectileBase::ProcessHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority() || !OtherActor || OtherActor == GetOwner()) return;

	UE_LOG(LogTemp, Warning, TEXT("Hit %s"), *OtherActor->GetName());
	MulticastSpawnImpact(Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	UGameplayStatics::ApplyPointDamage(Hit.GetActor(), Damage, -Hit.ImpactNormal, Hit, GetInstigatorController(), this, DamageType);
	Destroy();
}

void AProjectileBase::MulticastSpawnImpact_Implementation(const FVector& Location, const FRotator& Rotation)
{
	if (!Decal) return;
	if (UDecalComponent* MadeDecal = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), Decal, FVector(5, 5, 5), Location, Rotation, 5.0f))
	{
		MadeDecal->SetFadeScreenSize(0.005f);
	}
}