

#include "ml_ue4_poser.h"
#include <algorithm>
#include <stack>
#include <string>
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include <Runtime/Engine/Classes/Engine/SkeletalMesh.h>
#include <Runtime/Engine/Public/DrawDebugHelpers.h>

namespace ml
{
	static FVector ToFVector(cml::vector3d v)
	{
		FVector fv;
		for (int i = 0; i < 3; i++)
			fv[i] = v[i];
	

		return fv;
	}

	static FMatrix ToFMatrix(cml::matrix44d m)
	{
		FMatrix fm;
		for (int i = 0; i<4; i++)
			for (int j = 0; j < 4; j++)
			{
				fm.M[i][j] = m(j, i);
				//fm.M[i][j] = m(i, j);
			}

		return fm;
	}

	static FMatrix ToFMatrix(cml::matrix3 m)
	{
		FMatrix fm;
		fm = FMatrix::Identity;

		for (int i = 0; i<3; i++)
			for (int j = 0; j < 3; j++)
			{
				fm.M[i][j] = m(j, i);
				//fm.M[i][j] = m(i, j);
			}

		return fm;
	}

	static FQuat ToFQuat(cml::matrix3 m)
	{

		FQuat fq(ToFMatrix(m));

		return fq;
	}

	static cml::matrix44d ToCmlMatrix(FMatrix m)
	{
		cml::matrix44d cm;
		for (int i = 0; i<4; i++)
			for (int j = 0; j < 4; j++)
			{
				cm(i, j) = m.M[j][i];
				//cm(i, j) = m.M[i][j];
			}

		return cm;
	}

	static cml::matrix33d ToCmlMatrix3(FMatrix m)
	{
		cml::matrix33d cm;
		for (int i = 0; i<3; i++)
			for (int j = 0; j < 3; j++)
			{
				cm(i, j) = m.M[j][i];
				//cm(i, j) = m.M[i][j];
			}

		return cm;
	}

	static FRotator ToFRotator(cml::matrix3 m)
	{
		FRotator fr( ToFQuat(m) );
		return fr;
	}

	static FName ToFName(std::string name)
	{
		FString f_str(name.c_str());
		FName f_name = *f_str;

		return f_name;
	}

