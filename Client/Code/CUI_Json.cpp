#include "pch.h"
#include "CUI_Json.h"
#include "CRcTex.h"
#include "CTexture.h"
#include "CTransform.h"
#include "CProtoMgr.h"
#include "CRenderer.h"

#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

CUI_Json::CUI_Json(LPDIRECT3DDEVICE9 pGraphicDev)
	: CUI(pGraphicDev)
{
}

CUI_Json::CUI_Json(const CUI_Json& rhs)
	: CUI(rhs)
	, m_iSizeX(rhs.m_iSizeX)
	, m_iSizeY(rhs.m_iSizeY)
{
}

CUI_Json::~CUI_Json()
{
}

HRESULT CUI_Json::Ready_GameObject(const _tchar* pJsonPath, const _tchar* pProtoTextureTag)
{
	if ( FAILED(CUI::Ready_GameObject()) ) {
		MSG_BOX("CUI::Ready_GameObject FAIL");
		return E_FAIL;
	}

	if (pJsonPath)
	{
		if (FAILED(Load_Json(pJsonPath))) {
			MSG_BOX("Load_Json FAIL");
			return E_FAIL;
		}
	}
	else
	{
		m_iSizeX = WINCX;
		m_iSizeY = WINCY;
	}

	if ( FAILED(Add_Component(pProtoTextureTag)) ) {
		MSG_BOX("Add_Component FAIL");
		return E_FAIL;
	}

	m_fX = 0.f;
	m_fY = 0.f;
	m_fSizeX = (_float)m_iSizeX;
	m_fSizeY = (_float)m_iSizeY;

	return S_OK;
}

_int CUI_Json::Update_GameObject(const _float& fTimeDelta)
{
	return CUI::Update_GameObject(fTimeDelta);
}

void CUI_Json::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CUI::LateUpdate_GameObject(fTimeDelta);
}

void CUI_Json::Render_GameObject()
{
	CUI::Render_GameObject();
}

HRESULT CUI_Json::Load_Json(const _tchar* pJsonPath)
{
	// Convert _tchar to string for ifstream
	wstring ws(pJsonPath);
	string sPath(ws.begin(), ws.end());

	ifstream file(sPath);
	if (!file.is_open())
		return E_FAIL;

	json j;
	file >> j;

	// The JSON provided by the user is an array: [ { ... } ]
	if (j.is_array() && !j.empty())
	{
		auto& firstElement = j[0];
		
		_int iImportedX = 0;
		_int iImportedY = 0;
		_int iSizeX = 0;
		_int iSizeY = 0;

		if (firstElement.contains("Properties") && firstElement["Properties"].contains("ImportedSize"))
		{
			iImportedX = firstElement["Properties"]["ImportedSize"]["X"];
			iImportedY = firstElement["Properties"]["ImportedSize"]["Y"];
		}

		if (firstElement.contains("SizeX"))
			iSizeX = firstElement["SizeX"];
		if (firstElement.contains("SizeY"))
			iSizeY = firstElement["SizeY"];

		// If ImportedSize is missing, fallback to SizeX/SizeY
		if (iImportedX == 0) iImportedX = iSizeX;
		if (iImportedY == 0) iImportedY = iSizeY;

		m_iSizeX = iImportedX;
		m_iSizeY = iImportedY;

		if (iSizeX > 0 && iSizeY > 0)
		{
			m_vUVScale.x = (_float)iImportedX / (_float)iSizeX;
			m_vUVScale.y = (_float)iImportedY / (_float)iSizeY;
		}
	}

	return S_OK;
}

HRESULT CUI_Json::Add_Component(const _tchar* pProtoTextureTag)
{
	Engine::CComponent* pComponent = nullptr;

	pComponent = m_pTextureCom = dynamic_cast<Engine::CTexture*>(Engine::CProtoMgr::GetInstance()->Clone_Prototype(pProtoTextureTag));
	if (nullptr == pComponent) return E_FAIL;
	m_mapComponent[Engine::ID_STATIC].insert({ L"Com_Texture", pComponent });

	return S_OK;
}

CUI_Json* CUI_Json::Create(LPDIRECT3DDEVICE9 pGraphicDev, const _tchar* pJsonPath, const _tchar* pProtoTextureTag)
{
	CUI_Json* pInstance = new CUI_Json(pGraphicDev);

	if (FAILED(pInstance->Ready_GameObject(pJsonPath, pProtoTextureTag)))
	{
		Safe_Release(pInstance);
		MSG_BOX("CUI_Json Create Failed");
		return nullptr;
	}

	return pInstance;
}

void CUI_Json::Free()
{
	CUI::Free();
}
