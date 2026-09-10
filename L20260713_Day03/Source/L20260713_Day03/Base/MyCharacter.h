// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "InputAction.h"

#include "MyCharacter.generated.h"




class USpringArmComponent;
class UCameraComponent;
class UChildActorComponent;

UCLASS()
class L20260713_DAY03_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UChildActorComponent> Weapon;




	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> IA_Jump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> IA_Zoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> IA_Lean;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> IA_Fire;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> IA_Reload;

	UFUNCTION(BlueprintCallable)
	void StartFire();

	UFUNCTION(BlueprintCallable)
	void StopFire();

	UFUNCTION(Server, Reliable)
	void ServerSetFiring(bool bNewFiring);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	uint32 bFire : 1 = false;


	void Look(const FInputActionValue& Value);
	
	void Move(const FInputActionValue& Value);

	void StartZoom();

	void StopZoom();

	UFUNCTION(Server, Reliable)
	void ServerSetZoom(bool bNewZoom);

	void Reload();

	UFUNCTION(Server, Reliable)
	void ServerReload();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayReload();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayHitReaction(const FName& SectionName);


	void Lean(const FInputActionValue& Value);

	//void StopLean(const FInputActionValue& Value);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Stat")
	uint32 bArmed : 1 = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Stat")
	uint32 bZoom : 1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float LeanValue = 0;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_CurrentHP, Category = "Stat")
	float CurrentHP = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MaxHP = 100.0f;

	UPROPERTY(ReplicatedUsing=OnRep_Dead, BlueprintReadOnly, Category = "Stat")
	bool bIsDead = false;

	UFUNCTION()
	void OnRep_CurrentHP();

	UFUNCTION()
	void OnRep_Dead();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TObjectPtr<UParticleSystem> HitEffect;

	UFUNCTION()
	void SpawnHitEffect(const FHitResult& InResult);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TObjectPtr<UAnimMontage> HitReactionAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TObjectPtr<UAnimMontage> ReloadAnimMontage; 

private:
	void ApplyDeathState();
};