	static std::string ToStdString(FName name)
	{
		return TCHAR_TO_ANSI(*name.GetPlainNameString());
	}

UE4Poser::UE4Poser()
{
	u_poseable_ = nullptr;
}

void
UE4Poser::SetUPoseableMeshComponent(UPoseableMeshComponent *u_p)
{
	u_poseable_ = u_p;
	

}

void 
UE4Poser::SetJointTag(std::string bone_name, JointTag t)
{
	int b_id = GetUBoneIndex(bone_name);

	if ( b_id < 0 ) 
	{
		//std::cerr << bone_name << ", bone havs not been found!!!!!!!!!!!! [SetJointTag()]" << std::endl;
		UE_LOG(LogTemp, Warning, TEXT("ERROR!! - %s, bone has not been found!!!!!!!!!!!! [SetJointTag()]"), *FString(bone_name.c_str()));
	}

	
	joint_tag_to_bone_id_map_[t] = b_id;
	bone_id_to_joint_tag_map_[ b_id ]= t;
}

ml::JointTag 
UE4Poser::GetJointTag(std::string bone_name) const
{
	int b_id = GetUBoneIndex(bone_name);
	
	if (b_id < 0) return ml::UNKNOWN;
	if (bone_id_to_joint_tag_map_.count(b_id) == 0) return ml::UNKNOWN;

	return bone_id_to_joint_tag_map_.at(b_id);
}

bool UE4Poser::HasBone(std::string bone_name) const
{
	if (!u_poseable_) return false;

	int b_id = GetUBoneIndex(bone_name);
	if (b_id < 0) return false;
	
	return true;
}

void 
UE4Poser::SetRestUPose()
{
	TArray<FName> b_names;
	u_poseable_->GetBoneNames(b_names);
	for (int i = 0; i < b_names.Num(); i++)
	{
		int b_id = GetUBoneIndex(b_names[i]);

		u_poseable_->BoneSpaceTransforms[b_id].SetFromMatrix(u_poseable_->SkeletalMesh->GetRefPoseMatrix(b_id));
	}
}


void 
UE4Poser::BuildSkeleton()
{
	skeleton_.Clear();

	SetRestUPose();
	
	// Pelvis (root)
	// root joint must be at the first place of skeleton_.m_joints
	{
		FMatrix root_base_m = GetUBoneMatrix(ml::PELVIS);
		

		int root_joint_id = 0;
		ml::Joint root_joint;
		root_joint.offset.set(root_base_m.M[3][0], root_base_m.M[3][1], root_base_m.M[3][2]);
		root_joint.parent = -1;
		skeleton_.editable_joints().push_back(root_joint);
		skeleton_.editable_joint_name_map()[ GetUBoneNameStd(ml::PELVIS) ] = root_joint_id;
		skeleton_.editable_joint_tag_map()[ml::PELVIS] = root_joint_id;
	}

	// Set Joints (index, tag, name)
	for ( auto tag_boneId : joint_tag_to_bone_id_map_ )
	{
		if ( tag_boneId.first == ml::PELVIS  ) continue;

		int joint_id = (int)skeleton_.editable_joints().size();

		skeleton_.editable_joint_name_map()[ GetUBoneNameStd(tag_boneId.first) ] = joint_id;
		skeleton_.editable_joint_tag_map()[ tag_boneId.first ] = joint_id;
		skeleton_.editable_joints().push_back(ml::Joint());
	}

	// Set Parents and Offsets
	for ( int j=1; j<(int)skeleton_.num_joint(); j++ )
	{
		ml::Joint &cur_joint = skeleton_.editable_joints()[j];

		ml::JointTag cur_tag = skeleton_.joint_tag(j);
		ml::JointTag parent_tag = GetNearestTaggedParentTag(cur_tag);

		cur_joint.parent = skeleton_.joint_index(parent_tag);

		cml::matrix44d cur_offset = GetUBoneMatrixCml(cur_tag);
		cml::matrix44d parent_offset = GetUBoneMatrixCml(parent_tag);

		cur_joint.offset = cml::trans(cur_offset) - cml::trans(parent_offset);

	}

	
	posture_.body(&skeleton_);
	posture_.SetIdentityPose();		


	// For Debug
	if ( false )
	for (int i = 0; i < u_poseable_->GetNumBones(); i++)
	{
		FName b_name = u_poseable_->GetBoneName(i);
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red,
			//FString::Printf(TEXT("%s: %s"), *b_name.GetPlainNameString(), *u_poseable_->BoneSpaceTransforms[i].ToString())
			FString::Printf(TEXT("%s: %s"), *b_name.GetPlainNameString(), *u_poseable_->GetBoneTransform(i).ToString())
			//FString::Printf(TEXT("%s: %s"), *b_name.GetPlainNameString(), *FTransform(u_poseable_->SkeletalMesh->GetRefPoseMatrix(i)).ToString())
			//FString::Printf(TEXT("%s: %s"), *b_name.GetPlainNameString(), *u_poseable_->SkeletalMesh->GetRefPoseMatrix(i).ToQuat().ToString())
			//FString::Printf(TEXT("%s: %s"), *b_name.GetPlainNameString(), *u_poseable_->SkeletalMesh->RefSkeleton.GetRefBonePose()[i].ToString())
		);
		if (i > 10) break;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////
	// global_transfs_uTpose_to_mlTpose

	//std::map<ml::JointTag, FMatrix> global_transf_uTpose;
	{
		for (auto &d : joint_tag_to_bone_id_map_)
		{
			JointTag j_tag = d.first;
			int b_id = d.second;

			FMatrix t = u_poseable_->SkeletalMesh->GetRefPoseMatrix(b_id);
			FName parent_name = u_poseable_->GetParentBone(GetUBoneName(b_id));
			while (parent_name != NAME_None)
			{
				t = t * u_poseable_->SkeletalMesh->GetRefPoseMatrix(GetUBoneIndex(parent_name));

				parent_name = u_poseable_->GetParentBone(parent_name);
			}

			global_transfs_uTpose_[j_tag] = t;
		}

		ml::Posture t_pose = posture_;
		t_pose.SetIdentityPose(); // it must be a T-pose

		for (auto &d : joint_tag_to_bone_id_map_)
		{
			JointTag j_tag = d.first;

			FMatrix t = ToFMatrix( t_pose.GetGlobalTransf(j_tag) );

			global_transfs_mlTpose_[j_tag] = t;
		}
	}

	

}


double UE4Poser::CalculScaleTo(const ml::Body * body) const
{
	// Scale estimation
	double scale = 1.0;

	

	scale = ( cml::length(body->joint(ml::R_KNEE).offset) + cml::length(body->joint(ml::R_ANKLE).offset) )
			/ 
			( cml::length( skeleton_.joint(ml::R_KNEE).offset )
			+ cml::length( skeleton_.joint(ml::R_ANKLE).offset) );
	
	return scale;
}

void 
UE4Poser::Retarget(const ml::Posture & in, bool limb_ik, bool head_ik)
{
	// Scale estimation
	double scale = 1.0;
	if ( true )
	{
		scale = ( cml::length(in.body()->joint(ml::R_KNEE).offset) 
					+ cml::length(in.body()->joint(ml::R_ANKLE).offset) )
				/ 
				( cml::length( skeleton_.joint(ml::R_KNEE).offset )
					+ cml::length(skeleton_.joint(ml::R_ANKLE).offset) );
	}

	posture_.SetGlobalTrans(in.GetGlobalTranslation(ml::PELVIS) / scale);
	for ( unsigned int i=0; i<in.num_joint(); i++ )
	{
		ml::JointTag j_tag = in.body()->joint_tag(i);

		if ( j_tag != ml::UNKNOWN && skeleton_.HasTag( j_tag ) )
		{
			posture_.rotate(j_tag, in.rotate(j_tag));
		}
	}


	if ( head_ik 
		&& posture_.body()->HasTag(ml::HEAD) 
		&& posture_.body()->HasTag(ml::CHEST) 
		&& posture_.body()->HasTag(ml::SPINE) 
		) {

		posture_.IkLimb(ml::HEAD, ml::CHEST, ml::SPINE, in.GetGlobalTranslation(ml::HEAD)/scale);
		posture_.SetGlobalRotation(ml::HEAD, in.GetGlobalRoation(ml::HEAD));
	}

	if ( limb_ik ) {
		posture_.IkLimb(ml::L_WRIST, in.GetGlobalTranslation(ml::L_WRIST)/scale);
		posture_.IkLimb(ml::R_WRIST, in.GetGlobalTranslation(ml::R_WRIST)/scale);
		posture_.IkLimb(ml::L_ANKLE, in.GetGlobalTranslation(ml::L_ANKLE)/scale);
		posture_.IkLimb(ml::R_ANKLE, in.GetGlobalTranslation(ml::R_ANKLE)/scale);

		posture_.SetGlobalRotation(ml::L_WRIST, in.GetGlobalRoation(ml::L_WRIST));
		posture_.SetGlobalRotation(ml::R_WRIST, in.GetGlobalRoation(ml::R_WRIST));
		posture_.SetGlobalRotation(ml::L_ANKLE, in.GetGlobalRoation(ml::L_ANKLE));
		posture_.SetGlobalRotation(ml::R_ANKLE, in.GetGlobalRoation(ml::R_ANKLE));
	}

	UpdateCharacterPose();
}

void
UE4Poser::UpdateCharacterPose()
{
	if (!u_poseable_ || u_poseable_->GetNumBones() == 0)
		return;

	// This function copies the global transformations of all bones of 'posture_' to the 'u_poseable_'.
	// It assumes that the reference poses of 'posture_' and 'u_poseable_' are same as T-Pose.

	for ( auto &d : joint_tag_to_bone_id_map_ )
	{
		ml::JointTag j_tag = d.first;
		int b_id = d.second;
		FName u_bone_name = GetUBoneName(b_id);

		FMatrix m = global_transfs_uTpose_[j_tag] * global_transfs_mlTpose_[j_tag].Inverse() * ToFMatrix(posture_.GetGlobalTransf(j_tag));
		u_poseable_->SetBoneTransformByName(u_bone_name, FTransform(m), EBoneSpaces::ComponentSpace);
	}

	// https://wiki.unrealengine.com/Draw_3D_Debug_Points,_Lines,_and_Spheres:_Visualize_Your_Algorithm_in_Action
	FTransform component_m = u_poseable_->GetComponentTransform();

	// For Debug
	if ( false )
	for (unsigned int j = 0; j < skeleton_.num_joint(); j++)
	{
		DrawDebugPoint(u_poseable_->GetWorld(),
			component_m.TransformFVector4( ToFVector(posture_.GetGlobalTranslation(j)) ),
			//FVector(100, 100, 100),
			10,
			FColor(255, 0, 0));


		if (j > 0)
		{
			DrawDebugLine(u_poseable_->GetWorld(),
				component_m.TransformFVector4( ToFVector(posture_.GetGlobalTranslation(j)) ),
					component_m.TransformFVector4( ToFVector(posture_.GetGlobalTranslation( skeleton_.parent(j) )) ),
				FColor(0, 0, 0)
				);
		}
	}
	

}

ml::JointTag 
UE4Poser::GetNearestTaggedParentTag(ml::JointTag c) const
{
	ml::JointTag res = ml::UNKNOWN;

	if (joint_tag_to_bone_id_map_.count(c) == 0) return res;

	int cur_bone_id = joint_tag_to_bone_id_map_.at(c);
	while (cur_bone_id >= 0)
	{
		FName parent_name = u_poseable_->GetParentBone(u_poseable_->GetBoneName(cur_bone_id));
		if (parent_name.GetPlainNameString() == "None")
		{
			break;
		}

		int parent_bone_id = u_poseable_->GetBoneIndex(parent_name);
		
		if (bone_id_to_joint_tag_map_.count(parent_bone_id) > 0)
		{
			res = bone_id_to_joint_tag_map_.at(parent_bone_id);
			break;
		}

		cur_bone_id = parent_bone_id;
	}

	return res;
}



int UE4Poser::GetUBoneIndex(FName u_bone_name) const
{
	if (!u_poseable_) return -1;

	int b_id = u_poseable_->GetBoneIndex(u_bone_name);
	if (b_id == INDEX_NONE) return -1;
	return b_id;
}

int UE4Poser::GetUBoneIndex(std::string bone_name) const
{
	return GetUBoneIndex(ToFName(bone_name));
}

int UE4Poser::GetUBoneIndex(ml::JointTag j_tag) const
{
	if (!u_poseable_) return -1;
	if (joint_tag_to_bone_id_map_.count(j_tag) == 0) return -1;

	return joint_tag_to_bone_id_map_.at(j_tag);
}




FName UE4Poser::GetUBoneName(int u_bone_index) const
{
	if (!u_poseable_) return NAME_None;
	return u_poseable_->GetBoneName(u_bone_index);
}

FName UE4Poser::GetUBoneName(ml::JointTag j_tag) const
{
	if (!u_poseable_) return NAME_None;
	if (joint_tag_to_bone_id_map_.count(j_tag) == 0) return NAME_None;
	return GetUBoneName(joint_tag_to_bone_id_map_.at(j_tag));
}


std::string UE4Poser::GetUBoneNameStd(int u_bone_index) const
{
	FName n = GetUBoneName(u_bone_index);
	if (n == NAME_None) return "";
	return TCHAR_TO_ANSI(*n.GetPlainNameString());
}

std::string UE4Poser::GetUBoneNameStd(ml::JointTag j_tag) const
{
	FName n = GetUBoneName(j_tag);
	if (n == NAME_None) return "";
	return TCHAR_TO_ANSI(*n.GetPlainNameString());
}



FMatrix UE4Poser::GetUBoneMatrix(int u_bone_index) const
{
	if (!u_poseable_) return FMatrix();

	FMatrix component_m = u_poseable_->GetComponentTransform().ToMatrixWithScale();

	return u_poseable_->GetBoneMatrix(u_bone_index)*component_m.Inverse();
}

FMatrix UE4Poser::GetUBoneMatrix(FName u_bone_name) const
{
	return GetUBoneMatrix(GetUBoneIndex(u_bone_name));
}

FMatrix UE4Poser::GetUBoneMatrix(ml::JointTag j_tag) const
{
	if (joint_tag_to_bone_id_map_.count(j_tag) == 0) return FMatrix();

	return GetUBoneMatrix(joint_tag_to_bone_id_map_.at(j_tag));
}

cml::matrix44d UE4Poser::GetUBoneMatrixCml(int u_bone_index) const
{
	return ToCmlMatrix(GetUBoneMatrix(u_bone_index));
}

cml::matrix44d UE4Poser::GetUBoneMatrixCml(FName u_bone_name) const
{
	return ToCmlMatrix(GetUBoneMatrix(u_bone_name));
}

cml::matrix44d UE4Poser::GetUBoneMatrixCml(ml::JointTag j_tag) const
{
	return ToCmlMatrix(GetUBoneMatrix(j_tag));
}

};






