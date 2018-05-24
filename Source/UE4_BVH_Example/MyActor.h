// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PoseableMeshComponent.h"
#include "Motion/ml.h"
#include "MyActor.generated.h"

UCLASS()
class UE4_BVH_EXAMPLE_API AMyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool InitPoseableCharacter();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool flag_valid_poseable_char_;
	UPoseableMeshComponent *poseable_char_;
	ml::UE4Poser ml_u_poser;

	ml::Motion *motion_;
};
