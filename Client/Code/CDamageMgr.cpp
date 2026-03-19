#include "pch.h"
#include "CDamageMgr.h"
#include "CFontMgr.h"

IMPLEMENT_SINGLETON(CDamageMgr)

CDamageMgr::CDamageMgr()
{
}

CDamageMgr::~CDamageMgr()
{
	Free();
}

HRESULT CDamageMgr::Ready_DamageMgr(LPDIRECT3DDEVICE9 pGraphicDev)
{
	m_pGraphicDev = pGraphicDev;
	m_pGraphicDev->AddRef();
	return S_OK;
}

_int CDamageMgr::Update(const _float& fTimeDelta)
{
	for (auto& tDamage : m_listDamage)
	{
		tDamage.fLifeTime -= fTimeDelta;
		tDamage.fOffsetY += m_fRisingSpeed * fTimeDelta;
	}
	//수명이 끝난 Damage Font 제거
	m_listDamage.remove_if([](const DAMAGE_TEXT& text)
		{
			//lifetime이 0보다 작은 원소 제거
			return text.fLifeTime <= 0.f;
		});

	return 0;
}

void CDamageMgr::LateUpdate(const _float& fTimeDelta)
{}

void CDamageMgr::Render()
{
	for (auto& tDamage : m_listDamage)
	{
		_vec2 vScreen = WorldToScreen(tDamage.vWorldPos);
		vScreen.y -= tDamage.fOffsetY; //위로 올라가기
		//시간에 비례해서 투명
		_float fAlpha = tDamage.fLifeTime / m_fLifeTime;
		D3DXCOLOR color(1.f, 0.2f, 0.2f, fAlpha);

		_tchar szDamage[16];
		wsprintf(szDamage, L"%d", tDamage.iDamage);

		CFontMgr::GetInstance()->Render_Font(
			L"Font_Minecraft", szDamage, &vScreen, color);
	}
}

void CDamageMgr::AddDamage(_vec3 vWorldPos, _int iDamage)
{
	//damage text 구조체에 정보 추가
	DAMAGE_TEXT tDamage;
	tDamage.vWorldPos = vWorldPos;
	tDamage.iDamage = iDamage;	
	tDamage.fLifeTime = m_fLifeTime;
	tDamage.fOffsetY = 0.f;
	m_listDamage.push_back(tDamage);
}

_vec2 CDamageMgr::WorldToScreen(const _vec3 & vWorldPos)
{
	_matrix matView, matProj;
	m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);
	m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matProj);

	D3DVIEWPORT9 viewport;
	m_pGraphicDev->GetViewport(&viewport);

	_vec4 vClip;
	_vec4 vWorld(vWorldPos.x, vWorldPos.y, vWorldPos.z, 1.f);
	D3DXVec4Transform(&vClip, &vWorld, &(matView * matProj));

	// w 가 0 에 가까우면 카메라 뒤에 있는 것 → 화면 밖으로 날림
	if (fabsf(vClip.w) < 0.00001f)
		return _vec2(-9999.f, -9999.f);

	// 클립 공간 → NDC (-1 ~ +1)
	_float fNdcX = vClip.x / vClip.w;
	_float fNdcY = vClip.y / vClip.w;

	// NDC → 스크린 픽셀
	_float fScreenX = (fNdcX + 1.f) * 0.5f * (_float)viewport.Width;
	_float fScreenY = (1.f - fNdcY) * 0.5f * (_float)viewport.Height;

	return _vec2(fScreenX, fScreenY);
}

void CDamageMgr::Free()
{
	Safe_Release(m_pGraphicDev);
}
