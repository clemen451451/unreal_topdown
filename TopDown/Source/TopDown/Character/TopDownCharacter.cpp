// Copyright Epic Games, Inc. All Rights Reserved.

#include "TopDownCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "../Game/TopDownGameInstance.h"

ATopDownCharacter::ATopDownCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-80.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ATopDownCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitWeapon(InitWeaponName);
}

void ATopDownCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	ATopDownCharacter::MovementTick(DeltaSeconds);
	ATopDownCharacter::ZoomUpdate(DeltaSeconds);
}

void ATopDownCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ATopDownCharacter::InputAxisY);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATopDownCharacter::InputAxisX);
	PlayerInputComponent->BindAxis("MouseWheel", this, &ATopDownCharacter::InputWheelAxis);

	PlayerInputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &ATopDownCharacter::OnSprintKeyPressed);
	PlayerInputComponent->BindKey(EKeys::LeftShift, IE_Released, this, &ATopDownCharacter::OnSprintKeyReleased);

	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ATopDownCharacter::OnRightMouseButtonKeyPressed);
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &ATopDownCharacter::OnRightMouseButtonKeyReleased);

	PlayerInputComponent->BindKey(EKeys::R, IE_Released, this, &ATopDownCharacter::TryReloadWeapon);

	ChangeMovementState(EMovementState::Walk_State);
}

void ATopDownCharacter::OnSprintKeyPressed()
{
	IsPressedKeySprint = true;
	ChangeMovementState(EMovementState::Sprint_State);
}

void ATopDownCharacter::OnSprintKeyReleased()
{
	IsPressedKeySprint = false;
	ChangeMovementState(EMovementState::Run_State);
}

void ATopDownCharacter::OnRightMouseButtonKeyPressed()
{
	UE_LOG(LogTemp, Warning, TEXT("OnRightMouseButtonKeyPressed"));

	AttackCharEvent(true);
}

void ATopDownCharacter::OnRightMouseButtonKeyReleased()
{
	AttackCharEvent(false);
}

void ATopDownCharacter::InputAxisY(float value)
{
	AxisY = value;
}

void ATopDownCharacter::InputAxisX(float value)
{
	AxisX = value;
}

void ATopDownCharacter::InputWheelAxis(float value)
{
	CameraZoom = FMath::Clamp((CameraZoom + (value * ZoomPower)), MinCameraZoom, MaxCameraZoom);
}

void ATopDownCharacter::AttackCharEvent(bool bIsFiring)
{
	AWeaponDefault* myWeapon = nullptr;
	myWeapon = GetCurrentWeapon();
	if (myWeapon)
	{
		//ToDo Check melee or range
		myWeapon->SetWeaponStateFire(bIsFiring);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("ATPSCharacter::AttackCharEvent - CurrentWeapon -NULL"));
}

AWeaponDefault* ATopDownCharacter::GetCurrentWeapon()
{
	return CurrentWeapon;
}

void ATopDownCharacter::MovementTick(float DeltaTime)
{
	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), AxisY);
	AddMovementInput(FVector(0.0f, 1.0f, 0.0f), AxisX);
	
	APlayerController* myController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (myController)
	{
		FHitResult ResultHit;
		myController->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery6, false, ResultHit);

		float FindRotatorResultYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ResultHit.Location).Yaw;

		SetActorRotation(FQuat(FRotator(0.0f, FindRotatorResultYaw, 0.0f)));

		auto forwardVector = GetActorForwardVector();
		auto velocity = GetVelocity();

		velocity.Normalize();
		forwardVector = forwardVector * velocity;

		if ((forwardVector.X >= 0.2f && forwardVector.Y >= 0.2f) || (forwardVector.X >= 0.9f || forwardVector.Y >= 0.9f))
		{
			IsAccessSprint = true;

			if(IsPressedKeySprint)
				ChangeMovementState(EMovementState::Sprint_State);
		}
		else
		{
			if (MovementState == EMovementState::Sprint_State)
				ChangeMovementState(EMovementState::Run_State);

			IsAccessSprint = false;
		}

		if (CurrentWeapon)
		{
			CurrentWeapon->ShootEndLocation = ResultHit.Location + 0.0f; // offset. ������� ��������� � ����������� �� ���������
		}

		ATopDownCharacter::StaminaUpdate();
		ATopDownCharacter::CameraAimOffset(myController);
	}
}


void ATopDownCharacter::StaminaUpdate()
{
	if (MovementState == EMovementState::Sprint_State)
	{
		if (StaminaCurrentLevel >= StaminaRate)
		{
			StaminaCurrentLevel -= StaminaRate;

			if (StaminaCurrentLevel < StaminaRate)
			{
				IsPressedKeySprint = false;
				ChangeMovementState(EMovementState::Run_State);
			}
		}
	}
	else if (MovementState != EMovementState::Sprint_State)
	{
		if (StaminaCurrentLevel < StaminaMaxLevel)
			StaminaCurrentLevel += StaminaRate;
	}

	//UE_LOG(LogTemp, Warning, TEXT("STAMINA %f"), StaminaCurrentLevel);
}

