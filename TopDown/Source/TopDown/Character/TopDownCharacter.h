// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/ArrowComponent.h"
#include "../FuncLibrary/MyTypes.h"
#include "../ProjectileDefault.h"
#include "TopDownCharacter.generated.h"

UCLASS(Blueprintable)
class ATopDownCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	ATopDownCharacter();

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void InputAxisY(float value);
	void InputAxisX(float value);
	void InputWheelAxis(float value);
	void OnSprintKeyPressed();
	void OnSprintKeyReleased();
	void OnRightMouseButtonKeyPressed();
	void OnRightMouseButtonKeyReleased();
	void AttackCharEvent(bool bIsFiring);

	float AxisX = 0.0f;
	float AxisY = 0.0f;

	float AimOffset = 250.0f;
	float MinCameraZoom = 900.0f;
	float MaxCameraZoom = 1800.0f;
	float CameraZoom = 1000.0f;
	float ZoomPower = 300.0f;
	float ZoomSpeed = 3.0f;
	const float StaminaMaxLevel = 100.0f;
	const float StaminaRate = 0.5f;
	float StaminaCurrentLevel = StaminaMaxLevel;
	bool IsAccessSprint = false;
	bool IsPressedKeySprint = false;

	void MovementTick(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void CharacterUpdate();

	UFUNCTION(BlueprintCallable)
	void StaminaUpdate();

	UFUNCTION()
	void ZoomUpdate(float DeltaSeconds);

	UFUNCTION(BlueprintCallable)
	void ChangeMovementState(EMovementState NewMovementState);

	UFUNCTION(BlueprintCallable)
	bool IsAimStatus();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	EMovementState MovementState = EMovementState::Run_State;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	FCharacterSpeed MovementInfo;

	UFUNCTION()
	void CameraAimOffset(APlayerController* myController);

	UFUNCTION()
	void InitWeapon(FName IdWeaponName);

	AWeaponDefault* CurrentWeapon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Init Weapon Class")
	TSubclassOf<AWeaponDefault> InitWeaponClass = nullptr;

	AWeaponDefault* GetCurrentWeapon();

	UFUNCTION(BlueprintCallable)
	void TryReloadWeapon();
	UFUNCTION()
	void WeaponReloadStart(UAnimMontage* Anim);
	UFUNCTION()
	void WeaponReloadEnd();
	//UFUNCTION(BlueprintNativeEvent)
	//void WeaponReloadStart_BP_Implementation(UAnimMontage* Anim);
	//UFUNCTION(BlueprintNativeEvent)
	//void WeaponReloadEnd_BP_Implementation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
	FName InitWeaponName;

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
};
