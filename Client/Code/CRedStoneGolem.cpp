#include "pch.h"
#include "CRedStoneGolem.h"
#include "CRenderer.h"
#include "CDInputMgr.h"
#include "CBlockMgr.h"
#include "CManagement.h"

CRedStoneGolem::CRedStoneGolem(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
	, m_pTextureCom(nullptr)
	, m_pTransformCom(nullptr)
	, m_pColliderCom(nullptr)
	, m_pAtkColliderCom(nullptr)
	, m_eState(GOLEM_STATE_IDLE)
	, m_fAnimTime(0.f)
	, m_bOnGround(false)
	, m_fVelocityY(0.f)
	, m_fMaxHp(5000.f)
	, m_fHp(5000.f)
	, m_fAtk(100.f)
{
	ZeroMemory(m_pParts, sizeof(m_pParts));
}

CRedStoneGolem::~CRedStoneGolem()
{
}

HRESULT CRedStoneGolem::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	m_pStates[GOLEM_STATE_IDLE] = new CGolemState_Idle();
	m_pStates[GOLEM_STATE_WALK] = new CGolemState_Walk();
	m_pStates[GOLEM_STATE_ATTACK] = new CGolemState_Attack();
	m_pStates[GOLEM_STATE_SKILL] = new CGolemState_Skill();
	m_pStates[GOLEM_STATE_DEAD] = new CGolemState_Dead();

	m_pTransformCom->Set_Pos(-10.f, 10.f, 0.f);

	Set_PartsOffset();
	Set_DefaultScale();
	Set_WorldScale();
	Set_PartsParent();

	m_pCurState = m_pStates[GOLEM_STATE_IDLE];
	m_pCurState->Enter(this);

	return S_OK;
}

_int CRedStoneGolem::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	m_fAnimTime += fTimeDelta;

	// 상태가 알아서 거리 체크, 애니메이션, 이동을 처리
	if (m_pCurState)
		m_pCurState->Update(this, fTimeDelta);

	// Debug 입력은 골렘 본체에서 유지 (개발 편의)
	Debug_Input();

	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);
	m_pColliderCom->Update_OBB(*m_pTransformCom->Get_World());
	m_pAtkColliderCom->Update_OBB(*m_pTransformCom->Get_World());

	Apply_Gravity(fTimeDelta);
	Resolve_BlockCollision();

	for (int i = 0; i < GOLEM_END; ++i)
	{
		m_pParts[i]->Update_GameObject(fTimeDelta);
	}

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

	return iExit;
}

void CRedStoneGolem::LateUpdate_GameObject(const _float& fTimeDelta)
{
	for (int i = 0; i < GOLEM_END; ++i)
	{
		if (m_pParts[i])
			m_pParts[i]->LateUpdate_GameObject(fTimeDelta);
	}
}

void CRedStoneGolem::Render_GameObject()
{
	m_pTextureCom->Set_Texture(0);
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	for (_int i = 0; i < GOLEM_END; ++i)
	{
		m_pParts[i]->Render_GameObject();
	}

	m_pColliderCom->Render_OBB();
	m_pAtkColliderCom->Render_OBB();

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

HRESULT CRedStoneGolem::Add_Component()
{
	Engine::CComponent* pComponent = nullptr;

	// Transform
	pComponent = m_pTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

	// Texture
	pComponent = m_pTextureCom = dynamic_cast<CTexture*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RedStoneGolemTexture"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

	// Collider
	m_pColliderCom = CCollider::Create(m_pGraphicDev, _vec3(7.f, 7.f, 3.f), _vec3(0.f, -1.4f, 0.f));

	m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

	m_pAtkColliderCom = CCollider::Create(m_pGraphicDev, _vec3(7.f, 7.f, 2.5f), _vec3(0.f, -1.4f, 3.0f));

	m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pAtkColliderCom });

	for (int i = 0; i < GOLEM_END; i++)
	{
		m_pParts[i] = CRedStoneGolemPart::Create(m_pGraphicDev, (REDSTONEGOLEM_PART)i);
	}

	return S_OK;
}

