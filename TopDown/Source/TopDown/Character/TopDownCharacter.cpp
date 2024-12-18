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

		ATopDownCharacter::StaminaUpdate();
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

	UE_LOG(LogTemp, Warning, TEXT("STAMINA %f"), StaminaCurrentLevel);
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
	if (NewMovementState == EMovementState::Sprint_State && !IsAccessSprint)
		MovementState = EMovementState::Run_State;
	else
		MovementState = NewMovementState;

	CharacterUpdate();
}

bool ATopDownCharacter::IsAimStatus()
{
	if (MovementState == EMovementState::Aim_State)
		return true;
	return false;
}

void ATopDownCharacter::ZoomUpdate(float DeltaSeconds)
{
	FRotator RotatorCamera(-80.0f, 0.0f, 0.0f);

	float PrevTargetArmLength = CameraBoom->TargetArmLength;
	float NewZoom = FMath::FInterpTo(PrevTargetArmLength, CameraZoom, DeltaSeconds, ZoomSpeed);

	CameraBoom->TargetArmLength = NewZoom;
	CameraBoom->SetRelativeRotation(RotatorCamera);
}