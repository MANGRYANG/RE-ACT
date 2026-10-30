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

private:
    void HandleMove(const FInputActionValue& Value);
    void HandleJumpStarted();
    void HandleJumpStopped();
};