void CRedStoneGolem::Set_DefaultScale()
{
	for (_int i = 0; i < GOLEM_END; ++i)
	{
		switch (i)
		{
		case GOLEM_HEAD:
			m_pParts[i]->Get_Transform()->Set_Scale(0.45f);
			break;

		case GOLEM_BODY:
			m_pParts[i]->Get_Transform()->Set_Scale(1.0f);
			break;

		case GOLEM_CORE:
			m_pParts[i]->Get_Transform()->Set_Scale(0.45f);
			break;

		case GOLEM_LSHOULDER:
		case GOLEM_RSHOULDER:
			m_pParts[i]->Get_Transform()->Set_Scale(0.6f);
			break;

		case GOLEM_LARM:
		case GOLEM_RARM:
			m_pParts[i]->Get_Transform()->Set_Scale(0.6f);
			break;

		case GOLEM_HIP:
			m_pParts[i]->Get_Transform()->Set_Scale(0.6f);
			break;

		case GOLEM_LLEG:
		case GOLEM_RLEG:
			m_pParts[i]->Get_Transform()->Set_Scale(0.55f);
			break;

		default:
			break;
		}
	}
}

void CRedStoneGolem::Set_WorldScale()
{
	for (_int i = 0; i < GOLEM_END; ++i)
	{
		m_pParts[i]->Get_Transform()->Set_Scale(m_fWorldScale);
	}
}

void CRedStoneGolem::Set_PartsOffset()
{
	m_pParts[GOLEM_HEAD]->Set_LocalOffset({ 0.f * m_fWorldScale,  0.15f * m_fWorldScale, 0.42f * m_fWorldScale });
	m_pParts[GOLEM_BODY]->Set_LocalOffset({ 0.f * m_fWorldScale,  0.f * m_fWorldScale,    0.f * m_fWorldScale });
	m_pParts[GOLEM_CORE]->Set_LocalOffset({ 0.f * m_fWorldScale, -0.1f * m_fWorldScale,   -0.15f * m_fWorldScale });
	m_pParts[GOLEM_LSHOULDER]->Set_LocalOffset({ 0.7f * m_fWorldScale,  0.f * m_fWorldScale,  0.f * m_fWorldScale });
	m_pParts[GOLEM_RSHOULDER]->Set_LocalOffset({ -0.7f * m_fWorldScale,  0.f * m_fWorldScale,  0.f * m_fWorldScale });
	m_pParts[GOLEM_LARM]->Set_LocalOffset({ 0.1f * m_fWorldScale, -0.6f * m_fWorldScale,  0.f * m_fWorldScale });
	m_pParts[GOLEM_RARM]->Set_LocalOffset({ -0.1f * m_fWorldScale, -0.6f * m_fWorldScale,  0.f * m_fWorldScale });
	m_pParts[GOLEM_HIP]->Set_LocalOffset({ 0.f * m_fWorldScale, -0.5f * m_fWorldScale,   0.f * m_fWorldScale });
	m_pParts[GOLEM_LLEG]->Set_LocalOffset({ 0.35f * m_fWorldScale, -0.4f * m_fWorldScale, 0.f * m_fWorldScale });
	m_pParts[GOLEM_RLEG]->Set_LocalOffset({ -0.35f * m_fWorldScale, -0.4f * m_fWorldScale, 0.f * m_fWorldScale });
}

void CRedStoneGolem::Set_PartsParent()
{
	CTransform* pBody = m_pParts[GOLEM_BODY]->Get_Transform();
	CTransform* pHip = m_pParts[GOLEM_HIP]->Get_Transform();
	CTransform* pLShoulder = m_pParts[GOLEM_LSHOULDER]->Get_Transform();
	CTransform* pRShoulder = m_pParts[GOLEM_RSHOULDER]->Get_Transform();

	m_pParts[GOLEM_BODY]->Get_Transform()->Set_Parent(m_pTransformCom);

	m_pParts[GOLEM_HEAD]->Get_Transform()->Set_Parent(pBody);
	m_pParts[GOLEM_CORE]->Get_Transform()->Set_Parent(pBody);
	m_pParts[GOLEM_LSHOULDER]->Get_Transform()->Set_Parent(pBody);
	m_pParts[GOLEM_RSHOULDER]->Get_Transform()->Set_Parent(pBody);
	m_pParts[GOLEM_HIP]->Get_Transform()->Set_Parent(m_pTransformCom);

	m_pParts[GOLEM_LARM]->Get_Transform()->Set_Parent(pLShoulder);
	m_pParts[GOLEM_RARM]->Get_Transform()->Set_Parent(pRShoulder);
	m_pParts[GOLEM_LLEG]->Get_Transform()->Set_Parent(pHip);
	m_pParts[GOLEM_RLEG]->Get_Transform()->Set_Parent(pHip);
}

