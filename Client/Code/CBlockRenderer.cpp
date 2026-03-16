#include "pch.h"
#include "CBlockRenderer.h"
#include "CBlockMgr.h"
#include "CRenderer.h"

CBlockRenderer::CBlockRenderer(LPDIRECT3DDEVICE9 pGraphicDev)
	: Engine::CGameObject(pGraphicDev)
{
}

CBlockRenderer::~CBlockRenderer()
{
}

HRESULT CBlockRenderer::Ready_GameObject()
{
	return S_OK;
}

_int CBlockRenderer::Update_GameObject(const _float& fTimeDelta)
{
	Engine::CRenderer::GetInstance()->Add_RenderGroup(Engine::RENDER_PRIORITY, this);
	return 0;
}

void CBlockRenderer::LateUpdate_GameObject(const _float& fTimeDelta)
{
}

void CBlockRenderer::Render_GameObject()
{
	CBlockMgr::GetInstance()->Render();
}

CBlockRenderer* CBlockRenderer::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CBlockRenderer* pInstance = new CBlockRenderer(pGraphicDev);

	if (FAILED(pInstance->Ready_GameObject()))
	{
		Safe_Release(pInstance);
		return nullptr;
	}

	return pInstance;
}

void CBlockRenderer::Free()
{
	Engine::CGameObject::Free();
}