void ATopDownCharacter::CharacterUpdate()
{
	float ResSpeed = 150.0f;

	switch (MovementState)
	{
	case EMovementState::Aim_State:
		ResSpeed = MovementInfo.AimSpeed;
		break;
	case EMovementState::Walk_State:
		ResSpeed = MovementInfo.WalkSpeed;
		break;
	case EMovementState::Run_State:
		ResSpeed = MovementInfo.RunSpeed;
		break;
	case EMovementState::Sprint_State:
		ResSpeed = MovementInfo.SprintSpeed;
		break;
	default:
		break;
	}

	GetCharacterMovement()->MaxWalkSpeed = ResSpeed;
}

void ATopDownCharacter::ChangeMovementState(EMovementState NewMovementState)
{
	if(MovementState == EMovementState::Aim_State && NewMovementState != EMovementState::Aim_State)
		CameraBoom->SetWorldLocation(GetActorLocation());

	if (NewMovementState == EMovementState::Sprint_State && !IsAccessSprint)
		MovementState = EMovementState::Run_State;
	else
		MovementState = NewMovementState;

	CharacterUpdate();

	AWeaponDefault* myWeapon = GetCurrentWeapon();
	if (myWeapon)
	{
		myWeapon->UpdateStateWeapon(MovementState);  
	}
}

bool ATopDownCharacter::IsAimStatus()
{
	if (MovementState == EMovementState::Aim_State)
		return true;
	return false;
}

void ATopDownCharacter::CameraAimOffset(APlayerController* myController)
{
	if (MovementState == EMovementState::Aim_State)
	{
		FHitResult ResultHit;
		myController->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery6, false, ResultHit);

		FVector Offset;

		FVector ActorLocation = GetActorLocation();
		ActorLocation.Normalize();

		ResultHit.Location.Normalize();

		float Angle = FVector::DotProduct(ActorLocation, ResultHit.Location);
		
		Offset.X = AimOffset * sin(Angle);
		Offset.Y = AimOffset * cos(Angle);
		Offset.Z = CameraBoom->GetRelativeLocation().Z;

		CameraBoom->SetRelativeLocation(Offset);
	}
}