void CRedStoneGolem::Debug_Input()
{
	if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_0))
		Change_State(GOLEM_STATE_IDLE);

	if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_9))
		Change_State(GOLEM_STATE_WALK);

	if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_8))
		Change_State(GOLEM_STATE_ATTACK);

	if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_7))
		Change_State(GOLEM_STATE_SKILL);
}

void CRedStoneGolem::Golem_Animation(const _float& fTimeDelta)
{
	switch (m_eState)
	{
	case GOLEM_STATE_IDLE:
		Anim_Idle();
		break;

	case GOLEM_STATE_WALK:
		Anim_Walk();
		break;

	case GOLEM_STATE_ATTACK:
		Anim_NormalAttack();
		break;

	case GOLEM_STATE_SKILL:
		Anim_Skill();
		break;

	case GOLEM_STATE_DEAD:
		Anim_Dead();
		break;

	case GOLEM_STATE_END:
		break;

	default:
		break;
	}
}

void CRedStoneGolem::Reset_Pose()
{
	for (int i = 0; i < GOLEM_END; i++)
	{
		m_pParts[i]->Get_Transform()->Set_Rotation(ROT_X, 0.f);
		m_pParts[i]->Get_Transform()->Set_Rotation(ROT_Y, 0.f);
		m_pParts[i]->Get_Transform()->Set_Rotation(ROT_Z, 0.f);
	}
}

void CRedStoneGolem::Anim_Idle()
{
	const _float fCycleSpeed = 1.2f;
	const _float fAngle = m_fAnimTime * fCycleSpeed;

	m_pParts[GOLEM_LSHOULDER]->Get_Transform()->Set_Rotation(ROT_X, sinf(fAngle) * 5.f);
	m_pParts[GOLEM_RSHOULDER]->Get_Transform()->Set_Rotation(ROT_X, sinf(fAngle + D3DX_PI * 0.1f) * 5.f);

	m_pParts[GOLEM_LARM]->Get_Transform()->Set_Rotation(ROT_X, sinf(fAngle + D3DX_PI * 0.2f) * 3.f);
	m_pParts[GOLEM_RARM]->Get_Transform()->Set_Rotation(ROT_X, sinf(fAngle + D3DX_PI * 0.3f) * 3.f);

	m_pParts[GOLEM_HIP]->Get_Transform()->Set_Rotation(ROT_Z, sinf(fAngle * 0.7f) * 1.5f);
	m_pParts[GOLEM_HIP]->Get_Transform()->Set_Rotation(ROT_X, 0.f);

	m_pParts[GOLEM_LLEG]->Get_Transform()->Set_Rotation(ROT_X, 0.f);
	m_pParts[GOLEM_RLEG]->Get_Transform()->Set_Rotation(ROT_X, 0.f);
}

void CRedStoneGolem::Anim_Walk()
{
	const _float fCycleSpeed = 5.f;
	const _float fAngle = m_fAnimTime * fCycleSpeed;

	m_pParts[GOLEM_LLEG]->Get_Transform()->Set_Rotation(ROT_X, sinf(fAngle) * 25.f);
	m_pParts[GOLEM_RLEG]->Get_Transform()->Set_Rotation(ROT_X, sinf(fAngle + D3DX_PI) * 25.f);

	m_pParts[GOLEM_LSHOULDER]->Get_Transform()->Set_Rotation(ROT_X, sinf(fAngle + D3DX_PI) * 20.f);
	m_pParts[GOLEM_RSHOULDER]->Get_Transform()->Set_Rotation(ROT_X, sinf(fAngle) * 20.f);

	m_pParts[GOLEM_LARM]->Get_Transform()->Set_Rotation(ROT_X, sinf(fAngle + D3DX_PI + D3DX_PI * 0.3f) * 15.f);
	m_pParts[GOLEM_RARM]->Get_Transform()->Set_Rotation(ROT_X, sinf(fAngle + D3DX_PI * 0.3f) * 15.f);

	m_pParts[GOLEM_HIP]->Get_Transform()->Set_Rotation(ROT_Z, sinf(fAngle) * 6.f);
	m_pParts[GOLEM_HIP]->Get_Transform()->Set_Rotation(ROT_X, sinf(fAngle * 2.f) * 4.f);

	m_pParts[GOLEM_BODY]->Get_Transform()->Set_Rotation(ROT_Z, sinf(fAngle + D3DX_PI) * 3.f);
	m_pParts[GOLEM_BODY]->Get_Transform()->Set_Rotation(ROT_X, sinf(fAngle * 2.f + D3DX_PI) * 2.f);
}

