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

	// Rough layout definitions
	vector<UIDesc> childDescs = {
		{ L"Proto_HotbarBackgroundLeft", L"../Bin/Resource/Texture/UI/Materials/HotBar2/hotbarBackgroundLeft.json", -200.f, -50.f },
		{ L"Proto_HotbarBackgroundMiddle", L"../Bin/Resource/Texture/UI/Materials/HotBar2/hotbarBackgroundMiddle.json", -200.f, -50.f },
		{ L"Proto_HotbarBackgroundMiddleIndent", L"../Bin/Resource/Texture/UI/Materials/HotBar2/hotbarBackgroundMiddleIndent.json", -200.f, -50.f },
		{ L"Proto_HotbarBackgroundRight", L"../Bin/Resource/Texture/UI/Materials/HotBar2/hotbarBackgroundRight.json", -200.f, -50.f },
		{ L"Proto_IndentSlotLeft", L"../Bin/Resource/Texture/UI/Materials/HotBar2/SlotIndent/hotbar_indentslot_left.json", -200.f, -50.f },
		{ L"Proto_IndentSlotCenter", L"../Bin/Resource/Texture/UI/Materials/HotBar2/SlotIndent/hotbar_indentslot_center.json", -150.f, -50.f },
		{ L"Proto_IndentSlotRight", L"../Bin/Resource/Texture/UI/Materials/HotBar2/SlotIndent/hotbar_indentslot_right.json", -100.f, -50.f },
		{ L"Proto_RollingIcon", L"../Bin/Resource/Texture/UI/Materials/HotBar2/SlotIndent/rolling_icon.json", -150.f, -50.f },
		{ L"Proto_SmallSlot", L"../Bin/Resource/Texture/UI/Materials/HotBar2/SlotSmall/smallslot.json", 120.f, -40.f },
		{ L"Proto_IconInventory", L"../Bin/Resource/Texture/UI/Materials/HotBar2/Icons/v2_icon_inventory.json", 125.f, -35.f },
		{ L"Proto_IconMap", L"../Bin/Resource/Texture/UI/Materials/HotBar2/Icons/v2_icon_map.json", -150.f, -50.f },
		{ L"Proto_SquareFrame", L"../Bin/Resource/Texture/UI/Materials/HotBar/square_frame.json", -50.f, -50.f },
		{ L"Proto_Rocket", L"../Bin/Resource/Texture/UI/Materials/HotBar/rocket.json", -50.f, -50.f },
		{ L"Proto_PotionEmpty", L"../Bin/Resource/Texture/UI/Materials/HotBar/potion_empty.json", 0.f, -50.f },
		{ L"Proto_ArrowSlot", L"../Bin/Resource/Texture/UI/Materials/HotBar2/SlotArrow/arrow_slot.json", 50.f, -50.f },
		{ L"Proto_ArrowEmpty", L"../Bin/Resource/Texture/UI/Materials/HotBar/arrows_empty.json", -50.f, -50.f },
		{ L"Proto_Arrow", L"../Bin/Resource/Texture/UI/Materials/HotBar/arrow.json", 0.f, -50.f },
		{ L"Proto_IconTNTHUD", L"../Bin/Resource/Texture/UI/Materials/HotBar/icon_TNT_HUD.json", 50.f, -50.f },
		{ L"Proto_IconEmerald", L"../Bin/Resource/Texture/UI/Materials/HotBar2/Emeralds/icon_emerald.json", 50.f, -50.f },
		{ L"Proto_HeartFrame", L"../Bin/Resource/Texture/UI/Materials/HotBar/heart_frame.json", -350.f, -100.f },
		{ L"Proto_HeartColor", L"../Bin/Resource/Texture/UI/Materials/HotBar/heart_color.json", -350.f, -100.f }
	};

	// 1. Create and show only the Test Hotbar
	CUI_Json* pTestHotbar = CUI_Json::Create(m_pGraphicDev, nullptr, L"Proto_TestHotbarTexture");
	if (pTestHotbar)
	{
		this->Add_Child(pTestHotbar);
		pTestHotbar->Set_Scale(1.f);
		pTestHotbar->Set_Pos(0.f, 0.f); 
		pTestHotbar->Set_Visible(true);
	}

	// 2. Create and hide other elements
	for (auto& desc : childDescs)
	{
		CUI_Json* pChild = CUI_Json::Create(m_pGraphicDev, desc.jsonPath.c_str(), desc.protoTag.c_str());
		if (pChild)
		{
			pChild->Set_Pos(desc.x, desc.y);
			pChild->Set_Visible(false); // Hide for now
			this->Add_Child(pChild);
		}
	}


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
	CUI::Free();
}
