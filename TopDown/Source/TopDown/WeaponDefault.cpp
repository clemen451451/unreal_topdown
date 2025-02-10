// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponDefault.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AWeaponDefault::AWeaponDefault()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	RootComponent = SceneComponent;

	SkeletalMeshWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	SkeletalMeshWeapon->SetGenerateOverlapEvents(false);
	SkeletalMeshWeapon->SetCollisionProfileName(TEXT("NoCollision"));
	SkeletalMeshWeapon->SetupAttachment(RootComponent);

	StaticMeshWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh "));
	StaticMeshWeapon->SetGenerateOverlapEvents(false);
	StaticMeshWeapon->SetCollisionProfileName(TEXT("NoCollision"));
	StaticMeshWeapon->SetupAttachment(RootComponent);

	ShootLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("ShootLocation"));
	ShootLocation->SetupAttachment(RootComponent);

	ShellBulletLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("ShellBulletLocation"));
	ShellBulletLocation->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AWeaponDefault::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AWeaponDefault::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FireTick(DeltaTime);
	ReloadTick(DeltaTime);
	DispersionTick(DeltaTime);
}

void AWeaponDefault::FireTick(float DeltaTime)
{
	if (GetWeaponRound() > 0)
	{
		if (WeaponFiring)
		{
			if (FireTimer < 0.f)
			{
				if (!WeaponReloading)
				{
					Fire();

					EffectShotTimer = 0.3f;

					if(WeaponFireEffectComponent)
						WeaponFireEffectComponent->SetVisibility(true);
				}
			}
			else
			{
				EffectShotTimer -= DeltaTime;

				if (EffectShotTimer <= 0.0f)
				{
					if(WeaponFireEffectComponent)
						WeaponFireEffectComponent->SetVisibility(false);
				}

				FireTimer -= DeltaTime;
			}
		}
		else
		{
			if(FireTimer > 0.0f)
				FireTimer -= DeltaTime;
		}
	}
	else
	{
		if (!WeaponReloading)
		{
			InitReload();
		}
	}
}

void AWeaponDefault::ReloadTick(float DeltaTime)
{
	if (WeaponReloading)
	{
		if (ReloadTimer < 0.0f)
		{
			FinishReload();
		}
		else
		{
			ReloadTimer -= DeltaTime;
		}
	}
}

void AWeaponDefault::DispersionTick(float DeltaTime)
{
	if (!WeaponReloading)
	{
		if (!WeaponFiring)
		{
			if (ShouldReduceDispersion)
				CurrentDispersion = CurrentDispersion - CurrentDispersionReduction;
			else
				CurrentDispersion = CurrentDispersion + CurrentDispersionReduction;
		}

		if (CurrentDispersion < CurrentDispersionMin)
		{

			CurrentDispersion = CurrentDispersionMin;

		}
		else
		{
			if (CurrentDispersion > CurrentDispersionMax)
			{
				CurrentDispersion = CurrentDispersionMax;
			}
		}
	}
	if (ShowDebug)
		UE_LOG(LogTemp, Warning, TEXT("Dispersion: MAX = %f. MIN = %f. Current = %f"), CurrentDispersionMax, CurrentDispersionMin, CurrentDispersion);
}

void AWeaponDefault::WeaponInit()
{
	/*if (SkeletalMeshWeapon && !SkeletalMeshWeapon->SkeletalMesh)
	{
		SkeletalMeshWeapon->DestroyComponent(true);
	}

	if (StaticMeshWeapon && !StaticMeshWeapon->GetStaticMesh())
	{
		StaticMeshWeapon->DestroyComponent();
	}*/

	if (WeaponSetting.EffectFireWeapon)
	{
		WeaponFireEffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(WeaponSetting.EffectFireWeapon, ShootLocation, NAME_None, FVector(0.f, 0.f, 0.f), FRotator(0.f), EAttachLocation::Type::KeepRelativeOffset, false, true);
		
		SetWeaponStateFire(false);
	}

}

void AWeaponDefault::SetWeaponStateFire(bool bIsFire)
{
	if (CheckWeaponCanFire())
	{
		if (bIsFire)
		{
			if (this->WeaponReloading)
				bIsFire = false;
		}

		WeaponFiring = bIsFire;
	}
	else
	{
		WeaponFiring = false;
	}

	if (WeaponFireEffectComponent)
	{
		WeaponFireEffectComponent->SetVisibility(bIsFire);
	}
}

bool AWeaponDefault::CheckWeaponCanFire()
{
	return !BlockFire;
}

FProjectileInfo AWeaponDefault::GetProjectile()
{
	return WeaponSetting.ProjectileSetting;
}

