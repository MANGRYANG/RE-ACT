#include "REPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"

namespace
{
	const FName Attack1Section(TEXT("Attack_1"));
	const FName Attack2Section(TEXT("Attack_2"));
	const FName Attack3Section(TEXT("Attack_3"));
	const FName ComboWindowNotify(TEXT("ComboWindow"));
}

AREPlayerCharacter::AREPlayerCharacter()
{
	// 별도의 Tick 처리가 필요하지 않으므로 비활성화
	PrimaryActorTick.bCanEverTick = false;

	// 컨트롤러 회전이 캐릭터에 영향을 주지 않도록 설정
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 기본 캐릭터 이동 설정
	UCharacterMovementComponent* Movement = GetCharacterMovement();

	Movement->MaxWalkSpeed = 450.0f;
	Movement->JumpZVelocity = 900.0f;
	Movement->GravityScale = 1.75f;
	Movement->AirControl = 0.35f;

	// 이동 방향에 따라 캐릭터가 회전하지 않도록 설정
	Movement->bOrientRotationToMovement = false;
	Movement->bUseControllerDesiredRotation = false;

	// 캐릭터가 이동 가능한 평면을 XZ 평면으로 제한
	Movement->SetPlaneConstraintNormal(FVector(0.0f, 1.0f, 0.0f));

	// 제한 평면의 원점을 월드 좌표계 오리진으로 설정
	Movement->SetPlaneConstraintOrigin(FVector::ZeroVector);

	// 평면 제한 기능 활성화
	Movement->SetPlaneConstraintEnabled(true);

	// CameraSpringArm 컴포넌트 생성 및 초기화
	CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));

	// CameraSpringArm을 캐릭터의 루트 컴포넌트에 부착
	CameraSpringArm->SetupAttachment(GetRootComponent());

	// CameraSpringArm 회전 설정
	CameraSpringArm->SetUsingAbsoluteRotation(true);
	CameraSpringArm->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	// CameraSpringArm 위치 설정
	CameraSpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));

	// CameraSpringArm 길이 설정
	CameraSpringArm->TargetArmLength = 850.0f;

	// CameraSpringArm이 캐릭터와 분리되어 회전하지 않도록 설정
	CameraSpringArm->bUsePawnControlRotation = false;

	// Camera 충돌 테스트 비활성화
	CameraSpringArm->bDoCollisionTest = false;

	// Camera 컴포넌트 생성 및 초기화
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));

	// Camera를 CameraSpringArm에 부착
	Camera->SetupAttachment(CameraSpringArm, USpringArmComponent::SocketName);

	// Camera가 CameraSpringArm과 분리되어 회전하지 않도록 설정
	Camera->bUsePawnControlRotation = false;
}

void AREPlayerCharacter::MoveHorizontal(float InputValue)
{
	// 공격 중 이동 및 방향 전환 제한
	if (bIsAttacking)
	{
		return;
	}

	// 입력 값이 0에 가까운 경우 무시
	if (FMath::IsNearlyZero(InputValue))
	{
		return;
	}

	const float MoveValue = FMath::Clamp(InputValue, -1.0f, 1.0f);

	// 입력 방향에 따라 바라보는 방향 결정
	const float TargetYaw = (MoveValue > 0.0f) ? 0.0f : 180.0f;

	// 캐릭터를 해당 방향으로 즉시 회전
	SetActorRotation(FRotator(0.0f, TargetYaw, 0.0f));

	// 캐릭터의 이동 입력 처리
	AddMovementInput(FVector::ForwardVector, MoveValue);
}

void AREPlayerCharacter::SetRunning(bool bShouldRun)
{
	bIsRunning = bShouldRun;

	GetCharacterMovement()->MaxWalkSpeed = bIsRunning ? 1050.0f : 450.0f;
}

