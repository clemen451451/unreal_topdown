// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileDefault_Grenade.h"
#include "Kismet/GameplayStatics.h"

void AProjectileDefault_Grenade::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay AProjectileDefault_Grenade"));
}

void AProjectileDefault_Grenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TimerExplose(DeltaTime);
}

void AProjectileDefault_Grenade::TimerExplose(float DeltaTime)
{
	if (TimerEnabled)
	{
		if (TimerToExplose > TimeToExplose)
		{
			//Explose
			Explose();

		}
		else
		{
			TimerToExplose += DeltaTime;
		}
	}
}

void AProjectileDefault_Grenade::BulletCollisionSphereHit(class UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::BulletCollisionSphereHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileDefault_Grenade::ImpactProjectile()
{
	UE_LOG(LogTemp, Warning, TEXT("AProjectileDefault_Grenade::ImpactProjectile"));
	//Init Grenade
	TimerEnabled = true;
}

void AProjectileDefault_Grenade::Explose()
{
	UE_LOG(LogTemp, Warning, TEXT("Explose"));

	TimerEnabled = false;

	if (ProjectileSetting.ExploseFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ProjectileSetting.ExploseFX, GetActorLocation(), GetActorRotation(), FVector(1.0f));
	}
	if (ProjectileSetting.ExploseSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ProjectileSetting.ExploseSound, GetActorLocation());
	}

	DrawDebugSphere(GetWorld(), GetActorLocation(), ProjectileSetting.ProjectileMaxRadiusDamage, 8, FColor::Yellow, false, 6.0f); // радиус предельного дамага, + обозначение конца

	DrawDebugSphere(GetWorld(), GetActorLocation(), ProjectileSetting.ExploseDistanceMaxDamage, 8, FColor::Red, false, 6.0f); // дистанция для максимального дамага внутри взрыва

	DrawDebugSphere(GetWorld(), GetActorLocation(), ProjectileSetting.ProjectileMaxRadiusDamage / 2.0f, 8, FColor::Blue, false, 6.0f); // дистанция для 50% дамага от общего


	TArray<AActor*> IgnoredActor;
	UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(),
		ProjectileSetting.ExploseMaxDamage,
		ProjectileSetting.ExploseMaxDamage * 0.2f,
		GetActorLocation(),
		1000.0f,
		2000.0f,
		5,
		NULL, IgnoredActor, nullptr, nullptr);

	this->Destroy();
}
