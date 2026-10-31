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

	// 달리기 상태 및 이동 속도 변경
	void SetRunning(bool bShouldRun);

protected:
	// 캐릭터 위치를 따라가는 Camera를 위한 SpringArmComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "REACT|Camera")
	TObjectPtr<USpringArmComponent> CameraSpringArm;

	// 캐릭터 위치를 따라가는 Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "REACT|Camera")
	TObjectPtr<UCameraComponent> Camera;

	// 캐릭터의 달리기 상태
	UPROPERTY(BlueprintReadOnly, Category = "REACT|Movement")
	bool bIsRunning = false;
};