void AREPlayerCharacter::StartBasicAttack()
{
	// 이미 공격 중인 경우 다음 콤보 입력 처리
	if (bIsAttacking)
	{
		BufferComboInput();
		return;
	}

	UCharacterMovementComponent* Movement = GetCharacterMovement();

	// 지상에서만 기본 공격 허용
	if (!Movement || !Movement->IsMovingOnGround())
	{
		return;
	}

	// 캐릭터의 애니메이션 인스턴스
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	// 애니메이션 인스턴스가 유효하지 않은 경우
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player AnimInstance is not valid."));
		return;
	}

	// 기본 공격 몽타주가 지정되지 않은 경우
	if (!BasicAttackMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("BasicAttackMontage is not assigned."));
		return;
	}

	// 기본 공격 몽타주 재생
	const float MontageDuration = AnimInstance->Montage_Play(BasicAttackMontage.Get());

	if (MontageDuration <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to play BasicAttackMontage."));
		return;
	}

	// 이전 콤보 정보 초기화
	ResetComboState();

	// 공격 상태 진입
	bIsAttacking = true;

	// 현재 콤보 단계 및 활성화된 공격 섹션 설정
	CurrentComboIndex = 1;
	ActiveComboSection = Attack1Section;

	// 첫 번째 공격 섹션으로 점프
	AnimInstance->Montage_JumpToSection
	(
		Attack1Section,
		BasicAttackMontage.Get()
	);

	// 공격 시작 시 플레이어 이동 중단
	Movement->StopMovementImmediately();

	// 이전 프레임에 누적된 이동 입력 제거
	ConsumeMovementInputVector();

	// 몽타주 노티피케이션 시작 이벤트 연결
	AnimInstance->OnPlayMontageNotifyBegin.AddUniqueDynamic
	(
		this,
		&AREPlayerCharacter::OnComboNotifyBegin
	);

	// 몽타주 노티피케이션 종료 이벤트 연결
	AnimInstance->OnPlayMontageNotifyEnd.AddUniqueDynamic
	(
		this,
		&AREPlayerCharacter::OnComboNotifyEnd
	);

	// 몽타주 섹션 변경 이벤트에 대한 델리게이트 연결
	FOnMontageSectionChanged SectionChangedDelegate;

	SectionChangedDelegate.BindUObject
	(
		this,
		&AREPlayerCharacter::OnComboSectionChanged
	);

	AnimInstance->Montage_SetSectionChangedDelegate
	(
		SectionChangedDelegate,
		BasicAttackMontage.Get()
	);

	// 몽타주 종료 이벤트에 대한 델리게이트 연결
	FOnMontageEnded MontageEndedDelegate;

	MontageEndedDelegate.BindUObject
	(
		this,
		&AREPlayerCharacter::OnBasicAttackMontageEnded
	);

	AnimInstance->Montage_SetEndDelegate
	(
		MontageEndedDelegate,
		BasicAttackMontage.Get()
	);
}

bool AREPlayerCharacter::IsAttacking() const
{
	return bIsAttacking;
}

void AREPlayerCharacter::BufferComboInput()
{
	// 캐릭터의 애니메이션 인스턴스
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	// 애니메이션 인스턴스나 기본 공격 몽타주가 유효하지 않거나 몽타주가 종료된 경우 무시
	if (!AnimInstance || !BasicAttackMontage || !AnimInstance->Montage_IsPlaying(BasicAttackMontage.Get()))
	{
		return;
	}

	// 현재 활성화된 공격 콤보 섹션 가져오기
	const FName SectionName = AnimInstance->Montage_GetCurrentSection(BasicAttackMontage.Get());

	// 현재 활성화된 공격 콤보 섹션과 C++ 상태 동기화
	SyncComboSection(SectionName);

	// 첫 번째 공격과 두 번째 공격에서만 추가 콤보 입력 처리
	if (SectionName != Attack1Section && SectionName != Attack2Section)
	{
		return;
	}

	// 콤보 연계 구간이 만료된 상태이고 다음 공격이 확정되지 않은 경우 추가 입력 무시
	if (ComboWindowState == EComboWindowState::Expired && !bComboTransitionQueued)
	{
		return;
	}

	// 이미 예약된 다음 공격도 총 공격 횟수에 포함
	const int32 ReservedTransitionCount = bComboTransitionQueued ? 1 : 0;

	// 현재 공격과 이미 예약된 다음 공격을 제외한 추가 입력 버퍼 용량 계산
	const int32 MaxBufferedInputs = MaxComboCount - CurrentComboIndex - ReservedTransitionCount;

	// 최대 콤보 입력 수를 초과하지 않도록 제한
	if (MaxBufferedInputs <= 0 || PendingComboInputs >= MaxBufferedInputs)
	{
		return;
	}

	// 추가 공격 입력 누적
	++PendingComboInputs;

	// 현재 연계 가능한 시점이라면 다음 공격 연결
	if (ComboWindowState == EComboWindowState::Open && !bComboTransitionQueued)
	{
		CommitComboInput();
	}
}


void AREPlayerCharacter::CommitComboInput()
{
	// 저장된 입력이 없는 경우 무시
	if (PendingComboInputs <= 0)
	{
		return;
	}

	// 콤보 연계 구간이 열려 있지 않거나 이미 다음 공격이 예약된 경우 무시
	if (ComboWindowState != EComboWindowState::Open || bComboTransitionQueued)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	// 애니메이션 인스턴스나 기본 공격 몽타주가 유효하지 않거나 몽타주가 종료된 경우 무시
	if (!AnimInstance || !BasicAttackMontage || !AnimInstance->Montage_IsPlaying(BasicAttackMontage.Get()))
	{
		return;
	}

	FName NextSection = NAME_None;

	// 현재 섹션에 따라 다음 공격 섹션 결정
	if (ActiveComboSection == Attack1Section)
	{
		NextSection = Attack2Section;
	}
	else if (ActiveComboSection == Attack2Section)
	{
		NextSection = Attack3Section;
	}
	else
	{
		return;
	}

	// 현재 섹션 종료 후 다음 공격을 재생하도록 연결
	AnimInstance->Montage_SetNextSection
	(
		ActiveComboSection,
		NextSection,
		BasicAttackMontage.Get()
	);

	// 다음 공격 예약에 사용한 입력 소비
	--PendingComboInputs;

	// 동일 섹션에서 중복으로 예약하지 않도록 방지
	bComboTransitionQueued = true;
}

