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
	//플래그로 이번 프레임에 눌렸는지 아닌지를 판단
	bool bLButton = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);

	//이번 프레임에 눌렸는지 판단 - 이번 프레임
	m_bClickedThisFrame = bLButton && !m_bClicked;
	m_bClicked = bLButton;
	
	if (bLButton)
		m_eCursorState = eCursorState::CLICK;
	else if (m_eCursorState == eCursorState::CLICK)
		m_eCursorState = eCursorState::DEFAULT;

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

void CCursorMgr::GetPickingRay(_vec3& vRayOrigin, _vec3& vRayDir) const
{
	//마우스 커서의 위치에 따른 방향을 넘겨주는 함수
	//스크린 픽셀 -> NDC
	float fNDCX = (2.f * m_vMousePos.x / WINCX) - 1.f;
	float fNDCY = 1.f - (2.f * m_vMousePos.y / WINCY);
	//NDC -> View(투영 역변환)
	_vec3 vRayView;
	vRayView.x = fNDCX / m_matOriginProj._11;
	vRayView.y = fNDCY / m_matOriginProj._22;
	vRayView.z = 1.f;
	//view 공간에서 ray z=1 방향 고정
	_matrix matViewInv;
	D3DXMatrixInverse(&matViewInv, nullptr, &m_matOriginView);
	//Ray 방향 변환
	vRayDir.x = vRayView.x * matViewInv._11 + vRayView.y * matViewInv._21 + vRayView.z * matViewInv._31;
	vRayDir.y = vRayView.x * matViewInv._12 + vRayView.y * matViewInv._22 + vRayView.z * matViewInv._32;
	vRayDir.z = vRayView.x * matViewInv._13 + vRayView.y * matViewInv._23 + vRayView.z * matViewInv._33;
	D3DXVec3Normalize(&vRayDir, &vRayDir);
	//레이 원점 = 카메라 위치(View 역행렬의 이동)
	vRayOrigin.x = matViewInv._41;
	vRayOrigin.y = matViewInv._42;
	vRayOrigin.z = matViewInv._43;
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
