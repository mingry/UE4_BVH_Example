// Fill out your copyright notice in the Description page of Project Settings.

#include "MyActor.h"
#include "Misc/Paths.h"
#include "EngineUtils.h"
#include "UObjectIterator.h"


// Sets default values
AMyActor::AMyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyActor::BeginPlay()
{
	Super::BeginPlay();

	flag_valid_poseable_char_ = false;
	flag_valid_poseable_char_ = InitPoseableCharacter();


	if (flag_valid_poseable_char_)
	{
		motion_ = new ml::Motion;

		FString motion_filename = FPaths::ProjectContentDir() + "bvh/wd2_yoga_total_2_x2d.bvh";
		
		ml::LoadBVH_UE4(motion_filename, motion_);

		motion_->editable_body()->SetPredefinedTagMap();

		UE_LOG(LogTemp, Warning, TEXT("motion_filename = %s"), *(motion_filename));
		UE_LOG(LogTemp, Warning, TEXT("motion_size = %d"), motion_->size());
	}
}

bool AMyActor::InitPoseableCharacter()
{
	// Find "MyPoseable" in all components array.
	TArray<UActorComponent*> components;
	GetComponents(components);

	poseable_char_ = nullptr;
	for (int i = 0; i<components.Num(); i++)
	{
		if (components[i]->GetName() == "MyPoseable")
		{
			poseable_char_ = (UPoseableMeshComponent*)components[i];
			UE_LOG(LogTemp, Warning, TEXT("poseable_char = %s"), *(poseable_char_->GetName()));
		}
	}

	if (poseable_char_ == nullptr) return false;
	if (poseable_char_->SkeletalMesh == nullptr) return false;


	ml_u_poser.SetUPoseableMeshComponent(poseable_char_);

	// Set JointTags
	ml_u_poser.SetJointTag("pelvis", ml::PELVIS);
	ml_u_poser.SetJointTag("spine_01", ml::SPINE);
	ml_u_poser.SetJointTag("spine_02", ml::SPINE1);
	ml_u_poser.SetJointTag("spine_03", ml::CHEST);
	ml_u_poser.SetJointTag("neck_01", ml::NECK);
	ml_u_poser.SetJointTag("head", ml::HEAD);

	ml_u_poser.SetJointTag("thigh_r", ml::R_HIP);
	ml_u_poser.SetJointTag("calf_r", ml::R_KNEE);
	ml_u_poser.SetJointTag("foot_r", ml::R_ANKLE);
	ml_u_poser.SetJointTag("ball_r", ml::R_FOOT);

	ml_u_poser.SetJointTag("thigh_l", ml::L_HIP);
	ml_u_poser.SetJointTag("calf_l", ml::L_KNEE);
	ml_u_poser.SetJointTag("foot_l", ml::L_ANKLE);
	ml_u_poser.SetJointTag("ball_l", ml::L_FOOT);
	

	ml_u_poser.SetJointTag("clavicle_r", ml::R_CLAVICLE);
	ml_u_poser.SetJointTag("upperarm_r", ml::R_SHOULDER);
	ml_u_poser.SetJointTag("lowerarm_r", ml::R_ELBOW);
	ml_u_poser.SetJointTag("hand_r", ml::R_WRIST);

	ml_u_poser.SetJointTag("clavicle_l", ml::L_CLAVICLE);
	ml_u_poser.SetJointTag("upperarm_l", ml::L_SHOULDER);
	ml_u_poser.SetJointTag("lowerarm_l", ml::L_ELBOW);
	ml_u_poser.SetJointTag("hand_l", ml::L_WRIST);


	ml_u_poser.BuildSkeleton();
	

	return true;

}

// Called every frame
void AMyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (poseable_char_ == nullptr) return;
	if (flag_valid_poseable_char_ == false) return;

	/*TArray<FMatrix> mmm;
	mmm = m_com->RefBasesInvMatrix;*/

	float t = GetGameTimeSinceCreation();


	FTransform T;
	T.SetRotation(FQuat(FRotator(t, 10, 20)));

	int32 NumBones = poseable_char_->GetNumBones();

	if (false)
		for (int32 i = 0; i < NumBones; ++i)
		{
			FName const BoneName = poseable_char_->GetBoneName(i);
			//UE_LOG(LogTemp, Warning, TEXT("bone name = %s"), *(BoneName.GetPlainNameString()));
			//poseable_char_->BoneSpaceTransforms[i].SetRotation(FQuat(FRotator(t*10, t*10, t*20)));
			float cc = pow(cos(t / 10.), 4);
			poseable_char_->BoneSpaceTransforms[i].SetRotation(FQuat(FRotator(360 * cc, 360 * cc, 360 * cc)));
		}

	if (motion_->size()>0)
		ml_u_poser.Retarget(motion_->GetPostureAtTime_ms(t));

}

