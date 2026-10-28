// Fill out your copyright notice in the Description page of Project Settings.


#include "REGameMode.h"

#include "REPlayerController.h"
#include "REPlayerCharacter.h"

AREGameMode::AREGameMode()
{
	// 기본 Pawn 클래스 설정
	DefaultPawnClass = AREPlayerCharacter::StaticClass();
	
	// 기본 PlayerController 클래스 설정
	PlayerControllerClass = AREPlayerController::StaticClass();
}
