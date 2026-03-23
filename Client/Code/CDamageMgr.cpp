#include "pch.h"
#include "CDamageMgr.h"
#include "CFontMgr.h"
#include "CAncientGuardian.h"
#include "CRedStoneGolem.h"

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

HRESULT CDamageMgr::Ready_Component()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

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
		D3DXCOLOR color(1.f, 1.f, 1.f, fAlpha);

		_tchar szDamage[16];
		wsprintf(szDamage, L"%d", tDamage.iDamage);

		CFontMgr::GetInstance()->Render_Font(
			L"Font_Minecraft", szDamage, &vScreen, color);
	}
	Render_BossHP();
}

void CDamageMgr::Render_BossHP()
{
	//if (m_pGuardian)
	//{
	//	Render_SingleBossHP(
	//		m_pGuardian->Get_HP(),
	//		m_pGuardian->Get_MaxHp(),
	//		m_pGuardian->Is_Idle()); // IDLE이면 바 숨기기
	//}

	if (m_pRedStoneGolem)
	{
		bool bIdle = (m_pRedStoneGolem->Get_State() == GOLEM_STATE::GOLEM_STATE_IDLE);
		Render_SingleBossHP(
			m_pRedStoneGolem->Get_HP(),
			m_pRedStoneGolem->Get_MaxHP(),
			bIdle);
	}
}

void CDamageMgr::Render_SingleBossHP(float fHP, float fMaxHP, bool bIdle)
{
	if (bIdle) return;

	float fRatio = (fMaxHP > 0.f) ? fHP / fMaxHP : 0.f;
	float fDamageRatio = 1.f - fRatio;

	// 뷰/투영 UI 모드로 전환
	_matrix matOldView, matOldProj;
	m_pGraphicDev->GetTransform(D3DTS_VIEW, &matOldView);
	m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matOldProj);
	_matrix matIdent;
	D3DXMatrixIdentity(&matIdent);
	m_pGraphicDev->SetTransform(D3DTS_VIEW, &matIdent);
	m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matIdent);
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// 공통 월드 행렬 계산 람다
	auto CalcWorld = [&](_matrix& mat, float fX, float fY, float fW, float fH)
		{
			float fNDCX = (fX + fW * 0.5f) / (WINCX * 0.5f) - 1.f;
			float fNDCY = 1.f - (fY + fH * 0.5f) / (WINCY * 0.5f);
			D3DXMatrixTransformation2D(&mat,
				nullptr, 0.f,
				&_vec2(fW / WINCX, fH / WINCY),
				nullptr, 0.f,
				&_vec2(fNDCX, fNDCY));
		};

	// ── 1패스: 풀 HP 바 배경 ─────────────────────
	_matrix matWorld;
	CalcWorld(matWorld, m_fBarX, m_fBarY, m_fBarW, m_fBarH);
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
	m_pHealthBar->Set_Texture(0);
	m_pBufferCom->Render_Buffer();

	// ── 2패스: 데미지만큼 Empty 오버레이 ─────────
	if (fDamageRatio > 0.f)
	{
		// CHUD와 동일 - 오른쪽에서 줄어드는 방식
		float fEmptyW = m_fBarW * fDamageRatio;
		float fEmptyX = m_fBarX + m_fBarW * fRatio; // 남은 HP 끝에서 시작

		CalcWorld(matWorld, fEmptyX, m_fBarY, fEmptyW, m_fBarH);
		m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

		_matrix matTex;
		D3DXMatrixScaling(&matTex, fDamageRatio, 1.f, 1.f); // 가로로 줄이기
		m_pGraphicDev->SetTransform(D3DTS_TEXTURE0, &matTex);
		m_pGraphicDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

		m_pEmptyHealthBar->Set_Texture(0);
		m_pBufferCom->Render_Buffer();
	}

	// 항상 리셋
	m_pGraphicDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_pGraphicDev->SetTransform(D3DTS_VIEW, &matOldView);
	m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matOldProj);
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

void CDamageMgr::Clear_Boss()
{
	m_pGuardian = nullptr;
	m_pRedStoneGolem = nullptr;
}

void CDamageMgr::Clear_Guardian()
{
	m_pGuardian = nullptr;
}

void CDamageMgr::Clear_RedStone()
{
	m_pRedStoneGolem = nullptr;
}

HRESULT CDamageMgr::Add_Component()
{
	m_pBufferCom = dynamic_cast<CRcTex*>(
		CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
	if (!m_pBufferCom)
		return E_FAIL;
	m_pHealthBar = dynamic_cast<CTexture*>(
		CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_BossHealthBarTexture"));
	if (!m_pHealthBar)
		return E_FAIL;
	m_pEmptyHealthBar = dynamic_cast<CTexture*>(
		CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_EmptyBossHealthBarTexture"));
	if (!m_pEmptyHealthBar)
		return E_FAIL;

	return S_OK;
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
	Safe_Release(m_pHealthBar);
	Safe_Release(m_pEmptyHealthBar);
	Safe_Release(m_pBufferCom);
	Safe_Release(m_pGraphicDev);
}
