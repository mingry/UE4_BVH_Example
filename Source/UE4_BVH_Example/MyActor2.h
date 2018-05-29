// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PoseableMeshComponent.h"	// UPoseableMeshComponent 클래스 사용을 위해..
#include "Motion/ml.h"							// 모션데이터 처리 라이브러리 (ML library) 사용을 위해..
#include "MyActor2.generated.h"

UCLASS()
class UE4_BVH_EXAMPLE_API AMyActor2 : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyActor2();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/**
	캐릭터 모델의 자세 계산에 필요한 정보를 준비하는 함수.
	함수가 성공적으로 처리되는 경우 poseable_char_(포인터 변수)와 ml_u_posers_(객체 변수)가 사용 가능한 상태로 준비된다.
	*/
	bool InitPoseableCharacter();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPoseableMeshComponent *tom_poseable_;	// 캐릭터 모델 객체에 대한 포인터
	bool flag_valid_poseable_char_;				// 캐릭터 모델이 정상적으로 로드되었는지 확인하는 flag 변수

	ml::Motion motion_;							// 모션 캡쳐 데이터 

	ml::UE4Poser ml_u_poser_;					// 모션 캡쳐 데이터를 UE4 캐릭터 모델에 적용하는 절차를 처리하는 객체
	
};
