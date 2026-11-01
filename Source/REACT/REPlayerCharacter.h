#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "REPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;

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

	// 기본 공격 실행을 위한 함수
	void StartBasicAttack();

	// 현재 플레이어 공격 상태 확인
	bool IsAttacking() const;

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

	// 기본 공격 애니메이션 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "REACT|Combat")
	TObjectPtr<UAnimMontage> BasicAttackMontage;

	// 현재 플레이어 공격 상태
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "REACT|Combat")
	bool bIsAttacking = false;

	// 현재 공격의 콤보 단계
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "REACT|Combat")
	int32 CurrentComboIndex = 0;

private:
	// 콤보 연계 구간의 진행 상태
	enum class EComboWindowState : uint8
	{
		NotOpened,
		Open,
		Expired
	};

	// 현재 재생 중인 공격 섹션
	FName ActiveComboSection = NAME_None;

	// 최대 공격 횟수
	static constexpr int32 MaxComboCount = 3;

	// 처리 대기 중인 추가 공격 입력 개수
	int32 PendingComboInputs = 0;

	// 현재 콤보 연계 구간 상태
	EComboWindowState ComboWindowState = EComboWindowState::NotOpened;

	// 다음 공격으로의 전환이 예약되었는지 여부
	bool bComboTransitionQueued = false;

	// 이미 공격 중인 상태에서 추가 콤보 공격 입력이 들어온 경우 호출되는 함수
	void BufferComboInput();

	// 다음 공격 섹션으로의 연결을 확정하는 함수
	void CommitComboInput();

	// 현재 몽타주 섹션과 C++ 상태를 동기화하는 함수
	void SyncComboSection(FName SectionName);

	// 콤보 관련 임시 상태를 초기화하는 함수
	void ResetComboState();

	// 연계 가능 구간 시작
	UFUNCTION()
	void OnComboNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload);

	// 연계 가능 구간 종료
	UFUNCTION()
	void OnComboNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& Payload);

	// 몽타주 섹션이 변경될 때 현재 콤보 단계를 갱신하는 함수
	void OnComboSectionChanged(UAnimMontage* Montage, FName SectionName, bool bLooped);

	// 기본 공격 애니메이션 몽타주 종료 시 호출되는 함수
	void OnBasicAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
