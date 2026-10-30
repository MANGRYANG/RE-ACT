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
		// 이동 이벤트 발생 시 HandleMove 함수 호출
        EnhancedInput->BindAction
        (
            MoveAction.Get(),
            ETriggerEvent::Triggered,
            this,
            &AREPlayerController::HandleMove
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
}

void AREPlayerController::HandleMove(const FInputActionValue& Value)
{
    AREPlayerCharacter* PlayerCharacter = Cast<AREPlayerCharacter>(GetPawn());

    if (!PlayerCharacter)
    {
        return;
    }

    PlayerCharacter->MoveHorizontal(Value.Get<float>());
}

void AREPlayerController::HandleJumpStarted()
{
    AREPlayerCharacter* PlayerCharacter = Cast<AREPlayerCharacter>(GetPawn());

    if (PlayerCharacter)
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
