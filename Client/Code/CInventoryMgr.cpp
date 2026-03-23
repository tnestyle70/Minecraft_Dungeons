#include "pch.h"
#include "CInventoryMgr.h"
#include "CPlayer.h"

IMPLEMENT_SINGLETON(CInventoryMgr)

CInventoryMgr::CInventoryMgr()
{
}

CInventoryMgr::~CInventoryMgr()
{
	Free();
}

HRESULT CInventoryMgr::Ready_InventoryMgr(LPDIRECT3DDEVICE9 pGraphicDev)
{
	m_pGraphicDev = pGraphicDev;
	//InventorySlot들 전부 세팅
	for(int tab = 0; tab < (int)eInventoryTab::INVENTORY_END; tab++)
	{ 
		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				CInventorySlot* pSlot = CInventorySlot::Create(m_pGraphicDev);
				pSlot->Set_SlotInfo(
					500.f + col * 110.f,
					200.f + row * 110.f,
					100.f, 100.f);
				m_vecSlots[tab].push_back(pSlot);
			}
		}
	}
	//탭 버튼 세팅
	for (int tab = 0; tab < (int)eInventoryTab::INVENTORY_END; ++tab)
	{
		m_arrTabButton[tab] = CTabButton::Create(
			m_pGraphicDev, (eInventoryTab)tab,
			500.f + tab * 40.f, 
			100.f,
			30.f, 30.f);
	}
	//초기 활성 탭 표시
	m_arrTabButton[(int)m_eTab]->Set_State(eSlotState::CLICK);

	//장착 장비 세팅
	for (int equip = 0; equip < (int)eEquipType::EQUIP_END; ++equip)
	{
		m_arrEquipSlot[equip] = CEquipSlot::Create(
			m_pGraphicDev, (eEquipType)equip,
			100.f + equip * 70.f,
			100.f,
			70.f, 70.f);
	}

	return S_OK;
}

_int CInventoryMgr::Update(const _float& fTimeDelta)
{
	// 토글 - 항상 체크
	bool bI_Cur = (GetAsyncKeyState('I') & 0x8000) != 0;
	if (bI_Cur && !m_bI_Prev)
		m_bActive = !m_bActive;
	m_bI_Prev = bI_Cur;

	// 활성화됐을 때만 Update
	if (!m_bActive) return 0;

	//맵 버튼 업데이트

	//슬롯 Update
	for (auto& pSlot : m_vecSlots[(int)m_eTab])
		pSlot->Update_GameObject(fTimeDelta);
	//Tab Update
	for (int i = 0; i < (int)eInventoryTab::INVENTORY_END; ++i)
	{
		m_arrTabButton[i]->Update_GameObject(fTimeDelta);
	}
	//장착 슬롯 업데이트
	for (int i = 0; i < (int)eEquipType::EQUIP_END; ++i)
	{
		m_arrEquipSlot[i]->Update_GameObject(fTimeDelta);
	}
	
	Update_TabClick();
	Update_SlotSelection(); //슬롯 선택시 이전 선택 해제, 해당 슬롯 선택
	Update_EquipSlot(); //장착 슬롯을 인벤토리 슬롯과 연동

	return 0;
}

void CInventoryMgr::LateUpdate(const _float& fTimeDelta)
{
	if (!m_bActive)
		return;
	//Slot
	for (auto& pSlot : m_vecSlots[(int)m_eTab])
		pSlot->LateUpdate_GameObject(fTimeDelta);
	//Tab Update
	for (int i = 0; i < (int)eInventoryTab::INVENTORY_END; ++i)
	{
		m_arrTabButton[i]->LateUpdate_GameObject(fTimeDelta);
	}
	//Equip Slot
	for (int i = 0; i < (int)eEquipType::EQUIP_END; ++i)
	{
		m_arrEquipSlot[i]->LateUpdate_GameObject(fTimeDelta);
	}
	return;
}