void CRedStoneGolem::Anim_NormalAttack()
{
	const _float t = m_fAnimTime;

	if (t < 0.3f)
	{
		_float p = t / 0.3f;
		_float ep = 1.f - (1.f - p) * (1.f - p);

		m_pParts[GOLEM_BODY]->Get_Transform()->Set_Rotation(ROT_Y, 90.f * ep);
		m_pParts[GOLEM_HEAD]->Get_Transform()->Set_Rotation(ROT_Y, -70.f * ep);  
		m_pParts[GOLEM_HIP]->Get_Transform()->Set_Rotation(ROT_Y, -45.f * ep);
		m_pParts[GOLEM_LSHOULDER]->Get_Transform()->Set_Rotation(ROT_X, 80.f * ep);
		m_pParts[GOLEM_LARM]->Get_Transform()->Set_Rotation(ROT_X, 30.f * ep);
	}
	else if (t < 0.55f)
	{
		_float p = (t - 0.3f) / 0.25f;
		_float ep = p * p * p;  // 가속

		// ----- BODY -----
		_float bodyY_start = 90.f;
		_float bodyY_end = -60.f;

		_float bodyX_start = 0.f;
		_float bodyX_end = 10.f;

		m_pParts[GOLEM_BODY]->Get_Transform()->Set_Rotation(
			ROT_Y, bodyY_start + (bodyY_end - bodyY_start) * ep);

		m_pParts[GOLEM_BODY]->Get_Transform()->Set_Rotation(
			ROT_X, bodyX_start + (bodyX_end - bodyX_start) * ep);


		// ----- HIP -----
		_float epHip = ((p - 0.15f) < 0.f ? 0.f : (p - 0.15f) / 0.85f);
		epHip = epHip * epHip * epHip;

		_float hipY_start = -45.f;
		_float hipY_end = -25.f;

		m_pParts[GOLEM_HIP]->Get_Transform()->Set_Rotation(
			ROT_Y, hipY_start + (hipY_end - hipY_start) * epHip);


		// ----- HEAD -----
		_float headY_start = -70.f;
		_float headY_end = -10.f;

		m_pParts[GOLEM_HEAD]->Get_Transform()->Set_Rotation(
			ROT_Y, headY_start + (headY_end - headY_start) * ep);


		// ----- LEFT SHOULDER -----
		_float shoulderX_start = 80.f;
		_float shoulderX_end = 10.f;

		_float shoulderZ_start = 0.f;
		_float shoulderZ_end = 50.f;

		m_pParts[GOLEM_LSHOULDER]->Get_Transform()->Set_Rotation(
			ROT_X, shoulderX_start + (shoulderX_end - shoulderX_start) * ep);

		m_pParts[GOLEM_LSHOULDER]->Get_Transform()->Set_Rotation(
			ROT_Z, shoulderZ_start + (shoulderZ_end - shoulderZ_start) * ep);


		// ----- LEFT ARM -----
		_float epArm = ((p - 0.3f) < 0.f ? 0.f : (p - 0.3f) / 0.7f);
		epArm = epArm * epArm;

		_float armX_start = 30.f;
		_float armX_end = 5.f;

		_float armZ_start = 0.f;
		_float armZ_end = 50.f;

		m_pParts[GOLEM_LARM]->Get_Transform()->Set_Rotation(
			ROT_X, armX_start + (armX_end - armX_start) * epArm);

		m_pParts[GOLEM_LARM]->Get_Transform()->Set_Rotation(
			ROT_Z, armZ_start + (armZ_end - armZ_start) * epArm);


		// ----- RIGHT SHOULDER -----
		_float rShoulderX_start = 0.f;
		_float rShoulderX_end = -30.f;

		_float rShoulderZ_start = 0.f;
		_float rShoulderZ_end = -20.f;

		m_pParts[GOLEM_RSHOULDER]->Get_Transform()->Set_Rotation(
			ROT_X, rShoulderX_start + (rShoulderX_end - rShoulderX_start) * ep);

		m_pParts[GOLEM_RSHOULDER]->Get_Transform()->Set_Rotation(
			ROT_Z, rShoulderZ_start + (rShoulderZ_end - rShoulderZ_start) * ep);
	}
	else if (t < 0.65f)
	{
		m_pParts[GOLEM_BODY]->Get_Transform()->Set_Rotation(ROT_Y, -60.f);
		m_pParts[GOLEM_BODY]->Get_Transform()->Set_Rotation(ROT_X, 10.f);
		m_pParts[GOLEM_HEAD]->Get_Transform()->Set_Rotation(ROT_Y, -10.f);
		m_pParts[GOLEM_HIP]->Get_Transform()->Set_Rotation(ROT_Y, -25.f);
		m_pParts[GOLEM_LSHOULDER]->Get_Transform()->Set_Rotation(ROT_Z, 50.f);
		m_pParts[GOLEM_LSHOULDER]->Get_Transform()->Set_Rotation(ROT_X, 10.f);
		m_pParts[GOLEM_LARM]->Get_Transform()->Set_Rotation(ROT_Z, 50.f);
		m_pParts[GOLEM_LARM]->Get_Transform()->Set_Rotation(ROT_X, 5.f);
		m_pParts[GOLEM_RSHOULDER]->Get_Transform()->Set_Rotation(ROT_X, -30.f);
		m_pParts[GOLEM_RSHOULDER]->Get_Transform()->Set_Rotation(ROT_Z, -20.f);
	}
	else
	{
		_float p = (t - 0.65f) / 0.55f;
		_float ep = 1.f - (1.f - p) * (1.f - p);

		m_pParts[GOLEM_BODY]->Get_Transform()->Set_Rotation(ROT_Y, -60.f * (1.f - ep));
		m_pParts[GOLEM_BODY]->Get_Transform()->Set_Rotation(ROT_X, 10.f * (1.f - ep));
		m_pParts[GOLEM_HEAD]->Get_Transform()->Set_Rotation(ROT_Y, -10.f * (1.f - ep));
		m_pParts[GOLEM_HIP]->Get_Transform()->Set_Rotation(ROT_Y, -25.f * (1.f - ep));
		m_pParts[GOLEM_LSHOULDER]->Get_Transform()->Set_Rotation(ROT_Z, 50.f * (1.f - ep));
		m_pParts[GOLEM_LSHOULDER]->Get_Transform()->Set_Rotation(ROT_X, 10.f * (1.f - ep));
		m_pParts[GOLEM_LARM]->Get_Transform()->Set_Rotation(ROT_Z, 50.f * (1.f - ep));
		m_pParts[GOLEM_LARM]->Get_Transform()->Set_Rotation(ROT_X, 5.f * (1.f - ep));
		m_pParts[GOLEM_RSHOULDER]->Get_Transform()->Set_Rotation(ROT_X, -30.f * (1.f - ep));
		m_pParts[GOLEM_RSHOULDER]->Get_Transform()->Set_Rotation(ROT_Z, -20.f * (1.f - ep));
	}

	if (m_fAnimTime >= 1.2f)
		Change_State(GOLEM_STATE_IDLE);
}

