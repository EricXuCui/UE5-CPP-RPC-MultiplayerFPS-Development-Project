// Fill out your copyright no+t
// ice in the Description page of Project Settings.


#include "FPSPlayer.h"

// Sets default values
AFPSPlayer::AFPSPlayer()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	FPSPlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	FPSWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PlayerWeapon"));
	TPSWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TPSPlayerWeapon"));
	TPSPlayer = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TPSPlayer"));
	WeaponArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("FPSWeaponArrow"));
	WeaponArrow->SetupAttachment(FPSWeapon);
	TPSPlayer->SetupAttachment(GetRootComponent());
	TPSWeapon->SetupAttachment(TPSPlayer, FName("R_GunSocket"));
	FPSPlayerCamera->SetupAttachment(GetRootComponent());
	FPSPlayerCamera->bUsePawnControlRotation = true;
	GetMesh()->CastShadow = false;
	FPSWeapon->CastShadow = false;
	GetMesh()->SetupAttachment(FPSPlayerCamera);	
	FPSWeapon->SetupAttachment(GetMesh(),FName("R_GunSocket"));
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	GetMesh()->bOnlyOwnerSee = true;
	FPSWeapon->bOnlyOwnerSee = true;
	TPSWeapon->bOwnerNoSee = true;
	TPSPlayer->bOwnerNoSee = true;
	GetCharacterMovement()->SetIsReplicated(true);
	TPSPlayer->SetIsReplicated(true);
	TPSWeapon->SetIsReplicated(true);
	HP = 100.f;
	Damage = 20.f;
	Ammo = 30;
	bReplicates = true;
}

void AFPSPlayer::SetMaxWalkSpeed_Server_Implementation(float Val)
{
	SetMaxWalkSpeed_NetMulticast(Val);
}

void AFPSPlayer::SetMaxWalkSpeed_NetMulticast_Implementation(float Val)
{
	GetCharacterMovement()->MaxWalkSpeed = Val;
}

void AFPSPlayer::TPSMontagePlay_Server_Implementation(UAnimMontage* AnimMontage, float PlaySpeed /*= 1.f*/)
{
	TPSMontagePlay_NetMulticast(AnimMontage, PlaySpeed);
}

void AFPSPlayer::TPSMontagePlay_NetMulticast_Implementation(UAnimMontage* AnimMontage, float PlaySpeed /*= 1.f*/)
{
	TPSAnimInstance->Montage_Play(AnimMontage, PlaySpeed);
}

void AFPSPlayer::TPSWeaponMontagePlay_Server_Implementation(UAnimMontage* AnimMontage, float PlaySpeed /*= 1.f*/)
{
	TPSWeaponMontagePlay_NetMulticast(AnimMontage, PlaySpeed);
}

void AFPSPlayer::TPSWeaponMontagePlay_NetMulticast_Implementation(UAnimMontage* AnimMontage, float PlaySpeed /*= 1.f*/)
{
	TPSWeaponAnimInstance->Montage_Play(AnimMontage, PlaySpeed);
}

void AFPSPlayer::EnableDeath_Server_Implementation()
{
	EnableDeath_NetMulticast();
}

void AFPSPlayer::EnableDeath_NetMulticast_Implementation()
{
	TPSPlayer->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	TPSPlayer->SetSimulatePhysics(true);
	TPSPlayer->bOnlyOwnerSee = false;
	GetMesh()->DestroyComponent();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
}

void AFPSPlayer::TraceDetection_Server_Implementation(AFPSPlayer* Target)
{
	//TraceDetection_NetMulticast();
	Target->DamageCalculation(Damage);		
}

void AFPSPlayer::Aiming()
{
	if (!bReloading && !bDead)
	{
		if (bRunning)
		{
			StopRunning();
		}
		if (ArmAnimInstance)
		{
			ArmAnimInstance->StopAllMontages(0.2f);
		}
		FOVZoomIn();
		bAiming = true;
	}
}

void AFPSPlayer::StopAiming()
{
	if (!bReloading && !bDead)
	{
		if (ArmAnimInstance)
		{
			ArmAnimInstance->StopAllMontages(0.2f);
		}
		FOVZoomOut();
		bAiming = false;
	}
}

void AFPSPlayer::Running()
{
	if ( GetCharacterMovement()->Velocity.Size() > 100.f && !bDead)
	{
		bRunning = true;
		if (bAiming)
		{
			StopAiming();
		}
		if (ArmAnimInstance)
		{
			if (!bReloading)
			{
				ArmAnimInstance->StopAllMontages(0.2f);
			}
		}
		bFiring = false;
		SetMaxWalkSpeed_Server(500.f);
	}
}
 
void AFPSPlayer::StopRunning()
{
	bRunning = false;
	SetMaxWalkSpeed_Server(300.f);
}

void AFPSPlayer::FireExecute()
{
	if (Ammo > 0 && !bReloading && !bDead)
	{
		if (bRunning)
		{
			StopRunning();
		}
		if (bAiming)
		{
			if (ArmAnimInstance)
			{
				ArmAnimInstance->Montage_Play(FireMontage_Aiming);
			}

		}
		else
		{
			if (ArmAnimInstance)
			{
				ArmAnimInstance->Montage_Play(FireMontage);
			}
		}
		if (FPSWeaponAnimInstance)
		{
			FPSWeaponAnimInstance->Montage_Play(FPSWeaponFireMontage);
		}
		TPSWeaponMontagePlay_Server(FPSWeaponFireMontage);
		bFiring = true;
		CameraEffect();
		FVector StartLocation = WeaponArrow->GetComponentLocation();
		FVector EndLocation = WeaponArrow->GetComponentLocation() + WeaponArrow->GetForwardVector() * 50000;
		FHitResult HitResult;
		//DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Black, false, 1);
		bool Hit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Pawn);
		if (Hit)
		{
			if (HitResult.GetActor() != GetOwner(), Cast<AFPSPlayer>(HitResult.GetActor()))
			{

				AFPSPlayer* Target = Cast<AFPSPlayer>(HitResult.GetActor());
				TraceDetection_Server(Target);
				
			}
		}
		AddControllerPitchInput(-0.10f);
		Ammo--;
	}
	else
	{
		Reload();
	}
}

