#include "pch.h"
#include "CInventoryMgr.h"
#include "CPlayer.h"
#include "CInventoryBackground.h"

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
	m_pGraphicDev->AddRef();
	
	//InventorySlot들 전부 세팅
	for(int tab = 0; tab < (int)eInventoryTab::INVENTORY_END; tab++)
	{
		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				CInventorySlot* pSlot = CInventorySlot::Create(m_pGraphicDev, 
					(eInventoryTab)tab);
				pSlot->Set_SlotInfo(
					500.f + col * 110.f,
					200.f + row * 110.f,
					100.f, 100.f,
					(eInventoryTab)tab);
				bool bEmpty = true;
				//Item Empty값 설정
				if (row == 0 && col == 0)
				{
					bEmpty = false;

					pSlot->Set_ItemInfo(510.f + col * 110.f,
						210.f + row * 110.f,
						80.f, 80.f, bEmpty);
				}
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
			100.f + equip * 90.f,
			100.f,
			80.f, 80.f);
		m_arrEquipSlot[equip]->Set_ItemInfo(
			110.f + equip * 90.f,
			110.f,
			60.f, 60.f);
		//if (equip == 1)
		//	m_arrEquipSlot[equip]->Set_Equipped(false);
	}

	//아이템 패널 세팅
	for (int i = 0; i < (int)eEquipType::EQUIP_END; ++i)
	{
		m_arrItemPanel[i] = CItemPanel::Create(m_pGraphicDev, (eEquipType)i);
		m_arrItemPanel[i]->Set_Selected(false);
	}

	//배경
	m_pInventoryBG = CInventoryBackground::Create(m_pGraphicDev,
		0.f, 0.f, 1280.f, 720.f);
	
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

	//배경 업데이트
	//m_pInventoryBG->Update_GameObject(fTimeDelta);

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
	//아이템 설명창 업데이트
	for (int i = 0; i < (int)eEquipType::EQUIP_END; ++i)
	{
		m_arrItemPanel[i]->Update_GameObject(fTimeDelta);
	}
	
	Update_TabClick();
	Update_SlotSelection(); //슬롯 선택시 이전 선택 해제, 해당 슬롯 선택
	Update_EquipSlot(); //장착 슬롯을 인벤토리 슬롯과 연동
	Update_DoubleClickEquip();

	return 0;
}

void CInventoryMgr::LateUpdate(const _float& fTimeDelta)
{
	if (!m_bActive)
		return;
	//BackGround
	//m_pInventoryBG->LateUpdate_GameObject(fTimeDelta);
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
	//Item Desc
	for (int i = 0; i < (int)eEquipType::EQUIP_END; ++i)
	{
		m_arrItemPanel[i]->LateUpdate_GameObject(fTimeDelta);
	}
	return;
}

