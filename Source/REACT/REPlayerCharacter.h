#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "REPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class REACT_API AREPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AREPlayerCharacter();

	// X축 방향 이동을 위한 함수
	void MoveHorizontal(float InputValue);

protected:
	// 캐릭터 위치를 따라가는 Camera를 위한 SpringArmComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "REACT|Camera")
	TObjectPtr<USpringArmComponent> CameraSpringArm;

	// 캐릭터 위치를 따라가는 Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "REACT|Camera")
	TObjectPtr<UCameraComponent> Camera;
};
