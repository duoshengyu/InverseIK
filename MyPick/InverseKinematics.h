//------------------------------------------------------------------------------
//Simple IK implement
//------------------------------------------------------------------------------
#ifndef _IK_
#define _IK_

#include <d3dx9.h>

struct Bone;
class Character;

class InverseKinematics
{
public:
	InverseKinematics(Character* pSkinnedMesh);
	void UpdateHeadIK();
	void UpdateArmIK();
	void UpdateIK();
	void ApplyLookAtIK(D3DXVECTOR3 &lookAtTarget, float maxAngle);
	void ApplyArmIK(D3DXVECTOR3 &hingeAxis, D3DXVECTOR3 &target);
	void ApplyIK(Bone *Start, Bone *Joint, Bone *End, D3DXVECTOR3 &hingeAxis, D3DXVECTOR3 &target);
	void getAllName(Bone *bone);
	void UpdateCurMat(Bone* parent, Bone *bone);
	void addLHMove(D3DXVECTOR3 &move);
	void addRHMove(D3DXVECTOR3 &move);
	void addLLMove(D3DXVECTOR3 &move);
	void addRLMove(D3DXVECTOR3 &move);
private:
	Character *m_pCharacter;
	D3DXVECTOR3 m_headForward;
	Bone* m_pHeadBone;
	Bone* U_R_Arm;
	Bone* U_L_Arm;
	Bone* L_R_Arm;
	Bone* L_L_Arm;
	Bone* R_Hand;
	Bone* L_Hand;
	Bone* R_Thigh;
	Bone* L_Thigh;
	Bone* R_Calf;
	Bone* L_Calf;
	Bone* R_Foot;
	Bone* L_Foot;

	D3DXVECTOR3 LHandTarget;
	D3DXVECTOR3 RHandTarget;
	D3DXVECTOR3 LLegTarget;
	D3DXVECTOR3 RLegTarget;
};

#endif