void AWeaponDefault::Fire()
{
	FireTimer = WeaponSetting.RateOfFire;
	WeaponInfo.Round = WeaponInfo.Round - 1;
	ChangeDispersionByShot();

	if(WeaponSetting.SoundFireWeapon)
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), WeaponSetting.SoundFireWeapon, ShootLocation->GetComponentLocation());

	int8 NumberProjectile = GetNumberProjectileByShot();

	if (ShootLocation)
	{
		FVector SpawnLocation = ShootLocation->GetComponentLocation();
		FRotator SpawnRotation = ShootLocation->GetComponentRotation();
		FProjectileInfo ProjectileInfo;
		ProjectileInfo = GetProjectile();

		FVector EndLocation;
		for (int8 i = 0; i < NumberProjectile; i++)
		{
			EndLocation = GetFireEndLocation();

			FVector Dir = EndLocation - SpawnLocation;

			Dir.Normalize();

			FMatrix myMatrix(Dir, FVector(0, 1, 0), FVector(0, 0, 1), FVector::ZeroVector);
			SpawnRotation = myMatrix.Rotator();

			if (ProjectileInfo.Projectile)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParams.Owner = GetOwner();
				SpawnParams.Instigator = GetInstigator();

				AProjectileDefault* myProjectile = Cast<AProjectileDefault>(GetWorld()->SpawnActor(ProjectileInfo.Projectile, &SpawnLocation, &SpawnRotation, SpawnParams));
				if (myProjectile)
				{
					myProjectile->InitProjectile(WeaponSetting.ProjectileSetting);
				}
			}
			else
			{
				ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_PhysicsBody);
				const TArray<AActor*> ActorsToIgnore;
				FHitResult HitResult;
				FVector EndHitScanLocation = ShootLocation->GetComponentLocation() + ShootLocation->GetForwardVector() * WeaponSetting.DistacneTrace;

				if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), ShootLocation->GetComponentLocation(), EndHitScanLocation, TraceChannel, false, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true))
				{
					if (HitResult.GetActor() && HitResult.PhysMaterial.IsValid())
					{
						EPhysicalSurface mySurfacetype = UGameplayStatics::GetSurfaceType(HitResult);

						if (WeaponSetting.HitScanDecals.Contains(mySurfacetype))
						{
							UMaterialInterface* myMaterial = WeaponSetting.HitScanDecals[mySurfacetype];

							if (myMaterial && HitResult.GetActor())
							{
								UGameplayStatics::SpawnDecalAttached(myMaterial, FVector(20.0f), HitResult.GetComponent(), NAME_None, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation(), EAttachLocation::KeepWorldPosition, 10.0f);
							}
						}

					}

					if(ShowDebug)
						DrawDebugLine(GetWorld(), ShootLocation->GetComponentLocation(), EndHitScanLocation, FColor::Yellow, false, 5.f, (uint8)'\000', 0.5f);
				}
			}

			AWeaponDefault::BulletEffect();
		}
	}
}

void AWeaponDefault::BulletEffect()
{
	if (!StaticMeshWeapon || !ShellBulletLocation)
		return;

	FVector Location = ShellBulletLocation->GetComponentLocation();
	FRotator Rotation = ShellBulletLocation->GetComponentRotation();

	AStaticMeshActor* ShellBullet = GetWorld()->SpawnActor<AStaticMeshActor>(Location, Rotation);

	if (ShellBullet)
	{
		auto MeshComponent = ShellBullet->GetStaticMeshComponent();

		if (!MeshComponent)
			return;

		ShellBullet->SetLifeSpan(5.0f);

		MeshComponent->SetMobility(EComponentMobility::Movable);
		MeshComponent->SetStaticMesh(WeaponSetting.ShellBullets);
		MeshComponent->SetCollisionProfileName("Pawn");
		MeshComponent->SetSimulatePhysics(true);

		UPrimitiveComponent* PrimitiveComponent = ShellBullet->FindComponentByClass<UPrimitiveComponent>();

		if (PrimitiveComponent)
		{
			PrimitiveComponent->AddImpulse((GetActorRightVector() + GetActorForwardVector() + FVector(0.0f, 0.0f, 2.0f)) * 15.0f);
		}
	}
}



