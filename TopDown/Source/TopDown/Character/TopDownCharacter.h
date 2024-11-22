// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TopDown/FuncLibrary/MyTypes.h"
#include "TopDownCharacter.generated.h"

UCLASS(Blueprintable)
class ATopDownCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATopDownCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void InputAxisY(float value);
	void InputAxisX(float value);
	void InputWheelAxis(float value);
	void OnSprintKeyPressed();
	void OnSprintKeyReleased();

	float AxisX = 0.0f;
	float AxisY = 0.0f;

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

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
};

