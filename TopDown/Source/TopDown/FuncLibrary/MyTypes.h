// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "MyTypes.generated.h"

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Aim_State UMETA(DisplayName = "Aim State"),
	AimWalk_State UMETA(DisplayName = "AimWalk State"),
	Walk_State UMETA(DisplayName = "Walk State"),
	Run_State UMETA(DisplayName = "Run State"),
	Sprint_State UMETA(DisplayName = "Sprint State")
};

UENUM(BlueprintType)
enum class EReloadMagazineStages : uint8
{
	Not_Reload UMETA(DisplayName = "Not Reload"),
	Drop_Magazine UMETA(DisplayName = "Drop Magazine"),
	Take_Magazine UMETA(DisplayName = "Take Magazine"),
	Put_Magazine UMETA(DisplayName = "Put Magazine")
};

USTRUCT(BlueprintType)
struct FCharacterSpeed
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AimSpeed = 80.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 150.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RunSpeed = 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed = 400.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AimSpeedWalk = 100.0f;
};


USTRUCT(BlueprintType)
struct FProjectileInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	TSubclassOf<class AProjectileDefault> Projectile = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ProjectileDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ProjectileLifeTime = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ProjectileInitSpeed = 2000.0f;

	//material to decal on hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	TMap<TEnumAsByte<EPhysicalSurface>, UMaterialInterface*> HitDecals;

	//Sound when hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	USoundBase* HitSound = nullptr;
	//fx when hit check by surface
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	TMap<TEnumAsByte<EPhysicalSurface>, UParticleSystem*> HitFXs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	UParticleSystem* ExploseFX = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	USoundBase* ExploseSound = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ProjectileMaxRadiusDamage = 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ExploseMaxDamage = 40.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ExploseDecreaseRelativeCenter = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ExploseDistanceMaxDamage = 40.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSetting")
	float ExploseDistanceDecreaseDamage = 30.0f;
	//Timer add
};

USTRUCT(BlueprintType)
struct FWeaponDispersion
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Aim_StateDispersionAimMax = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Aim_StateDispersionAimMin = 0.3f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Aim_StateDispersionAimRecoil = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Aim_StateDispersionReduction = .3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float AimWalk_StateDispersionAimMax = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float AimWalk_StateDispersionAimMin = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float AimWalk_StateDispersionAimRecoil = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float AimWalk_StateDispersionReduction = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Walk_StateDispersionAimMax = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Walk_StateDispersionAimMin = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Walk_StateDispersionAimRecoil = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Walk_StateDispersionReduction = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Run_StateDispersionAimMax = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Run_StateDispersionAimMin = 4.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Run_StateDispersionAimRecoil = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	float Run_StateDispersionReduction = 0.1f;
};

USTRUCT(BlueprintType)
struct FAddicionalWeaponInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	int32 Round = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	int32 ReloadTime = 2;
};

USTRUCT(BlueprintType)
struct FWeaponInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TSubclassOf<class AWeaponDefault> WeaponClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float RateOfFire = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float ReloadTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	int32 MaxRound = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	int32 NumberProjectileByShot = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion ")
	FWeaponDispersion DispersionWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound ")
	USoundBase* SoundFireWeapon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound ")
	USoundBase* SoundReloadWeapon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX ")
	UNiagaraSystem* EffectFireWeapon = nullptr;

	//if null use trace logic (TSubclassOf<class AProjectileDefault> Projectile = nullptr)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile ")
	FProjectileInfo ProjectileSetting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace ")
	float WeaponDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace ")
	float DistacneTrace = 2000.0f;

	//one decal on all?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitEffect ")
	UDecalComponent* DecalOnHit = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim ")
	UAnimMontage* AnimCharFire = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim ")
	UAnimMontage* AnimCharReload = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh ")
	UStaticMesh* MagazineDrop = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh ")
	UStaticMesh* ShellBullets = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitEffect ")
	TMap<TEnumAsByte<EPhysicalSurface>, UMaterialInterface*> HitScanDecals;
};

UCLASS()
class UMyTypes : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
};