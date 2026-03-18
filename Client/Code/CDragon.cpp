#include "pch.h"
#include "CDragon.h"
#include "CRenderer.h"
#include "CManagement.h"
#include "CDInputMgr.h"

static const _vec3 s_PatrolPoints[] =
{
	_vec3(10.f, 10.f, 10.f),
	_vec3(-10.f, 12.f, 10.f),
	_vec3(-10.f, 10.f, -10.f),
	_vec3(10.f, 12.f, -10.f)
};

CDragon::CDragon(LPDIRECT3DDEVICE9 pGraphicDev)
	:CGameObject(pGraphicDev)
	, m_vMoveTarget(0.f, 8.f, 10.f)
	, m_vVelocity(0.f, 0.f, 0.f)
	, m_fMoveSpeed(5.f)
	, m_fWingTimer(0.f)
	, m_fWingSpeed(3.5f)
	, m_fWingAmp(D3DX_PI * 0.35f)
	, m_eState(eDragonState::IDLE)
	, m_fStateTimer(0.f)
	, m_iPatrolIndex(0)
	, m_fTailSwingTimer(0.f)
	, m_fTailSwingAmp(D3DX_PI * 0.8f)
{
	ZeroMemory(m_Spine, sizeof(m_Spine));
	ZeroMemory(m_Neck, sizeof(m_Neck));
	ZeroMemory(&m_Head, sizeof(m_Head));
	ZeroMemory(m_Tail, sizeof(m_Tail));
	ZeroMemory(m_WingL, sizeof(m_WingL));
	ZeroMemory(m_WingR, sizeof(m_WingR));
}

CDragon::CDragon(const CDragon & rhs)
	:CGameObject(rhs)
	, m_vMoveTarget(rhs.m_vMoveTarget)
	, m_vVelocity(rhs.m_vVelocity)
	, m_fMoveSpeed(rhs.m_fMoveSpeed)
	, m_fWingTimer(rhs.m_fWingTimer)
	, m_fWingSpeed(rhs.m_fWingSpeed)
	, m_fWingAmp(rhs.m_fWingAmp)
	, m_eState(rhs.m_eState)
	, m_fStateTimer(rhs.m_fStateTimer)
	, m_iPatrolIndex(rhs.m_iPatrolIndex)
	, m_fTailSwingTimer(rhs.m_fTailSwingTimer)
	, m_fTailSwingAmp(rhs.m_fTailSwingAmp)
{
	ZeroMemory(m_Spine, sizeof(m_Spine));
	ZeroMemory(m_Neck, sizeof(m_Neck));
	ZeroMemory(&m_Head, sizeof(m_Head));
	ZeroMemory(m_Tail, sizeof(m_Tail));
	ZeroMemory(m_WingL, sizeof(m_WingL));
	ZeroMemory(m_WingR, sizeof(m_WingR));
}

CDragon::~CDragon()
{
}

HRESULT CDragon::Ready_GameObject()
{
	if (FAILED(Init_SpineChain()))
		return E_FAIL;
	if (FAILED(Init_NeckAndHead()))
		return E_FAIL;
	if (FAILED(Init_TailChain()))
		return E_FAIL;
	if (FAILED(Init_WingChains()))
		return E_FAIL;

	return S_OK;
}

HRESULT CDragon::Create_BoneBuffer(DRAGON_BONE& bone, 
	_float fW, _float fH, _float fD, const FACE_UV& uv)
{
	//개별 bone에 따른 큐브 생성
	CUBE cube{};
	cube.fWidth = fW;
	cube.fHeight = fH;
	cube.fDepth = fD;
	//6면 동일 UV - Atlas 연동시 면별로 분리
	cube.front = cube.back = cube.top =
		cube.bottom = cube.left = cube.right = uv;

	bone.pBuffer = CCubeBodyTex::Create(m_pGraphicDev, cube);

	if (!bone.pBuffer)
	{
		MSG_BOX("Dragon BoneBuffer Create Failed");
		return E_FAIL;
	}

	D3DXMatrixIdentity(&bone.matWorld);

	return S_OK;
}

