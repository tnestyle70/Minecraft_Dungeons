#include "pch.h"
#include "CHotbar.h"
#include "CUI_Json.h"

CHotbar::CHotbar(LPDIRECT3DDEVICE9 pGraphicDev)
	: CUI(pGraphicDev)
{
}

CHotbar::CHotbar(const CHotbar& rhs)
	: CUI(rhs)
{
}

CHotbar::~CHotbar()
{
}

HRESULT CHotbar::Ready_GameObject()
{
	if (FAILED(CUI::Ready_GameObject()))
		return E_FAIL;

	// The Hotbar itself is an empty container at the bottom center
	// Children will be positioned relative to this.
	m_fSizeX = 0.f;
	m_fSizeY = 0.f;

	if (FAILED(Add_Children()))
		return E_FAIL;

	return S_OK;
}

_int CHotbar::Update_GameObject(const _float& fTimeDelta)
{
	return CUI::Update_GameObject(fTimeDelta);
}

void CHotbar::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CUI::LateUpdate_GameObject(fTimeDelta);
}

void CHotbar::Render_GameObject()
{
	CUI::Render_GameObject();
}

HRESULT CHotbar::Add_Children()
{
	struct UIDesc {
		wstring protoTag;
		wstring jsonPath;
		_float x, y;
	};

	// �� �����̽� �ٱ�
	constexpr float outOfWindow = -360.F;

	// ���� UI ����
	vector<UIDesc> childDescs = {
		{ L"Proto_Rocket", L"../Bin/Resource/Texture/UI/Materials/HotBar/rocket.json", outOfWindow, outOfWindow },				// ���� ������ Ȱ ĭ���� ��ġ ��ȯ
		{ L"Proto_ArrowEmpty", L"../Bin/Resource/Texture/UI/Materials/HotBar/arrows_empty.json", outOfWindow, outOfWindow },	// Ȱ ������ ȭ���� ������ Ȱ ĭ���� ��ġ ��ȯ
		{ L"Proto_Arrow", L"../Bin/Resource/Texture/UI/Materials/HotBar/arrow.json", outOfWindow, outOfWindow },				// Ȱ ������ ȭ���� ������ Ȱ ĭ���� ��ġ ��ȯ
		{ L"Proto_IconTNTHUD", L"../Bin/Resource/Texture/UI/Materials/HotBar/icon_TNT_HUD.json", outOfWindow, outOfWindow },	// TNT ������ Ȱ ĭ���� ��ġ ��ȯ

		{ L"Proto_HeartMain", L"../Bin/Resource/Texture/UI/Materials/Hotbar2/Heart/heart_main.json", 588.F, 615.F },			// ��Ʈ ������
		{ L"Proto_FilledHeart", L"../Bin/Resource/Texture/UI/Materials/HotBar2/Heart/filled_heart.json", 597.F, 626.F }			// ��Ʈ (TODO: �ǰݽ� ���� �ؽ��ĺ��� ����ȭ�� �ǰݽ� ü�� �ٴ� �ִϸ��̼� ����)
	};

	// 1. Create and show only the main Hotbar
	CUI_Json* pTestHotbar = CUI_Json::Create(m_pGraphicDev, nullptr, L"Proto_HotbarTexture");
	if (pTestHotbar)
	{
		this->Add_Child(pTestHotbar);
		pTestHotbar->Set_Scale(1.f);
		pTestHotbar->Set_Pos(0.f, 0.f); 
		pTestHotbar->Set_Visible(true);
	}

	for (auto& desc : childDescs)
	{
		CUI_Json* pChild = CUI_Json::Create(m_pGraphicDev, desc.jsonPath.c_str(), desc.protoTag.c_str());
		if (pChild)
		{
			pChild->Set_Pos(desc.x, desc.y);
			pChild->Set_Visible(true);
			this->Add_Child(pChild);
		}
	}

	// 2. Create and show the Test Hotbar LAST so it renders on top
	//CUI_Json* pTestHotbar = CUI_Json::Create(m_pGraphicDev, nullptr, L"Proto_TestHotbarTexture");
	//if (pTestHotbar)
	//{
	//	this->Add_Child(pTestHotbar);
	//	pTestHotbar->Set_Scale(1.f);
	//	pTestHotbar->Set_Pos(0.f, 0.f); 
	//	pTestHotbar->Set_Visible(true);
	//}

	return S_OK;
}

CHotbar* CHotbar::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CHotbar* pInstance = new CHotbar(pGraphicDev);

	if (FAILED(pInstance->Ready_GameObject()))
	{
		Safe_Release(pInstance);
		MSG_BOX("CHotbar Create Failed");
		return nullptr;
	}

	return pInstance;
}

void CHotbar::Free()
{
	for ( auto& child : m_vecChildren ) {
		Safe_Release(child);
		child = nullptr;
	}
	CUI::Free();
}
