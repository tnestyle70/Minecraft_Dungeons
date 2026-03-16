#include "pch.h"
#include "CDragon.h"
#include "CRenderer.h"

CDragon::CDragon(LPDIRECT3DDEVICE9 pGraphicDev)
	:CGameObject(pGraphicDev)
	, m_vMoveTarget(0.f, 8.f, 10.f)
	, m_vVelocity(0.f, 0.f, 0.f)
	, m_fMoveSpeed(5.f)
	, m_fWingTimer(0.f)
	, m_fWingSpeed(3.5f)
	, m_fWingAmp(D3DX_PI * 0.35f)
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
{}

CDragon::~CDragon()
{}

HRESULT CDragon::Ready_GameObject()
{
	return E_NOTIMPL;
}

_int CDragon::Update_GameObject(const _float& fTimeDelta)
{
	return _int();
}

void CDragon::LateUpdate_GameObject(const _float& fTimeDelta)
{}

void CDragon::Render_GameObject()
{}

HRESULT CDragon::Init_SpineChain()
{
	return E_NOTIMPL;
}

HRESULT CDragon::Init_NeckAndHead()
{
	return E_NOTIMPL;
}

HRESULT CDragon::Init_TailChain()
{
	return E_NOTIMPL;
}

HRESULT CDragon::Init_WingChains()
{
	return E_NOTIMPL;
}

HRESULT CDragon::Create_BoneBuffer(DRAGON_BONE& bone, _float fW, _float fH, _float fD, const FACE_UV& uv)
{
	return E_NOTIMPL;
}

void CDragon::Solve_CCD(DRAGON_BONE* pChain, _int iCount, const _vec3& vTarget, _int iMaxIter)
{}

void CDragon::Solve_FollowLeader(DRAGON_BONE * pChain, _int iCount)
{}

void CDragon::Update_WingFlap(const _float & fTimeDelta)
{}

void CDragon::Compute_BoneMatrix(DRAGON_BONE & bone)
{}

void CDragon::Update_ChainMatrices(DRAGON_BONE * pChain, _int iCount)
{}

void CDragon::Render_Chain(DRAGON_BONE * pChain, _int iCount)
{}

CDragon* CDragon::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	return nullptr;
}

void CDragon::Free()
{}
