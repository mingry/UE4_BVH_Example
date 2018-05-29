// Fill out your copyright notice in the Description page of Project Settings.

#include "MyActor2.h"
#include "Misc/Paths.h"				// FPaths::ProjectContentDir() 사용을 위해...
#include "EngineUtils.h"			// Debug 출력을 위해...

// Sets default values
AMyActor2::AMyActor2()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyActor2::BeginPlay()
{
	Super::BeginPlay();



	//////////////////////////////////////////////////////////////////////////////////////////
	//// 1. UE4 캐릭터 모델 준비
	//// 1.1 InitPoseableCharacter() 함수를 호출 
	// BluePrinter를 통해 지정된 캐릭터 모델을 c++ code에서 참조할 수 있도록 준비한다.
	// 구체적으로 말하면 멤버 변수인 'poseable_char_'와 'ml_u_posers_'를 사용 가능한 상태로 준비한다.
	// 함수가 성공적으로 처리된 경우 true가 리턴된다.
	// 반드시 InitPoseableCharacter() 함수의 내용도 살펴보자. 
	flag_valid_poseable_char_ = InitPoseableCharacter();




	//////////////////////////////////////////////////////////////////////////////////////////
	//// 2. 원본 모션 캡쳐 데이터 로드
	//// 2.1 모션 데이터 파일의 위치(폴더)와 이름 지정
	// FPaths::ProjectContentDir() 는 UE4 프로젝트 폴터의 Content 폴더 위치를 리턴한다.
	FString T_pose_filename = FPaths::ProjectContentDir() + "bvh/bsh_T.bvh";	// T-Pose 자세를 저장하고있는 모션 데이터 파일
	FString run_motion_filename = FPaths::ProjectContentDir() + "bvh/ljh_Run001.bvh"; // 로드해야할 모션 데이터 파일
	FString wing_motion_filename = FPaths::ProjectContentDir() + "bvh/kyc_wing.bvh"; // 로드해야할 모션 데이터 파일

	//// 2.2 run_motion 객체에 ljh_Run001.bvh을 로드한다.
	// 반드시 T-Pose 자세를 포함하고 있는 데이터 파일을 첫 번째 파라메터로 넣어야한다.
	// ML libaray에서는 T-Pose를 기준으로 상대적인 인체 자세의 변화를 분석하고 향후 모든 계산에 사용한다.
	ml::Motion run_motion;
	ml::LoadBVH_UE4(T_pose_filename, run_motion_filename, run_motion);

	//// 2.3 wing_motion 객체에 ljh_Run001.bvh을 로드한다.
	// 반드시 T-Pose 자세를 포함하고 있는 데이터 파일을 첫 번째 파라메터로 넣어야한다.
	// ML libaray에서는 T-Pose를 기준으로 상대적인 인체 자세의 변화를 분석하고 향후 모든 계산에 사용한다.
	ml::Motion wing_motion;
	ml::LoadBVH_UE4(T_pose_filename, wing_motion_filename, wing_motion);






	//////////////////////////////////////////////////////////////////////////////////////////
	//// 3. 모션 캡쳐 데이터 편집
	//// 3.1 달리기 부분 잘라내기.
	// 'run_motion'의 전체 내용 중 30번 프레임부터 20개를 잘라서 'run_onecylce_motion' 객체로 복사해 넣는다.
	ml::Motion run_onecylce_motion;
	run_motion.CropMotion(30, 20, &run_onecylce_motion);

	//// 3.2 달리다 서는 부분 잘라내기
	// 'run_motion'의 전체 내용 중 270번 프레임부터 60개를 잘라서 'stop_motion' 객체로 복사해 넣는다.
	ml::Motion stop_motion;
	run_motion.CropMotion(270, 60, &stop_motion);

	//// 3.3 날개짓 춤추기 잘라내기
	// 'wing_motion'의 전체 내용 중 66번 프레임부터 100개를 잘라서 'wing_motion_01' 객체로 복사해 넣는다.
	ml::Motion wing_motion_01;
	wing_motion.CropMotion(66, 100, &wing_motion_01);

	//// 3.4 이어 붙이기.
	// 잘라낸 모션 조각들 'motion_' 객체에 담으며 이어붙인다.
	// 새로운 하나의 모션 데이터를 만든다.
	motion_ = run_onecylce_motion;				// 첫 부분 복사 (달리기)
	motion_.Stitch_UE4(run_onecylce_motion);	// 다음 부분 이어붙이기 (달리기 반복)
	motion_.Stitch_UE4(run_onecylce_motion);	// 다음 부분 이어붙이기 (달리기 반복)
	motion_.Stitch_UE4(run_onecylce_motion);	// 다음 부분 이어붙이기 (달리기 반복)
	motion_.Stitch_UE4(stop_motion);			// 다음 부분 이어붙이기 (서기)
	motion_.Stitch_UE4(wing_motion_01);			// 다음 부분 이어붙이기 (날개 짓)
	motion_.Stitch_UE4(wing_motion_01);			// 다음 부분 이어붙이기 (날개 짓)
	motion_.Stitch_UE4(wing_motion_01);			// 다음 부분 이어붙이기 (날개 짓)

	





	//////////////////////////////////////////////////////////////////////////////////////////
	//// 4. 완성된 모션 데이터 각 인체 부위 이름 지정
	// 각각의 모션 캡쳐 데이터는 파일마다 인체 부위에 대한 이름이 서로 다르게 기록되어있을 것이다.
	// ML library를 사용하기 위해서는 각 인체 부위에 사전에 약속된 이름(JointTag)을 부여하여야 한다.
	// SetJointTag("모션 캡쳐 데이터 상의 이름", "ML library에 정의 된 이름").
	motion_.editable_body()->SetJointTag("Hips", ml::PELVIS);
	motion_.editable_body()->SetJointTag("Spine", ml::SPINE);
	motion_.editable_body()->SetJointTag("Spine1", ml::SPINE1);
	motion_.editable_body()->SetJointTag("Spine2", ml::CHEST);
	motion_.editable_body()->SetJointTag("Neck", ml::NECK);
	motion_.editable_body()->SetJointTag("Head", ml::HEAD);

	motion_.editable_body()->SetJointTag("RightUpLeg", ml::R_HIP);
	motion_.editable_body()->SetJointTag("RightLeg", ml::R_KNEE);
	motion_.editable_body()->SetJointTag("RightFoot", ml::R_ANKLE);
	motion_.editable_body()->SetJointTag("RightToeBase", ml::R_FOOT);

	motion_.editable_body()->SetJointTag("LeftUpLeg", ml::L_HIP);
	motion_.editable_body()->SetJointTag("LeftLeg", ml::L_KNEE);
	motion_.editable_body()->SetJointTag("LeftFoot", ml::L_ANKLE);
	motion_.editable_body()->SetJointTag("LeftToeBase", ml::L_FOOT);

	//motion_.editable_body()->SetJointTag("RightShoulder", ml::R_CLAVICLE);
	motion_.editable_body()->SetJointTag("RightArm", ml::R_SHOULDER);
	motion_.editable_body()->SetJointTag("RightForeArm", ml::R_ELBOW);
	motion_.editable_body()->SetJointTag("RightHand", ml::R_WRIST);

	//motion_.editable_body()->SetJointTag("LeftShoulder", ml::L_CLAVICLE);
	motion_.editable_body()->SetJointTag("LeftArm", ml::L_SHOULDER);
	motion_.editable_body()->SetJointTag("LeftForeArm", ml::L_ELBOW);
	motion_.editable_body()->SetJointTag("LeftHand", ml::L_WRIST);




}

