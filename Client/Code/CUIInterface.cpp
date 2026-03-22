#include "pch.h"
#include "CUIInterface.h"
#include "CCursorMgr.h"

CUIInterface::CUIInterface(LPDIRECT3DDEVICE9 pGraphicDev)
	:CGameObject(pGraphicDev)
{}

CUIInterface::~CUIInterface()
{}

HRESULT CUIInterface::Ready_GameObject()
{
	return S_OK;
}

_int CUIInterface::Update_GameObject(const _float& fTimeDelta)
{
	int iExit = CGameObject::Update_GameObject(fTimeDelta);

	_bool bInRect = IsMouseInRect();
	_bool bLBtn = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

	// ── 호버 감지 ──────────────────────────────
	if (bInRect && !m_bHovered) { m_bHovered = true;  Hover(); }
	if (!bInRect && m_bHovered) { m_bHovered = false; Leave(); }

	if (bLBtn && bInRect)
	{
		m_bClicked = true;
		Clicked();
	}

	return iExit;
}

void CUIInterface::LateUpdate_GameObject(const _float& fTimeDelta)
{
	return;
}

void CUIInterface::Render_GameObject()
{
	//UI 보여주기 위한 Transform, View, Projection 설정 후 복구
	BeginUIRender();

	m_pTextureCom->Set_Texture(0);

	m_pBufferCom->Render_Buffer();

	EndUIRender();
}

bool CUIInterface::IsMouseInRect()
{
	_vec2 vMouse = GetMousePos();
	//마우스가 Rect 안에 들어왔는지를 검증
	return (vMouse.x >= m_fX && vMouse.x <= m_fX + m_fW
		&& vMouse.y >= m_fY && vMouse.y <= m_fY + m_fH);
}

_vec2 CUIInterface::GetMousePos()
{
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(g_hWnd, &pt);
	return _vec2((float)pt.x, (float)pt.y);
}

void CUIInterface::BeginUIRender()
{
	//월드 행렬 항등 행렬로 설정
	_matrix matWorld;
	D3DXMatrixIdentity(&matWorld);
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
	//원본 뷰, 투영 저장
	m_pGraphicDev->GetTransform(D3DTS_VIEW, &m_matOriginView);
	m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &m_matOriginProj);
	//뷰, 투영 풀어주기
	_matrix matView, matProj;
	D3DXMatrixIdentity(&matView);
	D3DXMatrixIdentity(&matProj);
	m_pGraphicDev->SetTransform(D3DTS_VIEW, &matView);
	m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matProj);
	//CullMode 설정
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	//알파 블렌딩 활성화
	m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHAREF, 0xc0);
}

void CUIInterface::EndUIRender()
{
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	//투영 다시 적용시켜주기
	m_pGraphicDev->SetTransform(D3DTS_VIEW, &m_matOriginView);
	m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &m_matOriginProj);

	//알파블렌딩 - 옵션 다시 꺼주기!!
	m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

void CUIInterface::Set_Info(float fX, float fY, float fW, float fH)
{
	m_fX = fX, m_fY = fY, m_fW = fW, m_fH = fH;
}

void CUIInterface::Free()
{
	CGameObject::Free();
}
