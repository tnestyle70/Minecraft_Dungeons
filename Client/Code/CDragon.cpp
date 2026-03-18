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
	, m_vInputForward(0.f, 0.f, 1.f)
	, m_vInputRight(1.f, 0.f, 0.f)
	, m_vPlayerPos(0.f, 0.f, 0.f)
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
	, m_vInputForward(rhs.m_vInputForward)
	, m_vInputRight(rhs.m_vInputRight)
	, m_vPlayerPos(rhs.m_vPlayerPos)
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

	for (_int i = 0; i < DRAGON_NECK_COUNT; ++i)
		m_Neck[i].qRot = DirToQuaternion(m_Neck[i].vDir);
	m_Head.qRot = DirToQuaternion(m_Head.vDir);

	//텍스쳐 컴포넌트 클론 - CBlock과 완전히 동일한 방식
	m_pTextureCom = dynamic_cast<CTexture*>(
		CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_ObsidianPngTexture"));

	if (!m_pTextureCom)
	{
		MSG_BOX("Dragon Texture Clone Failed");
		return E_FAIL;
	}

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
	_vec3 vInputDelta(0.f, 0.f, 0.f);
	m_bManualControl = false;
	
	//m_vMoveTarget을 현재 기준을 중심으로 해서 밀어준다
	if (GetAsyncKeyState(VK_UP) & 0x8000)
	{
		vInputDelta += m_vInputForward * fInputSpeed * fTimeDelta;
		m_bManualControl = true;
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
	{
		vInputDelta -= m_vInputForward * fInputSpeed * fTimeDelta;
		m_bManualControl = true;
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
	{
		vInputDelta += m_vInputRight * fInputSpeed * fTimeDelta;
		m_bManualControl = true;
	}
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
	{
		vInputDelta -= m_vInputRight * fInputSpeed * fTimeDelta;
		m_bManualControl = true;
	}
	if (GetAsyncKeyState('Q') & 0x8000)
	{
		vInputDelta.y += fInputSpeed * fTimeDelta;
		m_bManualControl = true;
	}
	if (GetAsyncKeyState('E') & 0x8000)
	{
		vInputDelta.y -= fInputSpeed * fTimeDelta;
		m_bManualControl = true;
	}
	//움직였을 경우 타겟을 입력만큼 움직여주기
	if (m_bManualControl)
		m_vMoveTarget += vInputDelta;
	// A/D 입력 시 전진 방향 자체를 Y축 회전 → 용이 선회함
	if (GetAsyncKeyState(VK_LEFT) & 0x8000 || GetAsyncKeyState(VK_RIGHT) & 0x8000)
	{
		_float fTurnDir = (GetAsyncKeyState(VK_RIGHT) & 0x8000) ? 1.f : -1.f;
		_float fTurnAngle = D3DX_PI * fTurnDir * fTimeDelta * 1.2f;
		_matrix matTurn;
		D3DXMatrixRotationY(&matTurn, fTurnAngle);
		D3DXVec3TransformNormal(&m_vInputForward, &m_vInputForward, &matTurn);
		D3DXVec3TransformNormal(&m_vInputRight, &m_vInputRight, &matTurn);
		D3DXVec3Normalize(&m_vInputForward, &m_vInputForward);
		D3DXVec3Normalize(&m_vInputRight, &m_vInputRight);
	}

	// ── RTY 상태 강제 전환 (단발) ──────────────────
	// GetAsyncKeyState 는 홀드 감지라 Key_Down 흉내 필요
	// static으로 이전 프레임 상태 저장해서 비교
	static bool bR_prev = false, bT_prev = false, bY_prev = false;

	bool bR_cur = (GetAsyncKeyState('R') & 0x8000) != 0;
	bool bT_cur = (GetAsyncKeyState('T') & 0x8000) != 0;
	bool bY_cur = (GetAsyncKeyState('Y') & 0x8000) != 0;

	if (bR_cur && !bR_prev) Transition_State(eDragonState::IDLE);
	if (bT_cur && !bT_prev) Transition_State(eDragonState::ATTACK);
	if (bY_cur && !bY_prev) Transition_State(eDragonState::TAIL_ATTACK);

	bR_prev = bR_cur;
	bT_prev = bT_cur;
	bY_prev = bY_cur;
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
	Handle_Input(fTimeDelta);

	m_fStateTimer += fTimeDelta;

	//Finite State Machine Dispatch
	switch (m_eState)
	{
	case eDragonState::IDLE:
	{
		Update_IDLE(fTimeDelta);
		break;
	}
	case eDragonState::ATTACK:
	{
		Update_Attack(fTimeDelta);
		break;
	}
	case eDragonState::TAIL_ATTACK:
	{
		Update_TailAttack(fTimeDelta);
		break;
	}
	default:
		break;
	}
	
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

	//텍스쳐 바인딩
	if (m_pTextureCom)
		m_pTextureCom->Set_Texture(0);

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

D3DXQUATERNION CDragon::DirToQuaternion(const _vec3& vDir)
{
	//vDir로 LookAt 행렬 만들고 쿼터니언 변환
	_vec3 vZ = vDir;
	if (D3DXVec3Length(&vZ) < 0.0001f)
	{
		D3DXQUATERNION q;
		D3DXQuaternionIdentity(&q);
		return q;
	}
	D3DXVec3Normalize(&vZ, &vZ);

	_vec3 vWorldUp = _vec3(0.f, 1.f, 0.f);
	if (fabs(D3DXVec3Dot(&vZ, &vWorldUp)) > 0.99f)
		vWorldUp = _vec3(1.f, 0.f, 0.f);

	_vec3 vX, vY;

	D3DXVec3Cross(&vX, &vWorldUp, &vZ);
	D3DXVec3Normalize(&vX, &vX);
	D3DXVec3Cross(&vY, &vZ, &vX);
	D3DXVec3Normalize(&vY, &vY);

	_matrix mat;
	D3DXMatrixIdentity(&mat);
	mat._11 = vX.x; mat._12 = vX.y; mat._13 = vX.z;
	mat._21 = vY.x; mat._22 = vY.y; mat._23 = vY.z;
	mat._31 = vZ.x; mat._32 = vZ.y; mat._33 = vZ.z;
	
	D3DXQUATERNION q;
	D3DXQuaternionRotationMatrix(&q, &mat);
	D3DXQuaternionNormalize(&q, &q);

	return q;
}

_vec3 CDragon::QuaternionToDir(const D3DXQUATERNION& quaternion)
{
	//쿼터니언 -> 행렬 -> row2 추출
	_matrix mat;
	D3DXMatrixRotationQuaternion(&mat, &quaternion);
	return _vec3(mat._31, mat._32, mat._33);
}

void CDragon::Slerp_NeckChain(DRAGON_BONE* pResult, //Cyclic Coordinate Descent
	DRAGON_BONE* pCurrent, //현재 실제 뼈 배열
	_int iCount, 
	_float fAlpha) //보간 강도(0 ~ 1, 클수록 빠름)
{
	for (int i = 0; i < iCount; ++i)
	{
		//1. 목표 vDir -> 쿼터니언
		D3DXQUATERNION qTarget = DirToQuaternion(pResult[i].vDir);
		//2. dot 부호 확인
		//dot < 0이면 한 쪽을 반젆새ㅓ 항상 짧은 경로로 Slerp
		_float fDot = D3DXQuaternionDot(&pCurrent[i].qRot, &qTarget);
		if (fDot < 0.f)
		{
			qTarget = -qTarget; //부호 반전
		}
		//3. Slerp 보간
		D3DXQUATERNION qResult;
		D3DXQuaternionSlerp(&qResult, &pCurrent[i].qRot, &qTarget, fAlpha);
		D3DXQuaternionNormalize(&qResult, &qResult);
		//4.보간 결과를 현재 뼈에 적용
		pCurrent[i].qRot = qResult;
		pCurrent[i].vDir = QuaternionToDir(qResult);
		//5. 위치는 CCD 결과 그대로 쓰되, 방향이 바뀌었으니 앞 뼈 기준으로 재계산
		if (i > 0)
		{
			pCurrent[i].vPos = pCurrent[i - 1].vPos
				+ pCurrent[i].vDir * pCurrent[i].fBoneLen;
		}
		//i = 0은 루트 위치 고정
	}
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
	if (!m_bManualControl)
	{
		m_vMoveTarget = s_PatrolPoints[m_iPatrolIndex];

		_float fDist = D3DXVec3Length(&(m_vMoveTarget - m_Spine[0].vPos));
		if (fDist < 1.f)
			m_iPatrolIndex = (m_iPatrolIndex + 1) % 4; // m_iPatrolCount 대신 4
	}
	//m_vMoveTarget을 향해서 이동
	_vec3 vToTarget = m_vMoveTarget - m_Spine[0].vPos;	
	_float fDist = D3DXVec3Length(&vToTarget);
	//척추, 기준이 되는 Spine으로 용 이동
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
	//설정한 m_Spine[0]을 기준으로 전부 이동
	Solve_FollowLeader(m_Spine, DRAGON_SPINE_COUNT);
	//Neck Pos 설정
	m_Neck[0].vPos = m_Spine[0].vPos + _vec3(0.f, 0.3f, 0.f);


	//쿼터니언 Slerp 적용
	DRAGON_BONE combined[DRAGON_NECK_COUNT + 1];
	for (int i = 0; i < DRAGON_NECK_COUNT; ++i)
		combined[i] = m_Neck[i];
	combined[DRAGON_NECK_COUNT] = m_Head;

	_vec3 vNeckTarget = m_vMoveTarget + _vec3(0.f, 1.f, 0.f);
	Solve_CCD(combined, DRAGON_NECK_COUNT + 1, vNeckTarget, 6, D3DX_PI * 0.2f);

	// CCD 결과(combined) → 현재(m_Neck) 로 Slerp
	// fAlpha: IDLE 은 0.05f (느리게), ATTACK 은 0.15f (빠르게)
	DRAGON_BONE neckAndHead[DRAGON_NECK_COUNT + 1];
	for (int i = 0; i < DRAGON_NECK_COUNT; ++i)
		neckAndHead[i] = m_Neck[i];
	neckAndHead[DRAGON_NECK_COUNT] = m_Head;

	Slerp_NeckChain(combined, neckAndHead, DRAGON_NECK_COUNT + 1, fTimeDelta * 3.f);

	for (int i = 0; i < DRAGON_NECK_COUNT; ++i)
		m_Neck[i] = neckAndHead[i];
	m_Head = neckAndHead[DRAGON_NECK_COUNT];
	//꼬리 따라가기
	m_Tail[0].vPos = m_Spine[DRAGON_SPINE_COUNT - 1].vPos;
	Solve_FollowLeader(m_Tail, DRAGON_TAIL_COUNT);
}

void CDragon::Update_Attack(const _float& fTimeDelta)
{
	//Player Transform 받아와서 SetPos로 설정
	CComponent* pCom = CManagement::GetInstance()->Get_Component(
		ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform");
	if (!pCom)
		return;
	CTransform* pTransform = dynamic_cast<CTransform*>(pCom);
	if (pTransform)
		pTransform->Get_Info(INFO_POS, &m_vPlayerPos);
	//플레이어 위쪽으로 타겟 설정
	m_vMoveTarget = m_vPlayerPos + _vec3(0.f, 3.f, 0.f);
	//Spine0과 위에서 구한 MoveTarget과의 차이로 ToTarget과의 방향 벡터 구하기
	_vec3 vToTarget = m_vMoveTarget - m_Spine[0].vPos;
	_float fDist = D3DXVec3Length(&vToTarget);

	if (fDist > 1.5f)
	{
		_vec3 vDir;
		D3DXVec3Normalize(&vDir, &vToTarget);
		_vec3 vTargetVel = vDir * m_fMoveSpeed;
		//거리?
		_float fT = min(fTimeDelta * 4.f, 1.f);
		m_vVelocity = m_vVelocity + (vTargetVel - m_vVelocity) * fT;
		m_Spine[0].vPos += m_vVelocity * fTimeDelta;
		D3DXVec3Normalize(&m_Spine[0].vDir, &vDir);
	}

	//목 머리 : 플레이어를 향해 움직임
	Solve_FollowLeader(m_Spine, DRAGON_SPINE_COUNT);

	// 목 머리 : 플레이어를 향해 움직임  ← 주석만 있고 코드 없음
	// 아래 추가 필요
	m_Neck[0].vPos = m_Spine[0].vPos + _vec3(0.f, 0.3f, 0.f);

	DRAGON_BONE combined[DRAGON_NECK_COUNT + 1];
	for (int i = 0; i < DRAGON_NECK_COUNT; ++i)
		combined[i] = m_Neck[i];
	combined[DRAGON_NECK_COUNT] = m_Head;

	Solve_CCD(combined, DRAGON_NECK_COUNT + 1, m_vPlayerPos, 10,
		D3DX_PI * 0.45f); // ATTACK: 각도 제한 완화

	DRAGON_BONE neckAndHead[DRAGON_NECK_COUNT + 1];
	for (int i = 0; i < DRAGON_NECK_COUNT; ++i)
		neckAndHead[i] = m_Neck[i];
	neckAndHead[DRAGON_NECK_COUNT] = m_Head;

	Slerp_NeckChain(combined, neckAndHead, DRAGON_NECK_COUNT + 1,
		fTimeDelta * 8.f); // ATTACK: 빠르게 추적

	for (int i = 0; i < DRAGON_NECK_COUNT; ++i)
		m_Neck[i] = neckAndHead[i];
	m_Head = neckAndHead[DRAGON_NECK_COUNT];

	// 꼬리도 추가
	m_Tail[0].vPos = m_Spine[DRAGON_SPINE_COUNT - 1].vPos;
	Solve_FollowLeader(m_Tail, DRAGON_TAIL_COUNT);
}

void CDragon::Update_TailAttack(const _float& fTimeDelta)
{
	//꼬리 공격
}

_float CDragon::DistToPlayer() const
{
	CComponent* pComp = CManagement::GetInstance()->Get_Component(
		ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform");
	if (!pComp) return FLT_MAX; // 플레이어 없으면 무한 거리 반환

	CTransform* pTrans = dynamic_cast<CTransform*>(pComp);
	if (!pTrans) return FLT_MAX;

	_vec3 vPlayerPos;
	pTrans->Get_Info(INFO_POS, &vPlayerPos);

	_vec3 vDiff = vPlayerPos - m_Spine[0].vPos;
	return D3DXVec3Length(&vDiff);
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
	Safe_Release(m_pTextureCom);

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