void AWeaponDefault::UpdateStateWeapon(EMovementState NewMovementState)
{
	//ToDo Dispersion
	BlockFire = false;

	switch (NewMovementState)
	{
	case EMovementState::Aim_State:

		CurrentDispersionMax = WeaponSetting.DispersionWeapon.Aim_StateDispersionAimMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.Aim_StateDispersionAimMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.Aim_StateDispersionAimRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.Aim_StateDispersionReduction;
		break;
	case EMovementState::AimWalk_State:

		CurrentDispersionMax = WeaponSetting.DispersionWeapon.AimWalk_StateDispersionAimMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.AimWalk_StateDispersionAimMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.AimWalk_StateDispersionAimRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.Aim_StateDispersionReduction;
		break;
	case EMovementState::Walk_State:

		CurrentDispersionMax = WeaponSetting.DispersionWeapon.Walk_StateDispersionAimMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.Walk_StateDispersionAimMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.Walk_StateDispersionAimRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.Aim_StateDispersionReduction;
		break;
	case EMovementState::Run_State:

		CurrentDispersionMax = WeaponSetting.DispersionWeapon.Run_StateDispersionAimMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.Run_StateDispersionAimMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.Run_StateDispersionAimRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.Aim_StateDispersionReduction;
		break;
	case EMovementState::Sprint_State:
		BlockFire = true;
		SetWeaponStateFire(false);//set fire trigger to false
		//Block Fire
		break;
	default:
		break;
	}
}

void AWeaponDefault::ChangeDispersionByShot()
{
	CurrentDispersion = CurrentDispersion + CurrentDispersionRecoil;
}

float AWeaponDefault::GetCurrentDispersion() const
{
	float Result = CurrentDispersion;
	return Result;
}

FVector AWeaponDefault::ApplyDispersionToShoot(FVector DirectionShoot) const
{
	return FMath::VRandCone(DirectionShoot, GetCurrentDispersion() * PI / 180.f);
}

FVector AWeaponDefault::GetFireEndLocation() const
{
	bool bShootDirection = false;
	FVector EndLocation = FVector(0.f);

	FVector tmpV = (ShootLocation->GetComponentLocation() - ShootEndLocation);
	//UE_LOG(LogTemp, Warning, TEXT("Vector: X = %f. Y = %f. Size = %f"), tmpV.X, tmpV.Y, tmpV.Size());

	if (tmpV.Size() > SizeVectorToChangeShootDirectionLogic)
	{
		EndLocation = ShootLocation->GetComponentLocation() + ApplyDispersionToShoot((ShootLocation->GetComponentLocation() - ShootEndLocation).GetSafeNormal()) * -20000.0f;
		if (ShowDebug)
			DrawDebugCone(GetWorld(), ShootLocation->GetComponentLocation(), -(ShootLocation->GetComponentLocation() - ShootEndLocation), WeaponSetting.DistacneTrace, GetCurrentDispersion() * PI / 180.f, GetCurrentDispersion() * PI / 180.f, 32, FColor::Emerald, false, .1f, (uint8)'\000', 1.0f);
	}
	else
	{
		EndLocation = ShootLocation->GetComponentLocation() + ApplyDispersionToShoot(ShootLocation->GetForwardVector()) * 20000.0f;
		if (ShowDebug)
			DrawDebugCone(GetWorld(), ShootLocation->GetComponentLocation(), ShootLocation->GetForwardVector(), WeaponSetting.DistacneTrace, GetCurrentDispersion() * PI / 180.f, GetCurrentDispersion() * PI / 180.f, 32, FColor::Emerald, false, .1f, (uint8)'\000', 1.0f);
	}

	if (ShowDebug)
	{
		//direction weapon look
		DrawDebugLine(GetWorld(), ShootLocation->GetComponentLocation(), ShootLocation->GetComponentLocation() + ShootLocation->GetForwardVector() * 500.0f, FColor::Cyan, false, 5.f, (uint8)'\000', 0.5f);
		//direction projectile must fly
		DrawDebugLine(GetWorld(), ShootLocation->GetComponentLocation(), ShootEndLocation, FColor::Red, false, 5.f, (uint8)'\000', 0.5f);
		//Direction Projectile Current fly
		DrawDebugLine(GetWorld(), ShootLocation->GetComponentLocation(), EndLocation, FColor::Black, false, 5.f, (uint8)'\000', 0.5f);

		//DrawDebugSphere(GetWorld(), ShootLocation->GetComponentLocation() + ShootLocation->GetForwardVector()*SizeVectorToChangeShootDirectionLogic, 10.f, 8, FColor::Red, false, 4.0f);
	}

	return EndLocation;
}

int8 AWeaponDefault::GetNumberProjectileByShot() const
{
	return WeaponSetting.NumberProjectileByShot;
}

int32 AWeaponDefault::GetWeaponRound()
{
	return WeaponInfo.Round;
}

void AWeaponDefault::InitReload()
{
	WeaponReloading = true;

	ReloadTimer = WeaponSetting.ReloadTime;

	if (WeaponSetting.AnimCharReload)
		OnWeaponReloadStart.Broadcast(WeaponSetting.AnimCharReload);
}

void AWeaponDefault::FinishReload()
{
	WeaponReloading = false;
	WeaponInfo.Round = WeaponSetting.MaxRound;

	OnWeaponReloadEnd.Broadcast();
}