void AREPlayerCharacter::SyncComboSection(FName SectionName)
{
	// 유효한 공격 섹션인지 확인
	if (SectionName != Attack1Section && SectionName != Attack2Section && SectionName != Attack3Section)
	{
		return;
	}

	// 이미 활성화된 섹션이면 무시
	if (ActiveComboSection == SectionName)
	{
		return;
	}

	// 현재 활성화된 섹션 정보 갱신
	ActiveComboSection = SectionName;

	// 새로운 공격 섹션의 연계 구간 및 전환 예약 상태 초기화
	ComboWindowState = EComboWindowState::NotOpened;
	bComboTransitionQueued = false;

	if (SectionName == Attack1Section)
	{
		CurrentComboIndex = 1;
	}
	else if (SectionName == Attack2Section)
	{
		CurrentComboIndex = 2;
	}
	else if (SectionName == Attack3Section)
	{
		CurrentComboIndex = 3;
	}
}

void AREPlayerCharacter::ResetComboState()
{
	CurrentComboIndex = 0;
	ActiveComboSection = NAME_None;

	PendingComboInputs = 0;

	ComboWindowState = EComboWindowState::NotOpened;
	bComboTransitionQueued = false;
}

void AREPlayerCharacter::OnComboNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
	// 콤보 연계 Notify가 아니거나 공격 중이 아닌 경우 무시
	if (NotifyName != ComboWindowNotify || !bIsAttacking)
	{
		return;
	}

	// 현재 캐릭터 메시에서 발생한 이벤트인지 확인
	if (Payload.SkelMeshComponent != GetMesh())
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	// 애니메이션 인스턴스가 유효하지 않거나 기본 공격 몽타주가 재생 중이 아닌 경우 무시
	if (!AnimInstance || !BasicAttackMontage || !AnimInstance->Montage_IsPlaying(BasicAttackMontage.Get()))
	{
		return;
	}

	// 현재 재생 중인 몽타주 섹션 가져오기
	const FName SectionName = AnimInstance->Montage_GetCurrentSection(BasicAttackMontage.Get());

	// 첫 번째 공격과 두 번째 공격의 연계 구간에서만 콤보 공격 입력 허용
	if (SectionName != Attack1Section && SectionName != Attack2Section)
	{
		return;
	}

	// 현재 활성화된 공격 콤보 섹션과 C++ 상태 동기화
	SyncComboSection(SectionName);

	// 현재 섹션의 연계 가능 구간 상태를 Open으로 설정
	ComboWindowState = EComboWindowState::Open;

	// 저장된 추가 입력이 있으면 다음 공격 예약
	if (PendingComboInputs > 0)
	{
		CommitComboInput();
	}
}

void AREPlayerCharacter::OnComboNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
	// 콤보 연계 Notify가 아니거나 공격 중이 아닌 경우 무시
	if (NotifyName != ComboWindowNotify || !bIsAttacking)
	{
		return;
	}

	// 현재 캐릭터 메시에서 발생한 이벤트인지 확인
	if (Payload.SkelMeshComponent != GetMesh())
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	// 애니메이션 인스턴스가 유효하지 않거나 기본 공격 몽타주가 재생 중이 아닌 경우 무시
	if (!AnimInstance || !BasicAttackMontage || !AnimInstance->Montage_IsActive(BasicAttackMontage.Get()))
	{
		return;
	}

	const FName SectionName = AnimInstance->Montage_GetCurrentSection(BasicAttackMontage.Get());

	// 이미 다른 섹션으로 전환된 경우 무시
	if (SectionName != ActiveComboSection)
	{
		return;
	}

	// 현재 섹션의 연계 가능 구간 상태를 Expired로 설정
	ComboWindowState = EComboWindowState::Expired;
}

void AREPlayerCharacter::OnComboSectionChanged(UAnimMontage* Montage, FName SectionName, bool bLooped)
{
	// 현재 재생 중인 몽타주가 기본 공격 몽타주가 아니거나 공격 중이 아닌 경우 무시
	if (Montage != BasicAttackMontage.Get() || !bIsAttacking)
	{
		return;
	}

	// 새로 활성화된 공격 콤보 섹션과 C++ 상태 동기화
	SyncComboSection(SectionName);
}

void AREPlayerCharacter::OnBasicAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 현재 기본 공격 몽타주인지 확인
	if (Montage != BasicAttackMontage.Get())
	{
		return;
	}

	// 공격 및 콤보 상태 초기화
	bIsAttacking = false;
	ResetComboState();

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	// Notify 이벤트 연결 해제
	if (AnimInstance)
	{
		AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic
		(
			this,
			&AREPlayerCharacter::OnComboNotifyBegin
		);

		AnimInstance->OnPlayMontageNotifyEnd.RemoveDynamic
		(
			this,
			&AREPlayerCharacter::OnComboNotifyEnd
		);
	}
}