void AFPSPlayer::FireLoopCheck()
{
	if (bFiring)
	{
		FireExecute();
	}
}

void AFPSPlayer::Fire()
{
	bFiring = true;
}

void AFPSPlayer::Fire_Released()
{
	bFiring = false;
}

void AFPSPlayer::Reload()
{
	if (!bReloading && !bDead)
	{
		if (bAiming)
		{
			StopAiming();
		}	
		FPSWeaponAnimInstance->Montage_Play(FPSWeaponReloadMontage);
		ArmAnimInstance->Montage_Play(ReloadMontage);
		TPSMontagePlay_Server(TPSReloadMontage);
		bReloading = true;
		GetWorldTimerManager().SetTimer(RemoveReloadHandle, this, &AFPSPlayer::ReloadFinished, 2.6f, false);
	}
}


void AFPSPlayer::ReloadFinished()
{
	bReloading = false;
	Ammo = 30;
}




void AFPSPlayer::Respawn_Server_Implementation()
{
	//Respawn_NetMulticast();

		APlayerController* Temp = Cast<APlayerController>(GetOwner());
		if (Temp)
		{
			Temp->UnPossess();
			PlayerPawn = GetWorld()->SpawnActor<AFPSPlayer>(CharacterToSpawn, ReturnSpawnTransform());
			GEngine->AddOnScreenDebugMessage(-1, 2.F, FColor::Black, TEXT("Respawned"));
			Temp->Possess(Cast<ACharacter>(PlayerPawn));
		}
}

void AFPSPlayer::DamageCalculation(float Val)
{
	HP -= Val;
	if (IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, FString::SanitizeFloat(HP));
	}
	DamageCameraEffect();
	if (HP <= 0)
	{
		EnableDeath_Server();
		bDead = true;
		GetWorldTimerManager().SetTimer(DestroyHandle, this, &AFPSPlayer::DestryCorpse, 2.6f, false);
		GetWorldTimerManager().PauseTimer(FiringHandle);
		FPSWeapon->SetHiddenInGame(true);
	}
}


void AFPSPlayer::MoveForward(float Vaule)
{
	const FVector LocalDirection = FRotationMatrix(FRotator(0,GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::X);
	AddMovementInput(LocalDirection, Vaule);
}

void AFPSPlayer::MoveRight(float Vaule)
{
	const FVector LocalDirection = FRotationMatrix(FRotator(0, GetControlRotation().Yaw, 0)).GetUnitAxis(EAxis::Y);
	AddMovementInput(LocalDirection, Vaule);
}

void AFPSPlayer::MouseX(float Vaule)
{
	AddControllerYawInput(Vaule);
}

void AFPSPlayer::MouseY(float Vaule)
{
	AddControllerPitchInput(Vaule);
}

void AFPSPlayer::DestryCorpse()
{
	Respawn_Server();
	this->Destroy();
}

FTransform AFPSPlayer::ReturnSpawnTransform()
{
	TSubclassOf<APlayerStart> Start;
	Start = APlayerStart::StaticClass();
	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), Start, SpawnPoints);
	int RandomSpawnPoints = UKismetMathLibrary::RandomIntegerInRange(0, SpawnPoints.Num() - 1);
	return  SpawnPoints[RandomSpawnPoints]->GetTransform();
}

// Called when the game starts or when spawned
void AFPSPlayer::BeginPlay()
{
	Super::BeginPlay();
	ArmAnimInstance = GetMesh()->GetAnimInstance();
	FPSWeaponAnimInstance = FPSWeapon->GetAnimInstance();	
	TPSAnimInstance = TPSPlayer->GetAnimInstance();
	TPSWeaponAnimInstance = TPSWeapon->GetAnimInstance();
	GetWorldTimerManager().SetTimer(FiringHandle, this, &AFPSPlayer::FireLoopCheck, 0.15f, true, 0.f);
	PlayerController = Cast<APlayerController>(GetController());
}

// Called every frame
void AFPSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AFPSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Aiming", EInputEvent::IE_Pressed, this, &AFPSPlayer::Aiming);	
	PlayerInputComponent->BindAction("Aiming", EInputEvent::IE_Released, this, &AFPSPlayer::StopAiming);
	PlayerInputComponent->BindAction("Running", EInputEvent::IE_Pressed, this, &AFPSPlayer::Running);
	PlayerInputComponent->BindAction("Running", EInputEvent::IE_Released, this, &AFPSPlayer::StopRunning);
	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &AFPSPlayer::Fire);
	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Released, this, &AFPSPlayer::Fire_Released);
	PlayerInputComponent->BindAction("Reload", EInputEvent::IE_Pressed, this, &AFPSPlayer::Reload);
	PlayerInputComponent->BindAxis("MouseX", this, &AFPSPlayer::MouseX);
	PlayerInputComponent->BindAxis("MouseY", this, &AFPSPlayer::MouseY);
	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSPlayer::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSPlayer::MoveRight);
}