void CRedStoneGolem::Anim_Skill()
{
	const _float t = m_fAnimTime;

	if (t < 0.8f)
	{
		float p = t / 0.8f;
		float ep = 1.f - (1.f - p) * (1.f - p);

		m_pParts[GOLEM_LSHOULDER]->Get_Transform()->Set_Rotation(ROT_X, 120.f * ep);
		m_pParts[GOLEM_RSHOULDER]->Get_Transform()->Set_Rotation(ROT_X, 120.f * ep);

		m_pParts[GOLEM_LARM]->Get_Transform()->Set_Rotation(ROT_X, 40.f * ep);
		m_pParts[GOLEM_RARM]->Get_Transform()->Set_Rotation(ROT_X, 40.f * ep);

		m_pParts[GOLEM_BODY]->Get_Transform()->Set_Rotation(ROT_X, -10.f * ep);

		m_pParts[GOLEM_HIP]->Get_Transform()->Set_Rotation(ROT_X, 0.f);
	}
	else if (t < 1.2f)
	{
		float p = (t - 0.8f) / 0.4f;
		float ep = p * p;

		m_pParts[GOLEM_BODY]->Get_Transform()->Set_Rotation(ROT_X, -10.f + (50.f * ep));

		m_pParts[GOLEM_HIP]->Get_Transform()->Set_Rotation(ROT_X, -20.f * ep);

		m_pParts[GOLEM_LSHOULDER]->Get_Transform()->Set_Rotation(ROT_X, 120.f);
		m_pParts[GOLEM_RSHOULDER]->Get_Transform()->Set_Rotation(ROT_X, 120.f);

		m_pParts[GOLEM_LARM]->Get_Transform()->Set_Rotation(ROT_X, 40.f);
		m_pParts[GOLEM_RARM]->Get_Transform()->Set_Rotation(ROT_X, 40.f);
	}
	else if (t < 1.6f)
	{
		float p = (t - 1.2f) / 0.4f;
		float ep = p * p * p;

		float shoulder = 120.f + (-70.f * ep); // 120 → 50
		float arm = 40.f + (20.f * ep);        // 40 → 60

		m_pParts[GOLEM_LSHOULDER]->Get_Transform()->Set_Rotation(ROT_X, shoulder);
		m_pParts[GOLEM_RSHOULDER]->Get_Transform()->Set_Rotation(ROT_X, shoulder);

		m_pParts[GOLEM_LARM]->Get_Transform()->Set_Rotation(ROT_X, arm);
		m_pParts[GOLEM_RARM]->Get_Transform()->Set_Rotation(ROT_X, arm);

		m_pParts[GOLEM_BODY]->Get_Transform()->Set_Rotation(ROT_X, 50.f + (20.f * ep));

		m_pParts[GOLEM_HIP]->Get_Transform()->Set_Rotation(ROT_X, -20.f);
	}
	else
	{
		float p = (t - 1.6f) / 0.8f;
		float ep = 1.f - (1.f - p) * (1.f - p);

		m_pParts[GOLEM_LSHOULDER]->Get_Transform()->Set_Rotation(ROT_X, 50.f * (1.f - ep));
		m_pParts[GOLEM_RSHOULDER]->Get_Transform()->Set_Rotation(ROT_X, 50.f * (1.f - ep));

		m_pParts[GOLEM_LARM]->Get_Transform()->Set_Rotation(ROT_X, 60.f * (1.f - ep));
		m_pParts[GOLEM_RARM]->Get_Transform()->Set_Rotation(ROT_X, 60.f * (1.f - ep));

		m_pParts[GOLEM_BODY]->Get_Transform()->Set_Rotation(ROT_X, 70.f * (1.f - ep));

		m_pParts[GOLEM_HIP]->Get_Transform()->Set_Rotation(ROT_X, -20.f * (1.f - ep));
	}

	m_pParts[GOLEM_LLEG]->Get_Transform()->Set_Rotation(ROT_X, 0.f);
	m_pParts[GOLEM_RLEG]->Get_Transform()->Set_Rotation(ROT_X, 0.f);

	if (m_fAnimTime >= 2.4f)
		Change_State(GOLEM_STATE_IDLE);
}