void CInventoryMgr::Render()
{
	if (!m_bActive)
		return;
	//Slot
	for (auto& pSlot : m_vecSlots[(int)m_eTab])
		pSlot->Render_GameObject();
	//Inventory
	for (int i = 0; i < (int)eInventoryTab::INVENTORY_END; ++i)
	{
		m_arrTabButton[i]->Render_GameObject();
	}
	//Equip Slot
	for (int i = 0; i < (int)eEquipType::EQUIP_END; ++i)
	{
		m_arrEquipSlot[i]->Render_GameObject();
	}
	return;
}

void CInventoryMgr::Update_TabClick()
{
	for (int i = 0; i < (int)eInventoryTab::INVENTORY_END; ++i)
	{
		//이미 활성화된 탭은 스킵
		if ((int)m_eTab == i)
			continue;
		//이번 프레임에 클릭된 탭 버튼을 감지
		if (m_arrTabButton[i]->Get_State() != eSlotState::CLICK)
			continue;
		//이전 활성 탭 버튼을 Default로 복귀
		m_arrTabButton[(int)m_eTab]->Set_State(eSlotState::DEFAULT);
		//선택된 슬롯 초기화
		Clear_ClickedSlot();
		//새 탭으로 전환
		m_eTab = (eInventoryTab)i;
		break;
	}

}

void CInventoryMgr::Update_SlotSelection()
{
	for (auto& pSlot : m_vecSlots[(int)m_eTab])
	{
		//이번 프레임에 Click 상태 슬롯 감지
		if (pSlot->Get_SlotState() != eSlotState::CLICK)
			continue;
		if (pSlot == m_pClickedSlot)
			continue;
		//이전 선택 슬롯 해제
		if (m_pClickedSlot)
			m_pClickedSlot->Set_SlotState(eSlotState::DEFAULT);
		//새 슬롯 등록
		m_pClickedSlot = pSlot;
		return;
	}
}

void CInventoryMgr::Update_EquipSlot()
{
	//인벤토리 탭과 장비 슬롯이 1대1 대응되므로 바로 캐스팅
	const int iMatchEquip = (int)m_eTab;
	CEquipSlot* pMatchSlot = m_arrEquipSlot[iMatchEquip];
	//현재 탭과 무관한 장착 슬롯은 항상 Default 상태로 초기화
	for (int i = 0; i < (int)eEquipType::EQUIP_END; ++i)
	{
		if (i == iMatchEquip)
			continue;
		if (m_arrEquipSlot[i]->Get_State() == eSlotState::CLICK)
		{
			m_arrEquipSlot[i]->Set_State(eSlotState::DEFAULT);
		}
	}
	//현재 탭에 대응하는 장착 슬롯이 이번 프레임에 클릭됐는지 확인
	if (pMatchSlot->Get_State() != eSlotState::CLICK)
		return;

	//클릭 상태 소비? 무슨 의미를 가지는 걸까? 
	pMatchSlot->Set_State(eSlotState::DEFAULT);

	if (m_pClickedSlot != nullptr)
	{
		//인벤토리 슬롯이 선택된 상태 -> 해당 슬롯의 아이템을 장착
		pMatchSlot->Set_Equipped(true);
		Clear_ClickedSlot();
	}
	else if (pMatchSlot->Is_Equipped())
	{
		pMatchSlot->Set_Equipped(false);
	}
}

void CInventoryMgr::Clear_ClickedSlot()
{
	if (m_pClickedSlot)
	{
		m_pClickedSlot->Set_SlotState(eSlotState::DEFAULT);
		m_pClickedSlot = nullptr;
	}
}

void CInventoryMgr::Free()
{
	//인벤토리
	for (int i = 0; i < (int)eInventoryTab::INVENTORY_END; ++i)
	{
		for (auto& pSlot : m_vecSlots[i])
		{
			Safe_Release(pSlot);
		}
	}
	//탭
	for (int i = 0; i < (int)eInventoryTab::INVENTORY_END; ++i)
	{
		Safe_Release(m_arrTabButton[i]);
	}
	//장착
	for (int i = 0; i < (int)eInventoryTab::INVENTORY_END; ++i)
	{
		Safe_Release(m_arrEquipSlot[i]);
	}
	//clickedslot은 이미 해제된 상태
	m_pClickedSlot = nullptr;

	Safe_Release(m_pGraphicDev);
}