bool AMyActor2::InitPoseableCharacter()
{
	//////////////////////////////////////////////////////////////////////////////////////////
	//// 1. 'tom_poseable_' 설정
	//// 1.1 현재 Actor가 소유하고 있는 모든 UPoseableMeshComponent 객체의 포인터를 받아서,
	////     'components' 배열에 담는다. 
	TArray<UPoseableMeshComponent*> components;
	GetComponents(components);

	//// 1.2 components 배열을 검사하여 이름이 "JamePoseable" 인 것을 찾고,
	////     그 포인터를 맴버 변수 'jame_poseable_' 에 저정한다.
	tom_poseable_ = nullptr;
	for (int i = 0; i<components.Num(); i++)
	{
		if (components[i]->GetName() == "TomPoseable")
		{
			tom_poseable_ = (UPoseableMeshComponent*)components[i];
			// UE_LOG(LogTemp, Warning, TEXT("tom_poseable_ = %s"), *(tom_poseable_->GetName())); // 디버그 용 출력
		}
	}



	// "TomPoseable" 찾기에 실패한 경우 return false.
	if (tom_poseable_ == nullptr) return false;	

	// "TomPoseable" 이 적절한 메시 모델을 소유하고 있지 안은 경우 return false.
	if (tom_poseable_->SkeletalMesh == nullptr) return false;	




	//////////////////////////////////////////////////////////////////////////////////////////
	//// 2. 'ml_u_poser_' 설정
	//// 2.1 'tom_poseable_' 포인터 값을 입력한다.
	ml_u_poser_.SetUPoseableMeshComponent(tom_poseable_);

	//// 2.2 캐릭터 모델 인체 부위 이름 지정
	// 각각의 캐릭터 모델의 신체 부위 이름은 제작자가 임의로 부여하기 때문에 서로 다를 것이다.
	// ML library를 사용하기 위해서는 각 인체 부위에 사전에 약속된 이름(JointTag)을 부여하여야 한다.
	// SetJointTag("캐릭터 모델 상의 신체 이름", "ML library에 정의 된 이름").
	ml_u_poser_.SetJointTag("pelvis", ml::PELVIS);
	ml_u_poser_.SetJointTag("spine_01", ml::SPINE);
	ml_u_poser_.SetJointTag("spine_02", ml::SPINE1);
	ml_u_poser_.SetJointTag("spine_03", ml::CHEST);
	ml_u_poser_.SetJointTag("neck_01", ml::NECK);
	ml_u_poser_.SetJointTag("head", ml::HEAD);

	ml_u_poser_.SetJointTag("thigh_r", ml::R_HIP);
	ml_u_poser_.SetJointTag("calf_r", ml::R_KNEE);
	ml_u_poser_.SetJointTag("foot_r", ml::R_ANKLE);
	ml_u_poser_.SetJointTag("ball_r", ml::R_FOOT);

	ml_u_poser_.SetJointTag("thigh_l", ml::L_HIP);
	ml_u_poser_.SetJointTag("calf_l", ml::L_KNEE);
	ml_u_poser_.SetJointTag("foot_l", ml::L_ANKLE);
	ml_u_poser_.SetJointTag("ball_l", ml::L_FOOT);


	ml_u_poser_.SetJointTag("clavicle_r", ml::R_CLAVICLE);
	ml_u_poser_.SetJointTag("upperarm_r", ml::R_SHOULDER);
	ml_u_poser_.SetJointTag("lowerarm_r", ml::R_ELBOW);
	ml_u_poser_.SetJointTag("hand_r", ml::R_WRIST);

	ml_u_poser_.SetJointTag("clavicle_l", ml::L_CLAVICLE);
	ml_u_poser_.SetJointTag("upperarm_l", ml::L_SHOULDER);
	ml_u_poser_.SetJointTag("lowerarm_l", ml::L_ELBOW);
	ml_u_poser_.SetJointTag("hand_l", ml::L_WRIST);


	//// 2.3 위에 설정된 내용을 바탕으로 'ml_u_poser' 내부 데이터를 업데이트한다.
	// 'BuildSkeleton()' 함수는 반드시 2.1과 2.2의 설정이 끝난후 실행되어야한다.
	ml_u_poser_.BuildSkeleton();


	return true;

}

// Called every frame
void AMyActor2::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// "JamesPoseable"이 현재 nullptr 라면 여기서 종료.
	if (tom_poseable_ == nullptr) return;

	// InitPoseableCharacter() 함수 처리가 실패되었다면 여기서 종료.
	if (flag_valid_poseable_char_ == false) return;

	// 모션 캡쳐 데이터에 내용이 없으면 (frame수가 0이라면) 여기서 종료.
	if (motion_.size()==0) return;


	// 시간 받아오기.
	float t = GetGameTimeSinceCreation();


	// 시간 t에 해당하는 모션 데이터를 불러와서 'pose' 객체에 담는다.
	ml::Posture pose = motion_.GetPostureAtTime_ms(t);
	ml_u_poser_.Retarget( pose );

}

