#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "REPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class REACT_API AREPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	// 게임 시작 시 입력 매핑 등록
	virtual void BeginPlay() override;

	// Input Action 이벤트 연결
	virtual void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly, Category = "REACT|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "REACT|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "REACT|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "REACT|Input")
	TObjectPtr<UInputAction> BasicAttackAction;

private:
	void HandleMoveStarted(const FInputActionValue& Value);
	void HandleMove(const FInputActionValue& Value);
	void HandleMoveReleased();
	void HandleJumpStarted();
	void HandleJumpStopped();

	void HandleBasicAttackStarted();

	// 동일 방향으로의 연속 입력 허용 시간
	static constexpr double DoubleTapWindow = 0.25;

	// 현재 누르고 있는 이동 방향
	int32 HeldMoveDirection = 0;

	// 마지막으로 누른 이동 방향
	int32 LastTapDirection = 0;

	// 마지막 방향키 입력 시각
	double LastTapTime = -1.0;
};