void CInventoryMgr::Render()
{
	if (!m_bActive)
		return;
	//BackGround
	//GPU에게 다음에 그려질 텍스쳐 설정 GPU의 0번 소켓에 사용자가 지정한 텍스쳐 m_vecTexture[index]의 주소가 지정
	//위에서 저장한 텍스쳐로 DrawPrimitive 호출시 실질적으로 그리기
	//m_pInventoryBG->Render_GameObject();
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
	//Item Desc
	for (int i = 0; i < (int)eEquipType::EQUIP_END; ++i)
	{
		m_arrItemPanel[i]->Render_GameObject();
	}
	//Player Preview
	Render_PlayerPreview();

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

void CInventoryMgr::Update_DoubleClickEquip()
{
	//더블 클릭시 장착 탭에 장착되게 하기
	for (auto& pSlot : m_vecSlots[(int)m_eTab])
	{
		if (!pSlot->Is_DoubleClicked())
			continue;
		pSlot->Consume_DoubleClick();

		//pSlot->Set_SlotState(eSlotState::DEFAULT);

		//비어있을 경우 클릭 상태로 변경
		if (pSlot->Is_Empty())
			continue;

		//현재 탭 = 장착 슬롯 인덱스
		bool bEquipped = m_arrEquipSlot[(int)m_eTab]->Is_Equipped();
		m_arrEquipSlot[(int)m_eTab]->Set_Equipped(!bEquipped);

		if (bEquipped)
			m_pPlayer->UnEquip((eEquipType)m_eTab);
		else
			m_pPlayer->Equip((eEquipType)m_eTab);

		break;
	}
}

void CInventoryMgr::Update_EquipSlot()
{
	//더블 클릭시 설명창 나오게 하기, 다른 Click 상태의 Equip 버튼들 해제
	//다른 버튼들의 경우 해제를 해야 하는데, for문을 다 돌린다?
	//더블 클릭이 되었을 경우, 다른 버튼들에 대한 선택을 해제한다
	for (int i = 0; i < (int)eEquipType::EQUIP_END; ++i)
	{
		CEquipSlot* pSlot = m_arrEquipSlot[i];

		if (pSlot->Get_State() != eSlotState::CLICK)
			continue;
		
		if (pSlot == m_pEquipSlot)
			continue;
		//이전 선택 해제, 장비창 숨기기 

		if (m_pEquipSlot)
		{
			m_pEquipSlot->Set_State(eSlotState::DEFAULT);
			m_arrItemPanel[(int)m_pEquipSlot->Get_Type()]->Set_Selected(false);
		}

		//장비 설명
		m_arrItemPanel[i]->Set_Selected(true);
		
		m_pEquipSlot = pSlot;
		break;
	}
}

void CInventoryMgr::Render_PlayerPreview()
{
	if (!m_pPlayer) return;

	// 1. 기존 장치 상태(View, Proj, Viewport) 백업
	_matrix matOldView, matOldProj;
	D3DVIEWPORT9 vpOld;
	m_pGraphicDev->GetTransform(D3DTS_VIEW, &matOldView);
	m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matOldProj);
	m_pGraphicDev->GetViewport(&vpOld);

	// 2. 뷰포트 설정 및 Z버퍼 초기화 (이 영역만 새로 그리기 위해)
	D3DVIEWPORT9 vpPreview = { 100, 150, 300, 400, 0.f, 1.f };
	m_pGraphicDev->SetViewport(&vpPreview);
	// 프리뷰 영역의 깊이값만 초기화 (플레이어가 다른 UI 뒤나 앞에 가리는 것 방지)
	m_pGraphicDev->Clear(0, nullptr, D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.f, 0);
	
	// 3. 프리뷰 전용 카메라/투영 설정
	_matrix matView, matProj;
	D3DXMatrixLookAtLH(&matView, 
		&_vec3(0.f, 1.5f, 6.0f), 
		&_vec3(0.f, 1.5f, 0.f),
		&_vec3(0.f, 1.f, 0.f));

	// 종횡비(Aspect Ratio)는 가로/세로 (300/300 = 1.0f)로 맞춤
	D3DXMatrixPerspectiveFovLH(&matProj, D3DXToRadian(45.f), 300.f / 400.f, 
		0.1f, 100.f);

	m_pGraphicDev->SetTransform(D3DTS_VIEW, &matView);
	m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matProj);

	// 4. 플레이어 트랜스폼 임시 변경 및 **월드 행렬 갱신**
	CTransform* pTrans = m_pPlayer->Get_Transform();
	_vec3 vOrigPos, vOrigAngle;
	vOrigPos = pTrans->m_vInfo[INFO_POS]; // 위치 저장
	vOrigAngle = pTrans->m_vAngle;         // 각도 저장

	pTrans->m_vInfo[INFO_POS] = { 0.f, 0.f, 0.f };
	pTrans->m_vAngle.y = 180.f;

	// 중요: Render 전 World Matrix를 강제로 계산해줘야 함
	// (보통 CTransform에 월드 행렬을 갱신하는 public 함수가 있을 겁니다. 예: Update_Component)
	pTrans->Update_Component(0.f);

	// 5. 렌더링
	// 조명 설정이 필요하다면 여기서 일시적으로 조명을 켜주거나 Ambient를 조절해야 보입니다.
	m_pPlayer->Render_GameObject();

	// 6. 상태 원복 (가장 중요)
	pTrans->m_vInfo[INFO_POS] = vOrigPos;
	pTrans->m_vAngle = vOrigAngle;
	pTrans->Update_Component(0.f); // 월드 행렬을 다시 원래 필드 위치로 갱신

	m_pGraphicDev->SetTransform(D3DTS_VIEW, &matOldView);
	m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matOldProj);
	m_pGraphicDev->SetViewport(&vpOld);
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
	for (int i = 0; i < (int)eEquipType::EQUIP_END; ++i)
	{
		Safe_Release(m_arrEquipSlot[i]);
	}
	//아이템 패널
	for (int i = 0; i < (int)eEquipType::EQUIP_END; ++i)
	{
		Safe_Release(m_arrItemPanel[i]);
	}
	
	//clickedslot은 이미 해제된 상태
	m_pClickedSlot = nullptr;
	m_pEquipSlot = nullptr;

	Safe_Release(m_pGraphicDev);
}