void CRedStoneGolem::Anim_Dead()
{
	// todo
}

void CRedStoneGolem::Chase_Player(const _float& fTimeDelta)
{
	_vec3 vPlayerPos;

	CTransform* pPlayerTransformCom = dynamic_cast<CTransform*>(CManagement::GetInstance()->Get_Component(ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform"));
	pPlayerTransformCom->Get_Info(INFO_POS, &vPlayerPos);

	m_pTransformCom->Chase_Target(&vPlayerPos, 4.f, fTimeDelta);
}

void CRedStoneGolem::LookAt_Player()
{
	_vec3 vPos, vPlayerPos, vDir;

	m_pTransformCom->Get_Info(INFO_POS, &vPos);

	CTransform* pPlayerTransform = dynamic_cast<CTransform*>(
		CManagement::GetInstance()->Get_Component(
			ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform"));

	pPlayerTransform->Get_Info(INFO_POS, &vPlayerPos);

	vDir = vPlayerPos - vPos;
	vDir.y = 0.f;   // 수평 방향만
	D3DXVec3Normalize(&vDir, &vDir);

	_float fAngle = atan2f(vDir.x, vDir.z);
	m_pTransformCom->Set_Rotation(ROT_Y, D3DXToDegree(fAngle));
}

void CRedStoneGolem::Check_Distance()
{
	_vec3 vPos, vPlayerPos, vDir;

	m_pTransformCom->Get_Info(INFO_POS, &vPos);

	CTransform* pPlayerTransformCom = dynamic_cast<CTransform*>(CManagement::GetInstance()->Get_Component(ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform"));

	pPlayerTransformCom->Get_Info(INFO_POS, &vPlayerPos);

	vDir = vPlayerPos - vPos;

	_float fDistance = D3DXVec3Length(&vDir);

	if (fDistance <= 6.f)
		Change_State(GOLEM_STATE_ATTACK);
	else if (fDistance <= 15.f)
		Change_State(GOLEM_STATE_WALK);
	else
		Change_State(GOLEM_STATE_IDLE);
}

void CRedStoneGolem::Change_State(GOLEM_STATE eState)
{
	// 현재 상태가 전환을 허용하지 않으면 무시
	// → 공격/스킬 중엔 Check_Distance가 Walk 요청해도 차단됨
	if (m_pCurState && !m_pCurState->Can_Transition())
		return;

	if (m_pCurState)
		m_pCurState->Exit(this);

	m_pCurState = m_pStates[eState];
	m_eState = eState;

	m_pCurState->Enter(this);
}

void CRedStoneGolem::Apply_Gravity(const _float& fTimeDelta)
{
	if (m_bOnGround)
		return;

	m_fVelocityY += m_fGravity * fTimeDelta;
	if (m_fVelocityY < m_fMaxFall)
		m_fVelocityY = m_fMaxFall;

	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);
	vPos.y += m_fVelocityY * fTimeDelta;
	m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
}

void CRedStoneGolem::Resolve_BlockCollision()
{
	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);
	m_pColliderCom->Update_AABB(vPos);
	AABB tBossAABB = m_pColliderCom->Get_AABB();

	m_bOnGround = false;

	int iMinX = (int)floorf(tBossAABB.vMin.x);
	int iMaxX = (int)ceilf(tBossAABB.vMax.x);
	int iMinY = (int)floorf(tBossAABB.vMin.y) - 3;
	int iMaxY = (int)ceilf(tBossAABB.vMax.y);
	int iMinZ = (int)floorf(tBossAABB.vMin.z);
	int iMaxZ = (int)ceilf(tBossAABB.vMax.z);

	for (int y = iMinY; y <= iMaxY; ++y)
	{
		for (int x = iMinX; x <= iMaxX; ++x)
		{
			for (int z = iMinZ; z <= iMaxZ; ++z)
			{
				BlockPos tBlockPos = { x, y, z };
				if (!CBlockMgr::GetInstance()->HasBlock(tBlockPos))
					continue;

				AABB tBlockAABB = CBlockMgr::GetInstance()->Get_BlockAABB(tBlockPos);
				if (!m_pColliderCom->IsColliding(tBlockAABB))
					continue;

				_vec3 vResolve = m_pColliderCom->Resolve(tBlockAABB);
				vPos += vResolve;
				m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
				m_pColliderCom->Update_AABB(vPos);
				tBossAABB = m_pColliderCom->Get_AABB();

				if (fabsf(vResolve.y) > 0.f)
				{
					if (vResolve.y > 0.f)
					{
						m_bOnGround = true;
						m_fVelocityY = 0.f;
					}
					else
					{
						m_fVelocityY = 0.f;
					}
				}
			}
		}
	}
}

CRedStoneGolem* CRedStoneGolem::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CRedStoneGolem* pRedStoneGolem = new CRedStoneGolem(pGraphicDev);

	if (FAILED(pRedStoneGolem->Ready_GameObject()))
	{
		Safe_Release(pRedStoneGolem);
		MSG_BOX("RedStoneGolem Create Failed");
		return nullptr;
	}

	return pRedStoneGolem;
}

void CRedStoneGolem::Free()
{
	for (int i = 0; i < GOLEM_STATE_END; ++i)
	{
		delete m_pStates[i];
		m_pStates[i] = nullptr;
	}

	for (int i = 0; i < GOLEM_END; ++i)
	{
		Safe_Release(m_pParts[i]);
	}

	CGameObject::Free();
}