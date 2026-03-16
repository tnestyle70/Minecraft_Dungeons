#pragma once
#include "CGameObject.h"
#include "CCubeBodyTex.h"

//드래곤 관절 구조체
struct DRAGON_BONE
{
	_vec3 vPos; //이 뼈의 월드 위치
	_vec3 vDir; //다음 뼈를 향하는 방향 벡터(정규화)
	float fBoneLen; //이 뼈에서 다음 뼈까지의 고정 길이
	_matrix matWorld;
	CCubeBodyTex* pBuffer; //이 뼈에 해당하는 렌더 버퍼
};

//드래곤 파트별 뼈 개수 상수
constexpr int DRAGON_SPINE_COUNT = 7; //몸통 척추
constexpr int DRAGON_NECK_COUNT = 3; //목
constexpr int DRAGON_TAIL_COUNT = 6; //꼬리
constexpr int DRAGON_WING_COUNT = 4; //날개 한 쪽 세그먼트

//Dragon - Update Flow
//Spine -> moveTarget 방향으로 속도 lerp 만큼 이동
//Spine -> Follow the Leader(척추 추종)
//Neck Head -> CCD IK(목/머리가 이동 방향을 바라봄)
//Tail -> Follow the Leader(꼬리 추종)
//WingL/R sin 파형 날개짓
//Compute BoneMatrix -> SetTransform -> Render
class CDragon : public CGameObject
{
public:
	CDragon(LPDIRECT3DDEVICE9 pGraphicDev);
	CDragon(const CDragon& rhs);
	virtual ~CDragon();
public:
	virtual HRESULT Ready_GameObject() override;
	virtual _int Update_GameObject(const _float& fTimeDelta)override;
	virtual void LateUpdate_GameObject(const _float& fTimeDelta)override;
	virtual void Render_GameObject();
public:
	//드래곤 제어 Application Programming Interface
	void Set_MoveTarget(const _vec3& vTarget) { m_vMoveTarget = vTarget; }
	_vec3 Get_HeadPos() const { return m_Head.vPos; }
	_vec3 Get_SpineRoot() const { return m_Spine[0].vPos; }
private:
	//초기화 헬퍼
	HRESULT Init_SpineChain();
	HRESULT Init_NeckAndHead();
	HRESULT Init_TailChain();
	HRESULT Init_WingChains();
	//fW / H / D : 큐브 크기
	HRESULT Create_BoneBuffer(DRAGON_BONE& bone,
		_float fW, _float fH, _float fD, const FACE_UV& uv);
private:
	//IK - 추종 알고리즘
	//CCD IK - Cyclic Coordinate Descent
	void Solve_CCD(DRAGON_BONE* pChain, _int iCount,
		const _vec3& vTarget, _int iMaxIter = 10);
	//Follow the Leader
	void Solve_FollowLeader(DRAGON_BONE* pChain, _int iCount);
	//날개짓
	void Update_WingFlap(const _float& fTimeDelta);
	//뼈 월드 행렬 - Look-At 방식, DX9 Row-Major
	//vZ = Normalize(bone.vDir)
	//vX = normalize(worldUp * z)
	//vY = vZ * vX
	void Compute_BoneMatrix(DRAGON_BONE& bone);
	void Update_ChainMatrices(DRAGON_BONE* pChain, _int iCount);
	void Render_Chain(DRAGON_BONE* pChain, _int iCount);
	
private:
	//뼈 체인
	DRAGON_BONE m_Spine[DRAGON_SPINE_COUNT];
	DRAGON_BONE m_Neck[DRAGON_NECK_COUNT];
	DRAGON_BONE m_Head;
	DRAGON_BONE m_Tail[DRAGON_TAIL_COUNT];
	DRAGON_BONE m_WingL[DRAGON_WING_COUNT];
	DRAGON_BONE m_WingR[DRAGON_WING_COUNT];
	//이동
	_vec3 m_vMoveTarget; //목표 위치 
	_vec3 m_vVelocity; //루트 속도(Lerp 가속)
	_float m_fMoveSpeed; //최대 이동 속도
	//날개짓
	_float m_fWingTimer; //누적 시간
	_float m_fWingSpeed; //날개짓 주기(rad/sec)
	_float m_fWingAmp; //날개짓 최대 각도(rad)
public:
	static CDragon* Create(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual void Free();
};