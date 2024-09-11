// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/KismetMathLibrary.h"	
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "FPSPlayer.generated.h"

UCLASS()
class FPSMULTIPLAYER_API AFPSPlayer : public ACharacter
{
	GENERATED_BODY()
	
public:
	// Sets default values for this character's properties
	AFPSPlayer();
	//Components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPComponent")
	UCameraComponent* FPSPlayerCamera;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPComponent")
	USkeletalMeshComponent* FPSWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPComponent")
	USkeletalMeshComponent* TPSPlayer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPComponent")
	USkeletalMeshComponent* TPSWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPComponent")
	UArrowComponent * WeaponArrow;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPComponent")
	APlayerController * PlayerController;
	
	//Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPVariables")
	float HP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPVariables")
	bool bReloading;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPVariables")
	bool bRunning;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPVariables")
	float Damage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPVariables")
	int Ammo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPVariables")
	bool bDead;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPVariables")
	bool bAiming;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPVariables")
	bool bFiring;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPVariables")
	TSubclassOf<ACharacter> CharacterToSpawn;

	//AnimMontages
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPVariables")
	UAnimInstance * ArmAnimInstance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPVariables")
	UAnimInstance* TPSAnimInstance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPVariables")
	UAnimInstance* TPSWeaponAnimInstance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPVariables")
	UAnimInstance * FPSWeaponAnimInstance; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPAnimMontages")
	UAnimMontage* FireMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPAnimMontages")
	UAnimMontage* FireMontage_Aiming;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPAnimMontages")
	UAnimMontage* ReloadMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPAnimMontages")
	UAnimMontage* FPSWeaponReloadMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPAnimMontages")
	UAnimMontage* FPSWeaponFireMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CPPAnimMontages")
	UAnimMontage* TPSReloadMontage;

	UFUNCTION(BlueprintImplementableEvent)
	void FOVZoomIn();
	UFUNCTION(BlueprintImplementableEvent)
	void FOVZoomOut();
	UFUNCTION(BlueprintImplementableEvent)
	void CameraEffect();
	UFUNCTION(BlueprintImplementableEvent)
	void DamageCameraEffect();
	//Network
	UFUNCTION(Server,Reliable)
	void SetMaxWalkSpeed_Server(float Val);
	UFUNCTION(NetMulticast,Reliable)
	void SetMaxWalkSpeed_NetMulticast(float Val);
	UFUNCTION(Server,Reliable)
	void TPSMontagePlay_Server(UAnimMontage * AnimMontage, float PlaySpeed = 1.f);
	UFUNCTION(NetMulticast,Reliable)
	void TPSMontagePlay_NetMulticast(UAnimMontage* AnimMontage, float PlaySpeed = 1.f);
	UFUNCTION(Server,Reliable)
	void TPSWeaponMontagePlay_Server(UAnimMontage * AnimMontage, float PlaySpeed = 1.f);
	UFUNCTION(NetMulticast,Reliable)
	void TPSWeaponMontagePlay_NetMulticast(UAnimMontage* AnimMontage, float PlaySpeed = 1.f);
	UFUNCTION(Server,Reliable)
	void EnableDeath_Server();
	UFUNCTION(NetMulticast,Reliable)
	void EnableDeath_NetMulticast();
	UFUNCTION(Server,Reliable)
	void TraceDetection_Server(AFPSPlayer* Target);
	UFUNCTION(Server,Reliable)
	void Respawn_Server();
	void DamageCalculation(float Val);
	void Aiming();
	void StopAiming();
	void Running();
	void StopRunning();
	void FireExecute();
	void FireLoopCheck();
	void Fire();
	void Fire_Released();
	void Reload();
	void ReloadFinished();
	void Dead();
	void MoveForward(float Vaule);
	void MoveRight(float Vaule);
	void MouseX(float Vaule);
	void MouseY(float Vaule);
	void DestryCorpse();
	FTransform ReturnSpawnTransform();
	AFPSPlayer* PlayerPawn;

	FTimerHandle RemoveReloadHandle;
	FTimerHandle FiringHandle;
	FTimerHandle DestroyHandle;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
