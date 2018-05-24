
#pragma once

#include "ml.h"
#include <memory>
#include <vector>
#include "Components/PoseableMeshComponent.h"

namespace ml
{

/**
For using an instance of this class, an instance of UPoseableMeshComponent 
must be set before anything else is done. (see SetUPoseableMeshComponent(UPoseableMeshComponent *u_p) function).
*/
class UE4Poser
{
public:
	UE4Poser();

	//////////////////////////////////////////////////
	// Initialize

	/**
	This an instance of UPoseableMeshComponent must be set before anything else.
	*/
	void SetUPoseableMeshComponent(UPoseableMeshComponent *u_p);

	/** 
	Set a joint tag to the bone of bone_name.
	*/
	void SetJointTag(std::string bone_name, JointTag t);

	/**
	BuildSkeleton must be called after completing SetJointTag.
	*/
	void BuildSkeleton();



	bool HasBone(std::string bone_name) const;
	std::string GetUBoneNameStd(ml::JointTag j_tag) const;

	//////////////////////////////////////////////////
	// Pose
	void Retarget(const ml::Posture &in, bool limb_ik=false, bool head_ik=false);


protected:


	void SetRestUPose();

	double CalculScaleTo(const ml::Body *body) const;
	void UpdateCharacterPose();

	

	//void SetBoneMatrix(int bone_id, cml::matrix44d m);


	/* if not exit, return -1*/
	int GetUBoneIndex(FName u_bone_name) const;
	/* if not exit, return -1*/
	int GetUBoneIndex(std::string bone_name) const;
	/* if not exit, return -1*/
	int GetUBoneIndex(ml::JointTag j_tag) const;

	FName GetUBoneName(int u_bone_index) const;
	FName GetUBoneName(ml::JointTag j_tag) const;
	std::string GetUBoneNameStd(int u_bone_index) const;

	// global transform matrix 
	FMatrix GetUBoneMatrix(int u_bone_index) const;
	FMatrix GetUBoneMatrix(FName u_bone_name) const;
	FMatrix GetUBoneMatrix(ml::JointTag j_tag) const;

	cml::matrix44d GetUBoneMatrixCml(int u_bone_index) const;
	cml::matrix44d GetUBoneMatrixCml(FName u_bone_name) const;
	cml::matrix44d GetUBoneMatrixCml(ml::JointTag j_tag) const;


	ml::JointTag GetJointTag(std::string bone_name) const;
	//const GL_SceneNode* GetBoneNodeByJointTag(ml::JointTag c) const;
	//const GL_SceneNode* GetNearestTaggedParentBoneNode(const GL_SceneNode* c) const;
	ml::JointTag GetNearestTaggedParentTag(ml::JointTag c) const;

protected:
	UPoseableMeshComponent *u_poseable_;
	
	Body skeleton_;
	Posture posture_;


	//std::map<int, FMatrix> bone_id_to_global_offset_map_; // base T-pose
	std::map<ml::JointTag, FMatrix> global_transfs_uTpose_; // base T-pose
	std::map<ml::JointTag, FMatrix> global_transfs_mlTpose_; // base T-pose

	std::map<ml::JointTag, int> joint_tag_to_bone_id_map_;
	std::map<int, ml::JointTag> bone_id_to_joint_tag_map_;

	
};






};