void ATopDownCharacter::InitWeapon(FName IdWeaponName)
{
	UTopDownGameInstance* myGI = Cast<UTopDownGameInstance>(GetGameInstance());
	FWeaponInfo myWeaponInfo;
	if (myGI)
	{
		if (myGI->GetWeaponInfoByName(IdWeaponName, myWeaponInfo))
		{
			if (myWeaponInfo.WeaponClass)
			{
				FVector SpawnLocation = FVector(0);
				FRotator SpawnRotation = FRotator(0);

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParams.Owner = GetOwner();
				SpawnParams.Instigator = GetInstigator();

				AWeaponDefault* myWeapon = Cast<AWeaponDefault>(GetWorld()->SpawnActor(myWeaponInfo.WeaponClass, &SpawnLocation, &SpawnRotation, SpawnParams));
				if (myWeapon)
				{
					FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
					myWeapon->AttachToComponent(GetMesh(), Rule, FName("WeaponSocketRightHand"));
					CurrentWeapon = myWeapon;

					myWeapon->WeaponSetting = myWeaponInfo;
					myWeapon->UpdateStateWeapon(MovementState);

					myWeapon->OnWeaponReloadStart.AddDynamic(this, &ATopDownCharacter::WeaponReloadStart);
					myWeapon->OnWeaponReloadEnd.AddDynamic(this, &ATopDownCharacter::WeaponReloadEnd);

					myWeapon->WeaponInit();
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ATopDownCharacter::InitWeapon - Weapon not found in table -NULL"));
		}
	}
}

void ATopDownCharacter::TryReloadWeapon()
{
	if (CurrentWeapon)
	{
		if (CurrentWeapon->GetWeaponRound() <= CurrentWeapon->WeaponSetting.MaxRound)
			CurrentWeapon->InitReload();
	}
}

void ATopDownCharacter::WeaponReloadStart(UAnimMontage* Anim)
{
	WeaponReloadStart_BP(Anim);

	if (CurrentWeapon)
	{
		CurrentWeapon->SetWeaponStateFire(false);

		if (CurrentWeapon->WeaponSetting.AnimCharReload)
		{
			PlayAnimMontage(CurrentWeapon->WeaponSetting.AnimCharReload);

			CurrentReloadMagazineStage = EReloadMagazineStages::Drop_Magazine;
			GetWorldTimerManager().SetTimer(ReloadMagazineTimerHandle, this, &ATopDownCharacter::OnReloadMagazineTimer, 0.7f, false);
		}
	}
}

void ATopDownCharacter::OnReloadMagazineTimer()
{
	switch (CurrentReloadMagazineStage)
	{
		case EReloadMagazineStages::Drop_Magazine:
		{
			if (CurrentWeapon->WeaponSetting.MagazineDrop)
			{
				USceneComponent* MagazineComponent = CurrentWeapon->StaticMeshWeapon->GetChildComponent(0);

				if (!MagazineComponent)
					return;

				FVector MagazineLocation = MagazineComponent->GetComponentLocation();
				FRotator MagazineRotation = MagazineComponent->GetComponentRotation();
				FVector MagazineScale = MagazineComponent->GetComponentScale();

				MagazineComponent->SetVisibility(false);

				AStaticMeshActor* MagazineActor = GetWorld()->SpawnActor<AStaticMeshActor>(MagazineLocation, MagazineRotation);

				if (MagazineActor)
				{
					auto MeshComponent = MagazineActor->GetStaticMeshComponent();

					if (!MeshComponent)
						return;

					MagazineActor->SetMobility(EComponentMobility::Movable);
					MagazineActor->SetActorScale3D(MagazineScale);

					MeshComponent->SetStaticMesh(CurrentWeapon->WeaponSetting.MagazineDrop);
					MeshComponent->SetCollisionProfileName("Pawn");
					MeshComponent->SetSimulatePhysics(true);

					UPrimitiveComponent* PrimitiveComponent = MagazineActor->FindComponentByClass<UPrimitiveComponent>();

					if (PrimitiveComponent)
					{
						PrimitiveComponent->AddImpulse((-GetActorRightVector() + GetActorForwardVector()) * 30.0f);
					}
				}
			}

			CurrentReloadMagazineStage = EReloadMagazineStages::Take_Magazine;

			GetWorldTimerManager().SetTimer(ReloadMagazineTimerHandle, this, &ATopDownCharacter::OnReloadMagazineTimer, 0.6f, false);

			UE_LOG(LogTemp, Warning, TEXT("Drop Magazine"));
			break;
		}
		case EReloadMagazineStages::Take_Magazine:
		{
			if (CurrentWeapon->WeaponSetting.MagazineDrop)
			{
				USceneComponent* MagazineComponent = CurrentWeapon->StaticMeshWeapon->GetChildComponent(0);

				if (!MagazineComponent)
					return;

				TempSaveMagazineComponent = MagazineComponent;
				TempSaveMagazineTransform = MagazineComponent->GetRelativeTransform();

				FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);

				MagazineComponent->AttachToComponent(GetMesh(), Rule, FName("MagazineSocketLeftHand"));
				MagazineComponent->SetVisibility(true);

				CurrentReloadMagazineStage = EReloadMagazineStages::Put_Magazine;

				GetWorldTimerManager().SetTimer(ReloadMagazineTimerHandle, this, &ATopDownCharacter::OnReloadMagazineTimer, 1.3f, false);

				UE_LOG(LogTemp, Warning, TEXT("Take Magazine"));
			}
			break;
		}
		case EReloadMagazineStages::Put_Magazine:
		{
			if (CurrentWeapon->WeaponSetting.MagazineDrop)
			{
				if (!TempSaveMagazineComponent)
					return;

				FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);

				TempSaveMagazineComponent->AttachToComponent(CurrentWeapon->StaticMeshWeapon, Rule);
				TempSaveMagazineComponent->SetRelativeTransform(TempSaveMagazineTransform);


				CurrentReloadMagazineStage = EReloadMagazineStages::Not_Reload;

				UE_LOG(LogTemp, Warning, TEXT("Put Magazine"));
			}
			break;
		}
		default:
		{
			CurrentReloadMagazineStage = EReloadMagazineStages::Not_Reload;
			break;
		}
	}
}


void ATopDownCharacter::WeaponReloadEnd()
{
	WeaponReloadEnd_BP();
}


void ATopDownCharacter::WeaponReloadStart_BP(UAnimMontage* Anim)
{
}

void ATopDownCharacter::WeaponReloadEnd_BP()
{
}


void ATopDownCharacter::WeaponReloadStart_BP_Implementation(UAnimMontage* Anim)
{
	// in BP
}

void ATopDownCharacter::WeaponReloadEnd_BP_Implementation()
{
	// in BP
}

void ATopDownCharacter::ZoomUpdate(float DeltaSeconds)
{
	FRotator RotatorCamera(-80.0f, 0.0f, 0.0f);

	float PrevTargetArmLength = CameraBoom->TargetArmLength;
	float NewZoom = FMath::FInterpTo(PrevTargetArmLength, CameraZoom, DeltaSeconds, ZoomSpeed);

	CameraBoom->TargetArmLength = NewZoom;
	CameraBoom->SetRelativeRotation(RotatorCamera);
}