void CDragon::Handle_Input(const _float& fTimeDelta)
{
	const _float fInputSpeed = 15.f;
	
	//m_vMoveTarget을 현재 기준을 중심으로 해서 밀어준다
	if (GetAsyncKeyState('W'))
	{
		//vInputDelta += 
	}
}

HRESULT CDragon::Init_SpineChain()
{
	//크기를 Index 0이 가장 앞(목 쪽), 6이 꼬리 쪽
	const _float fW[DRAGON_SPINE_COUNT] = { 2.f, 2.f, 1.8f, 1.6f, 1.4f, 1.2f, 1.f };
	const _float fH[DRAGON_SPINE_COUNT] = { 1.5f, 1.5f, 1.4f, 1.3f, 1.2f, 1.1f, 1.f };
	const _float fD[DRAGON_SPINE_COUNT] = { 1.5f, 1.5f, 1.4f, 1.3f, 1.2f, 1.1f, 1.f };
	
	const _float fBoneLen = 1.5f;
	//임시 전체 UV
	FACE_UV uv = { 0.f, 0.f, 1.f, 1.f };

	for (int i = 0; i < DRAGON_SPINE_COUNT; ++i)
	{
		m_Spine[i].vPos = _vec3(0.f, 0.f, -(_float)i * fBoneLen);
		m_Spine[i].vDir = _vec3(0.f, 0.f, 1.f);
		m_Spine[i].fBoneLen = fBoneLen;

		if (FAILED(Create_BoneBuffer(m_Spine[i], fW[i], fH[i], fD[i], uv)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CDragon::Init_NeckAndHead()
{
	//머리 3개 : Spine 앞에 +Z/+Y 방향으로 이어붙임
	//머리 1개  :목 끝에서 한 단계 더
	const _float fNeckLen = 1.2f;
	FACE_UV uv = { 0.f, 0.f, 1.f, 1.f };

	for (int i = 0; i < DRAGON_NECK_COUNT; ++i)
	{
		_vec3 vBase = m_Spine[0].vPos;
		m_Neck[i].vPos = _vec3(vBase.x, 
			vBase.y + (_float)(i + 1) * 0.4f,
			vBase.z + (_float)(i + 1) * fNeckLen);
		m_Neck[i].vDir = _vec3(0.f, 0.f, 1.f);
		m_Neck[i].fBoneLen = fNeckLen;

		if (FAILED(Create_BoneBuffer(m_Neck[i], 0.8f, 0.8f, 1.f, uv)))
			return E_FAIL;
	}
	//머리 - 목 마지막 뼈 끝에 붙음
	_vec3 vNeckEnd = m_Neck[DRAGON_NECK_COUNT - 1].vPos;
	m_Head.vPos = _vec3(vNeckEnd.x,
		vNeckEnd.y + 0.4f,
		vNeckEnd.z + fNeckLen);
	m_Head.vDir = _vec3(0.f, 0.f, 1.f);
	m_Head.fBoneLen = 1.8f;

	if (FAILED(Create_BoneBuffer(m_Head, 1.6f, 1.2f, 1.8f, uv)))
		return E_FAIL;

	return S_OK;
}

HRESULT CDragon::Init_TailChain()
{
	//꼬리 6개 : Spine[6] 뒤(-Z)에서 시작, 끝으로 갈 수록 최소
	_vec3 vBase = m_Spine[DRAGON_SPINE_COUNT - 1].vPos;
	const _float fTailLen = 1.1f;
	FACE_UV uv = { 0.f, 0.f, 1.f, 1.f };

	for (int i = 0; i < DRAGON_TAIL_COUNT; ++i)
	{
		_float fScale = 1.f - (_float)i * 0.12f; //끝으로 갈 수록 작아짐
		m_Tail[i].vPos = _vec3(vBase.x, vBase.y, vBase.z - (_float)(i + 1) * fTailLen);
		m_Tail[i].vDir = _vec3(0.f, 0.f, -1.f);
		m_Tail[i].fBoneLen = fTailLen;

		if (FAILED(Create_BoneBuffer(m_Tail[i],
			fScale * 1.f, fScale * 1.f, fScale * 1.f, uv)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CDragon::Init_WingChains()
{
	_vec3 vWingRoot = m_Spine[2].vPos;
	const _float fWingLen = 1.8f;
	FACE_UV uv = { 0.f, 0.f, 1.f, 1.f };

	// 세그먼트별 크기: 납작하고 길게 (날개 느낌)
	const _float fWW[DRAGON_WING_COUNT] = { 1.3f, 1.3f, 1.3f, 1.3f };
	const _float fWH[DRAGON_WING_COUNT] = { 0.9f, 0.9f, 0.9f, 0.9f };
	const _float fWD[DRAGON_WING_COUNT] = { 1.3f, 1.3f, 1.3f, 1.3f };

	for (_int i = 0; i < DRAGON_WING_COUNT; ++i)
	{
		_float fOfs = (_float)(i + 1) * fWingLen;

		m_WingL[i].vPos = _vec3(vWingRoot.x - fOfs, vWingRoot.y, vWingRoot.z);
		m_WingL[i].vDir = _vec3(-1.f, 0.f, 0.f);
		m_WingL[i].fBoneLen = fWingLen;

		m_WingR[i].vPos = _vec3(vWingRoot.x + fOfs, vWingRoot.y, vWingRoot.z);
		m_WingR[i].vDir = _vec3(1.f, 0.f, 0.f);
		m_WingR[i].fBoneLen = fWingLen;

		if (FAILED(Create_BoneBuffer(m_WingL[i], fWW[i], fWH[i], fWD[i], uv))) return E_FAIL;
		if (FAILED(Create_BoneBuffer(m_WingR[i], fWW[i], fWH[i], fWD[i], uv))) return E_FAIL;
	}
	return S_OK;
}

_int CDragon::Update_GameObject(const _float& fTimeDelta)
{
	//루트 이동 Spine[0] -> m_vMoveTarget 방향으로 부드럽게 회전
	_vec3 vToTarget = m_vMoveTarget - m_Spine[0].vPos;
	_float fDist = D3DXVec3Length(&vToTarget);

	if (fDist > 0.5f)
	{
		_vec3 vDir;
		D3DXVec3Normalize(&vDir, &vToTarget);

		_vec3 vTargetVel = vDir * m_fMoveSpeed;
		//속도 Lerp : 갑자기 방향 바꾸지 않고 부드럽게 가속
		_float fT = min(fTimeDelta * 3.f, 1.f);
		m_vVelocity = m_vVelocity + (vTargetVel - m_vVelocity) * fT;
		m_Spine[0].vPos += m_vVelocity * fTimeDelta;
		//루트 방향도 이동 방향으로 갱신
		D3DXVec3Normalize(&m_Spine[0].vDir, &vDir);
	}

	//Follow the Leader - Spine[1...6]
	Solve_FollowLeader(m_Spine, DRAGON_SPINE_COUNT);

	//Cyclic Coordinate Dynamics - Neck, Head
	m_Neck[0].vPos = m_Spine[0].vPos + _vec3(0.f, 0.3f, 0.f);

	//Neck + Head를 하나의 배열로 묶어서 CCD에 넘김
	DRAGON_BONE combined[DRAGON_NECK_COUNT + 1];
	for (int i = 0; i < DRAGON_NECK_COUNT; ++i)
	{
		combined[i] = m_Neck[i];
	}

	combined[DRAGON_NECK_COUNT] = m_Head;

	//목 타겟 - 이동 방향, 약간 위 바이어스 -> 드래곤 앞을 바라봄
	_vec3 vNeckTarget = m_vMoveTarget + _vec3(0.f, 1.f, 0.f);
	Solve_CCD(combined, DRAGON_NECK_COUNT + 1, vNeckTarget, 8);
	
	//결과 복사
	for (int i = 0; i < DRAGON_NECK_COUNT; ++i)
	{
		m_Neck[i] = combined[i];
	}
	m_Head = combined[DRAGON_NECK_COUNT];
	
	//Follow the Leader - 꼬리
	m_Tail[0].vPos = m_Spine[DRAGON_SPINE_COUNT - 1].vPos;
	Solve_FollowLeader(m_Tail, DRAGON_TAIL_COUNT);
	
	//Flag
	Update_WingFlap(fTimeDelta);
	
	//전체 뼈 월드 행렬 갱신
	Update_ChainMatrices(m_Spine, DRAGON_SPINE_COUNT);
	Update_ChainMatrices(m_Neck, DRAGON_NECK_COUNT);
	Compute_BoneMatrix(m_Head);
	Update_ChainMatrices(m_Tail, DRAGON_TAIL_COUNT);
	Update_ChainMatrices(m_WingL, DRAGON_WING_COUNT);
	Update_ChainMatrices(m_WingR, DRAGON_WING_COUNT);

	//렌더 그룹 등록
	CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);
	
	return CGameObject::Update_GameObject(fTimeDelta);
}

void CDragon::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CDragon::Render_GameObject()
{
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	//텍스쳐 없으므로 우선은 단색으로 설정
	m_pGraphicDev->SetTexture(0, nullptr);

	Render_Chain(m_Spine, DRAGON_SPINE_COUNT);
	Render_Chain(m_Neck, DRAGON_NECK_COUNT);

	if (m_Head.pBuffer)
	{
		m_pGraphicDev->SetTransform(D3DTS_WORLD, &m_Head.matWorld);
		m_Head.pBuffer->Render_Buffer();
	}

	Render_Chain(m_Tail, DRAGON_TAIL_COUNT);
	Render_Chain(m_WingL, DRAGON_WING_COUNT);
	Render_Chain(m_WingR, DRAGON_WING_COUNT);

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

void CDragon::Solve_CCD(DRAGON_BONE* pChain, _int iCount,
	const _vec3& vTarget, _int iMaxIter, _float fMaxAngle)
{
	const _float fEpsilon = 0.001f;

	for (_int iter = 0; iter < iMaxIter; ++iter)
	{
		_vec3 vDiff = vTarget - pChain[iCount - 1].vPos;
		if (D3DXVec3Length(&vDiff) < fEpsilon) break;

		for (_int i = iCount - 2; i >= 0; --i)
		{
			_vec3 rawEnd = pChain[iCount - 1].vPos - pChain[i].vPos;
			_vec3 rawTarget = vTarget - pChain[i].vPos;
			_vec3 vToEnd, vToTarget;
			D3DXVec3Normalize(&vToEnd, &rawEnd);
			D3DXVec3Normalize(&vToTarget, &rawTarget);

			_float fDot = D3DXVec3Dot(&vToEnd, &vToTarget);
			fDot = max(-1.f, min(1.f, fDot));
			_float fAngle = acosf(fDot);
			if (fAngle < 0.001f) continue;

			fAngle = min(fAngle, fMaxAngle); // 외부에서 제어

			_vec3 vAxis;
			D3DXVec3Cross(&vAxis, &vToEnd, &vToTarget);
			if (D3DXVec3Length(&vAxis) < 0.001f) continue;
			D3DXVec3Normalize(&vAxis, &vAxis);

			_matrix matRot;
			D3DXMatrixRotationAxis(&matRot, &vAxis, fAngle);

			for (_int j = i + 1; j < iCount; ++j)
			{
				_vec3 vRel = pChain[j].vPos - pChain[i].vPos;
				D3DXVec3TransformCoord(&vRel, &vRel, &matRot);
				pChain[j].vPos = pChain[i].vPos + vRel;
				D3DXVec3TransformNormal(&pChain[j].vDir, &pChain[j].vDir, &matRot);
				D3DXVec3Normalize(&pChain[j].vDir, &pChain[j].vDir);
			}
		}
	}
}

void CDragon::Solve_FollowLeader(DRAGON_BONE * pChain, _int iCount)
{
	//Solve_Follow Leader
	for (int i = 1; i < iCount; ++i)
	{
		_vec3 vDir = pChain[i].vPos - pChain[i - 1].vPos;
		float fLen = D3DXVec3Length(&vDir);

		if (fLen < 0.001f)
		{
			//완전히 겹쳤을 경우 이전 뼈 방향으로 밀어내기
			vDir = pChain[i - 1].vDir * -1.f;
		}
		else
		{
			D3DXVec3Normalize(&vDir, &vDir);
		}
		pChain[i].vPos = pChain[i - 1].vPos + vDir * pChain[i].fBoneLen;
		pChain[i].vDir = vDir * -1.f; //체인 앞 방향 갱신
	}
}

void CDragon::Update_WingFlap(const _float & fTimeDelta)
{
	//sin 파형 날개짓
	//fAngle = wingamplitude * (1 + i * 0,15f) * sin(wingtimer + i * 0.3f)
	m_fWingTimer += fTimeDelta * m_fWingSpeed;
	
	_vec3 vWingRoot = m_Spine[2].vPos; //날개 부착점

	for (int i = 0; i < DRAGON_WING_COUNT; ++i)
	{
		float fPhase = (float)i * 0.3; //세그먼트 별 위상
		float fAmplitudeMul = 1.f + (float)i * 0.15f; //진폭
		float fAngle = m_fWingAmp * fAmplitudeMul * sinf(m_fWingTimer + fPhase);

		_matrix matFlapL, matFlapR;
		D3DXMatrixRotationZ(&matFlapL, fAngle);
		D3DXMatrixRotationZ(&matFlapR, -fAngle);

		_vec3 vPrevL = (i == 0) ? vWingRoot : m_WingL[i - 1].vPos;
		_vec3 vPrevR = (i == 0) ? vWingRoot : m_WingR[i - 1].vPos;

		//기준 오프셋 벡터 : 왼쪽 - X, 오른쪽 + X
		_vec3 vBaseL = _vec3(-m_WingL[i].fBoneLen, 0.f, 0.f);
		_vec3 vBaseR = _vec3(m_WingR[i].fBoneLen, 0.f, 0.f);

		D3DXVec3TransformCoord(&vBaseL, &vBaseL, &matFlapL);
		D3DXVec3TransformCoord(&vBaseR, &vBaseR, &matFlapR);

		m_WingL[i].vPos = vPrevL + vBaseL;
		m_WingR[i].vPos = vPrevR + vBaseR;

		D3DXVec3Normalize(&m_WingL[i].vDir, &vBaseL);
		D3DXVec3Normalize(&m_WingR[i].vDir, &vBaseR);
	}
}

void CDragon::Update_TailSwing(const _float& fTimeDelta)
{
	
}

void CDragon::Compute_BoneMatrix(DRAGON_BONE & bone)
{
	//vZ = Noramlize(bone.vDir)
	//vX = Normalize(worldUp * vZ)
	//vY = vZ * vX
	//matWorld(Row_Major)
	//vX.x vX.y vX.z 0 <- row 0 (right)
	//vY.x vY.y vY.z 0 <- row 1 (up)
	//vZ.x vZ.y vZ.z 0 <- row 2 (look)
	//px   py   pz   1 <- row 3 (pos)
	_vec3 vZ = bone.vDir;
	float fLen = D3DXVec3Length(&vZ);

	if (fLen < 0.001f)
	{
		//방향 벡터 없으면 위치만 반영한 단위 행렬
		D3DXMatrixTranslation(&bone.matWorld,
			bone.vPos.x, bone.vPos.y, bone.vPos.z);
		return;
	}
	D3DXVec3Normalize(&vZ, &vZ);
	//worldUp 설정
	_vec3 vWorldUp = _vec3(0.f, 1.f, 0.f);
	if (fabsf(D3DXVec3Dot(&vZ, &vWorldUp)) > 0.99f)
		vWorldUp = _vec3(1.f, 0.f, 0.f);
	//외적 두 번으로 기저벡터 뽑아서 행렬 직접 조립
	_vec3 vX, vY;
	D3DXVec3Cross(&vX, &vWorldUp, &vZ);
	D3DXVec3Normalize(&vX, &vX);
	D3DXVec3Cross(&vY, &vZ, &vX);
	D3DXVec3Normalize(&vY, &vY);

	//row 0 : right(vX)
	bone.matWorld._11 = vX.x; bone.matWorld._12 = vX.y; bone.matWorld._13 = vX.z; bone.matWorld._14 = 0.f;
	//row 1 : up(vY)
	bone.matWorld._21 = vY.x; bone.matWorld._22 = vY.y; bone.matWorld._23 = vY.z; bone.matWorld._24 = 0.f;
	//row 2 : look(vZ)
	bone.matWorld._31 = vZ.x; bone.matWorld._32 = vZ.y; bone.matWorld._33 = vZ.z; bone.matWorld._34 = 0.f;
	//row 3 : position
	bone.matWorld._41 = bone.vPos.x; bone.matWorld._42 = bone.vPos.y;
	bone.matWorld._43 = bone.vPos.z; bone.matWorld._44 = 1.f;
}

void CDragon::Update_ChainMatrices(DRAGON_BONE* pChain, _int iCount)
{
	for (int i = 0; i < iCount; ++i)
	{
		Compute_BoneMatrix(pChain[i]);
	}
}

void CDragon::Render_Chain(DRAGON_BONE * pChain, _int iCount)
{
	for (int i = 0; i < iCount; ++i)
	{
		if (!pChain[i].pBuffer)
			continue;
		m_pGraphicDev->SetTransform(D3DTS_WORLD, &pChain[i].matWorld);
		pChain[i].pBuffer->Render_Buffer();
	}
}

void CDragon::Transition_State(eDragonState eNext)
{
	//Finite State Machine
	if (m_eState == eNext)
		return;
	m_eState = eNext;
	m_fStateTimer = 0.f; //타이머 리셋

	switch (eNext)
	{
	case eDragonState::IDLE:
		//IDLE 진입 : 느린 날개짓, 다음 순찰 지점
		m_fWingSpeed = 2.5f;
		m_fWingAmp = D3DX_PI * 0.3f;
		m_vMoveTarget = s_PatrolPoints[m_iPatrolIndex];
		break;
	case eDragonState::ATTACK:
		m_fWingSpeed = 5.f;
		m_fWingAmp = D3DX_PI * 0.45f;
		m_fMoveSpeed = 8.f;
		break;
	case eDragonState::TAIL_ATTACK:
		m_fTailSwingTimer = 0.f;
		m_fTailSwingAmp = D3DX_PI * 0.8f; //꼬리 최대 회전 144
		m_fMoveSpeed = 3.f; //이동 느르게
		break;
	default:
		break;
	}
}

void CDragon::Update_IDLE(const _float& fTimeDelta)
{
	//현재 순찰 지점으로 이동
	m_vMoveTarget = s_PatrolPoints[m_iPatrolIndex];
	_vec3 vToTarget = m_vMoveTarget - m_Spine[0].vPos;
	_float fDist = D3DXVec3Length(&vToTarget);

	if (fDist > 1.f)
	{
		_vec3 vDir;
		D3DXVec3Normalize(&vDir, &vToTarget);
		_vec3 vTargetVel = vDir * m_fMoveSpeed;
		_float fT = min(fTimeDelta * 2.f, 1.f);
		m_vVelocity = m_vVelocity + (vTargetVel - m_vVelocity) * fT;
		m_Spine[0].vPos += m_vVelocity * fTimeDelta;
		D3DXVec3Normalize(&m_Spine[0].vDir, &vDir);
	}
	else
	{
		//웨이포인트 도달 -> 다음 지점
		m_iPatrolIndex = (m_iPatrolIndex + 1) % m_iPatrolCount;
	}
	//척추 추종
	Solve_FollowLeader(m_Spine, DRAGON_SPINE_COUNT);

	//목은 이동 방향을 바라봄(IDLE : 기본 각도 제한)
	m_Neck[0].vPos = m_Spine[0].vPos + _vec3(0.f, 0.3f, 0.f);
	DRAGON_BONE combined[DRAGON_NECK_COUNT + 1];

	for (int i = 0; i < DRAGON_NECK_COUNT; ++i)
		combined[i] = m_Neck[i];

	combined[DRAGON_NECK_COUNT] = m_Head;

	_vec3 vNeckTarget = m_vMoveTarget + _vec3(0.f, 1.f, 0.f);

	Solve_CCD(combined, DRAGON_NECK_COUNT + 1, vNeckTarget, 6,
		D3DX_PI * 0.2f); //IDLE : 관절 제한 좁게

	for (int i = 0; i < DRAGON_NECK_COUNT; ++i)
	{
		m_Neck[i] = combined[i];
	}
	m_Head = combined[DRAGON_NECK_COUNT];
	//꼬리 추종
	m_Tail[0].vPos = m_Spine[DRAGON_SPINE_COUNT - 1].vPos;
	Solve_FollowLeader(m_Tail, DRAGON_TAIL_COUNT);

	//공격 사거리 내 -> State Translation
	if (DistToPlayer() < m_fAttackRange)
	{
		Transition_State(eDragonState::ATTACK);
	}
}

void CDragon::Update_Attack(const _float& fTimeDelta)
{
	//플레이어를 향해 돌진
	//m_vMoveTarget = m_vPlayerPos + _vec3(0.f, 4.f, 0.f); //머리 위로 약간

	//Player Transform 받아와서 SetPos로 설정
	CComponent* pPlayerTransform = CManagement::GetInstance()->Get_Component(
		ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform");
	if (!pPlayerTransform)
		return;
	
	CTransform* pPlayerTrans = dynamic_cast<CTransform*>(pPlayerTransform);

	pPlayerTrans->Get_Info(INFO_POS, &m_vMoveTarget);
	
	_vec3 vToTarget = m_vMoveTarget - m_Spine[0].vPos;

	_float fDist = D3DXVec3Length(&vToTarget);

	if (fDist > 1.5f)
	{
		_vec3 vDir;
		D3DXVec3Normalize(&vDir, &vToTarget);
		_vec3 vTargetVel = vDir * m_fMoveSpeed;
	}
}

void CDragon::Update_TAIL_ATTACK(const _float& fTimeDelta)
{
	//꼬리 공격
}

_float CDragon::DistToPlayer() const
{
	//플레이어와의 거리 설정
	return 0;
}

CDragon* CDragon::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CDragon* pDragon = new CDragon(pGraphicDev);
	
	if (FAILED(pDragon->Ready_GameObject()))
	{
		Safe_Release(pDragon);
		MSG_BOX("CDragon Create Failed");
		return nullptr;
	}
	return pDragon;
}

void CDragon::Free()
{
	//Release Chain
	auto ReleaseChain = [](DRAGON_BONE* pChain, _int iCount)
		{
			for (int i = 0; i < iCount; ++i)
			{
				Safe_Release(pChain[i].pBuffer);
			}
		};
	ReleaseChain(m_Spine, DRAGON_SPINE_COUNT);
	ReleaseChain(m_Neck, DRAGON_NECK_COUNT);
	Safe_Release(m_Head.pBuffer);
	ReleaseChain(m_Tail, DRAGON_TAIL_COUNT);
	ReleaseChain(m_WingL, DRAGON_WING_COUNT);
	ReleaseChain(m_WingR, DRAGON_WING_COUNT);

	CGameObject::Free();
}
