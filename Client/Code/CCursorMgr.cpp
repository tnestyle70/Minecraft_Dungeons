#include "pch.h"
#include "CCursorMgr.h"

IMPLEMENT_SINGLETON(CCursorMgr)

CCursorMgr::CCursorMgr()
{
	ZeroMemory(m_pTextures, sizeof(m_pTextures));
	D3DXMatrixIdentity(&m_matOriginView);
	D3DXMatrixIdentity(&m_matOriginProj);
}

CCursorMgr::~CCursorMgr()
{
	Free();
}

HRESULT CCursorMgr::Ready_CursorMgr(LPDIRECT3DDEVICE9 pGraphicDev)
{
	m_pGraphicDev = pGraphicDev;
	m_pGraphicDev->AddRef();

	//마우스 커서 숨기기
	ShowCursor(FALSE);

	if (FAILED(AddComponent()))
		return E_FAIL;

	return S_OK;
}

_int CCursorMgr::Update(const _float& fTimeDelta)
{
	//마우스 스크린 -> 클라이언트 변환 이후 클릭 상태 감지
	POINT pt{};
	GetCursorPos(&pt);
	ScreenToClient(g_hWnd, &pt);
	m_vMousePos = _vec2((_float)pt.x, (_float)pt.y);
	//좌클릭 감지시 마우스 커서 교체
	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
	{
		m_eCursorState = eCursorState::CLICK;
	}
	//좌클릭이 아니고, Click 상태일 경우 Default로 되돌리기
	else if (m_eCursorState == eCursorState::CLICK)
	{
		m_eCursorState = eCursorState::DEFAULT;
	}
	
	return 0;
}

void CCursorMgr::Render()
{
	if (!m_pBufferCom) return;

	// 텍스쳐 선택 (없으면 DEFAULT 로 폴백)
	CTexture* pTex = m_pTextures[(_int)m_eCursorState];
	if (!pTex)
		pTex = m_pTextures[(_int)eCursorState::DEFAULT];
	if (!pTex) return;

	BeginCursorRender();

	// NDC 변환
	_float fNDCX = (m_vMousePos.x + m_fCursorSize * 0.5f) / (WINCX * 0.5f) - 1.f;
	_float fNDCY = 1.f - (m_vMousePos.y + m_fCursorSize * 0.5f) / (WINCY * 0.5f);
	_float fScaleX = m_fCursorSize / WINCX;
	_float fScaleY = m_fCursorSize / WINCY;

	_matrix matWorld;
	D3DXMatrixTransformation2D(&matWorld,
		nullptr, 0.f,
		&_vec2(fScaleX, fScaleY),
		nullptr, 0.f,
		&_vec2(fNDCX, fNDCY));

	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

	pTex->Set_Texture(0);
	m_pBufferCom->Render_Buffer();

	EndCursorRender();
}

HRESULT CCursorMgr::AddComponent()
{
	m_pBufferCom = dynamic_cast<CRcTex*>(
		CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
	if (!m_pBufferCom)
	{
		MSG_BOX("CursorMgr Buffer Clone Failed");
		return E_FAIL;
	}
	//상태별 텍스쳐 등록
	m_pTextures[(_int)eCursorState::DEFAULT] = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_CursorTexture"));
	m_pTextures[(_int)eCursorState::CLICK] = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_CursorClickTexture"));
	m_pTextures[(_int)eCursorState::ENEMY] = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_AttackCursorTexture"));
	m_pTextures[(_int)eCursorState::ENEMY_HOVER] = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_AttackCursorHoverTexture"));

	if (!m_pTextures[(_int)eCursorState::DEFAULT])
	{
		MSG_BOX("CursorMgr Default Texture Failed");
		return E_FAIL;
	}

	return S_OK;
}

void CCursorMgr::BeginCursorRender()
{
	m_pGraphicDev->GetTransform(D3DTS_VIEW, &m_matOriginView);
	m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &m_matOriginProj);

	_matrix matIdentity;
	D3DXMatrixIdentity(&matIdentity);
	m_pGraphicDev->SetTransform(D3DTS_VIEW, &matIdentity);
	m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matIdentity);

	m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pGraphicDev->SetRenderState(D3DRS_ZENABLE, FALSE);
}

void CCursorMgr::EndCursorRender()
{
	m_pGraphicDev->SetTransform(D3DTS_VIEW, &m_matOriginView);
	m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &m_matOriginProj);

	m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_pGraphicDev->SetRenderState(D3DRS_ZENABLE, TRUE);
}


void CCursorMgr::Free()
{
	//커서 복원
	ShowCursor(TRUE);

	for (int i = 0; i < (_int)eCursorState::CURSOR_END; ++i)
	{
		Safe_Release(m_pTextures[i]);
	}

	Safe_Release(m_pGraphicDev);
	Safe_Release(m_pBufferCom);
}
