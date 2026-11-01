#include "REPlayerController.h"
#include "REPlayerCharacter.h"

#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

void AREPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 현재 Controller에 연결된 로컬 플레이어
	ULocalPlayer* LocalPlayer = GetLocalPlayer();

	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("LocalPlayer is not valid."));
		return;
	}

	// Enhanced Input 입력 매핑 관리 시스템
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);

	if (!Subsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("Enhanced Input subsystem not found."));
		return;
	}

	if (!DefaultMappingContext)
	{
		UE_LOG(LogTemp, Error, TEXT("DefaultMappingContext is not assigned."));
		return;
	}

	// 플레이어 입력 매핑 활성화
	Subsystem->AddMappingContext(DefaultMappingContext.Get(), 0);
}

void AREPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);

	if (!EnhancedInput)
	{
		UE_LOG(LogTemp, Error, TEXT("EnhancedInputComponent not found."));
		return;
	}

	// 이동 입력 이벤트 연결
	if (MoveAction)
	{
		// 이동 시작 이벤트 발생 시 HandleMoveStarted 함수 호출
		EnhancedInput->BindAction
		(
			MoveAction.Get(),
			ETriggerEvent::Started,
			this,
			&AREPlayerController::HandleMoveStarted
		);

		// 이동 지속 이벤트 발생 시 HandleMove 함수 호출
		EnhancedInput->BindAction
		(
			MoveAction.Get(),
			ETriggerEvent::Triggered,
			this,
			&AREPlayerController::HandleMove
		);

		// 이동 완료 이벤트 발생 시 HandleMoveReleased 함수 호출
		EnhancedInput->BindAction
		(
			MoveAction.Get(),
			ETriggerEvent::Completed,
			this,
			&AREPlayerController::HandleMoveReleased
		);

		// 이동 취소 이벤트 발생 시 HandleMoveReleased 함수 호출
		EnhancedInput->BindAction
		(
			MoveAction.Get(),
			ETriggerEvent::Canceled,
			this,
			&AREPlayerController::HandleMoveReleased
		);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveAction is not assigned."));
	}

	// 점프 입력 이벤트 연결
	if (JumpAction)
	{
		// 점프 시작 이벤트 발생 시 HandleJumpStarted 함수 호출
		EnhancedInput->BindAction
		(
			JumpAction.Get(),
			ETriggerEvent::Started,
			this,
			&AREPlayerController::HandleJumpStarted
		);

		// 점프 완료 이벤트 발생 시 HandleJumpStopped 함수 호출
		EnhancedInput->BindAction
		(
			JumpAction.Get(),
			ETriggerEvent::Completed,
			this,
			&AREPlayerController::HandleJumpStopped
		);

		// 점프 취소 이벤트 발생 시 HandleJumpStopped 함수 호출
		EnhancedInput->BindAction
		(
			JumpAction.Get(),
			ETriggerEvent::Canceled,
			this,
			&AREPlayerController::HandleJumpStopped
		);
	}

	else
	{
		UE_LOG(LogTemp, Warning, TEXT("JumpAction is not assigned."));
	}

	// 기본 공격 입력 이벤트 연결
	if (BasicAttackAction)
	{
		// 공격 시작 이벤트 발생 시 HandleBasicAttackStarted 함수 호출
		EnhancedInput->BindAction
		(
			BasicAttackAction.Get(),
			ETriggerEvent::Started,
			this,
			&AREPlayerController::HandleBasicAttackStarted
		);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BasicAttackAction is not assigned."));
	}
}

void AREPlayerController::HandleMoveStarted(const FInputActionValue& Value)
{
	AREPlayerCharacter* PlayerCharacter = Cast<AREPlayerCharacter>(GetPawn());

	if (!PlayerCharacter)
	{
		return;
	}

	const float MoveValue = Value.Get<float>();

	// 이동 방향
	int32 InputDirection = 0;

	// 오른쪽 이동 입력인 경우
	if (MoveValue > 0.1f)
	{
		InputDirection = 1;
	}
	// 왼쪽 이동 입력인 경우
	else if (MoveValue < -0.1f)
	{
		InputDirection = -1;
	}

	// 입력 방향이 없는 경우 이동 해제 처리
	if (InputDirection == 0)
	{
		HandleMoveReleased();
		return;
	}

	const double CurrentTime = GetWorld()->GetTimeSeconds();

	// 연속 입력이 동일 방향으로 들어왔는지 확인
	const bool bDoubleTap =
		HeldMoveDirection == 0 &&
		LastTapDirection == InputDirection &&
		LastTapTime >= 0.0 &&
		(CurrentTime - LastTapTime) <= DoubleTapWindow;

	// 캐릭터의 달리기 상태 설정
	PlayerCharacter->SetRunning(bDoubleTap);

	// 현재 입력 정보 갱신
	HeldMoveDirection = InputDirection;
	LastTapDirection = InputDirection;
	LastTapTime = CurrentTime;
}

void AREPlayerController::HandleMove(const FInputActionValue& Value)
{
	AREPlayerCharacter* PlayerCharacter = Cast<AREPlayerCharacter>(GetPawn());

	if (!PlayerCharacter)
	{
		return;
	}

	const float MoveValue = Value.Get<float>();

	// 이동 방향
	int32 InputDirection = 0;

	// 오른쪽 이동 입력인 경우
	if (MoveValue > 0.1f)
	{
		InputDirection = 1;
	}
	// 왼쪽 이동 입력인 경우
	else if (MoveValue < -0.1f)
	{
		InputDirection = -1;
	}

	// 이동 중 반대 방향으로 전환된 경우 달리기 상태 해제
	if (InputDirection != 0 && InputDirection != HeldMoveDirection && HeldMoveDirection != 0)
	{
		PlayerCharacter->SetRunning(false);

		// 현재 이동 방향 갱신
		HeldMoveDirection = InputDirection;
	}

	PlayerCharacter->MoveHorizontal(MoveValue);
}

void AREPlayerController::HandleMoveReleased()
{
	HeldMoveDirection = 0;

	if (AREPlayerCharacter* PlayerCharacter = Cast<AREPlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->SetRunning(false);
	}
}

void AREPlayerController::HandleJumpStarted()
{
	AREPlayerCharacter* PlayerCharacter = Cast<AREPlayerCharacter>(GetPawn());

	if (PlayerCharacter && !PlayerCharacter->IsAttacking())
	{
		PlayerCharacter->Jump();
	}
}

void AREPlayerController::HandleJumpStopped()
{
	AREPlayerCharacter* PlayerCharacter = Cast<AREPlayerCharacter>(GetPawn());

	if (PlayerCharacter)
	{
		PlayerCharacter->StopJumping();
	}
}

void AREPlayerController::HandleBasicAttackStarted()
{
	AREPlayerCharacter* PlayerCharacter = Cast<AREPlayerCharacter>(GetPawn());

	if (PlayerCharacter)
	{
		PlayerCharacter->StartBasicAttack();
	}
}