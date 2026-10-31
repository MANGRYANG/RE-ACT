#include "REPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
AREPlayerCharacter::AREPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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