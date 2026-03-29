#include "pch.h"
#include "CBlock.h"
#include "CRenderer.h"
#include "CBlockMgr.h"

CBlock::CBlock(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
{
}

CBlock::CBlock(const CGameObject& rhs)
	: CGameObject(rhs)
{
}

CBlock::~CBlock()
{
}

HRESULT CBlock::Ready_GameObject(const _vec3& vPos, eBlockType eType)
{
	m_eType = eType;

	if (FAILED(Add_Component()))
		return E_FAIL;

	switch (eType)
	{
		case BLOCK_IRONBAR:
		{
			m_pTransformCom->m_vScale = { 0.15f, 5.f, 0.15f };
			m_pTransformCom->Set_Pos(vPos.x, vPos.y + 2.f, vPos.z);
			break;
		}
		case BLOCK_JUMPINGTRAP:
		{
			m_pTransformCom->m_vScale = { 5.f, 0.5f, 5.f };
			m_pTransformCom->Set_Pos(vPos.x, vPos.y + 2.f, vPos.z);
			break;
		}
		default:
		{
			m_pTransformCom->m_vScale = { 1.f, 1.f, 1.f };
			m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
			break;
		}
	}
	
	//블럭은 움직이지 않으니 생성 시 한 번만 AABB 세팅
	m_pColliderCom->Update_AABB(vPos);

	return S_OK;
}

_int CBlock::Update_GameObject(const _float& fTimeDelta)
{
	//트랜스폼 때문에 업데이트는 해주되, Add Render Gruop은 해주지 않음
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	//개별 블럭 렌더링용
	if(CBlockMgr::GetInstance()->GetRenderMode() !=  eRenderMode::RENDER_BATCH)
		CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

	return iExit;
}

void CBlock::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CBlock::Render_GameObject()
{
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransformCom->Get_World());

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	m_pTextureCom->Set_Texture(0);

	m_pBufferCom->Render_Buffer();

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	//디버깅용으로 일단 렌더링
	//m_pColliderCom->Render_Collider();
}

HRESULT CBlock::Add_Component()
{
	CComponent* pComponent = nullptr;

	pComponent = m_pBufferCom = dynamic_cast<CCubeTex*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_CubeTex"));
	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

	// Transform
	pComponent = m_pTransformCom = dynamic_cast<CTransform*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

	// Texture
	pComponent = m_pTextureCom = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(GetTextureName()));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

	//Collider
	_vec3 vSize, vOffset;
	vSize = GetColliderSize(m_eType);
	vOffset = GetColliderOffset(m_eType);
	pComponent = m_pColliderCom = CCollider::Create(m_pGraphicDev,
		vSize,
		vOffset);
	if (!pComponent)
		return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_Collider", pComponent });

	return S_OK;
}

HRESULT CBlock::Set_Material()
{
	return E_NOTIMPL;
}

const _tchar* CBlock::GetTextureName()
{
	//타입에 따른 각기 다른 텍스쳐 프로토 반환

	switch (m_eType)
	{
	case BLOCK_GRASS:
		return L"Proto_GrassTexture";
	case BLOCK_DIRT:
		return L"Proto_DirtTexture";
	case BLOCK_ROCK:
		return L"Proto_RockTexture";
	case BLOCK_SAND:
		return L"Proto_SandTexture";
	case BLOCK_BEDROCK:
		return L"Proto_BedrockTexture";
	case BLOCK_OBSIDIAN:
		return L"Proto_ObsidianTexture";
	case BLOCK_STONEBRICK:
		return L"Proto_StoneBrickTexture";
	case BLOCK_IRONBAR:
		return L"Proto_StoneBrickTexture";
	case BLOCK_JUMPINGTRAP:
		return L"Proto_SandTexture";
	case BLOCK_OAK:
		return L"Proto_OakTexture";
	case BLOCK_OAK_LEAVES:
		return L"Proto_OakLeavesTexture";
	case BLOCK_CHERRY_LEAVES:
		return L"Proto_CherryLeavesTexture";
	case BLOCK_LAVA:
		return L"Proto_LavaTexture";
	case BLOCK_PLANKS_ACACIA:
		return L"Proto_PlankAcaciaTexture";
	case BLOCK_PLANKS_SPRUCE:
		return L"Proto_PlankSpruceTexture";
	case BLOCK_OAKWOOD:
		return L"Proto_OakWoodTexture";
	case BLOCK_REDSTONE:
		return L"Proto_RedStoneTexture";
	default:
		break;
	}

	return L"Proto_GrassTexture";
}

_vec3 CBlock::GetColliderSize(eBlockType eType)
{
	_vec3 vSize;

	switch (eType)
	{
	case BLOCK_IRONBAR:
		vSize = { 0.15f, 5.f, 0.15f };
		break;
	case BLOCK_JUMPINGTRAP:
		vSize = { 5.f, 0.15f, 5.f };
		break;
	default:
		vSize = { 1.f, 1.f, 1.f };
		break;
	}

	return vSize;
}

_vec3 CBlock::GetColliderOffset(eBlockType eType)
{
	_vec3 vOffset;

	switch (eType)
	{
	case BLOCK_IRONBAR:
		vOffset = { 0.f, 2.f, 0.f };
		break;
	case BLOCK_JUMPINGTRAP:
		vOffset = { 0.f, 2.f, 0.f };
		break;
	default:
		vOffset = { 0.f, 0.f, 0.f };
		break;
	}

	return vOffset;
}

CBlock* CBlock::Create(LPDIRECT3DDEVICE9 pGraphicDev,
	const _vec3& vPos, eBlockType eType)
{
	CBlock* pBlock = new CBlock(pGraphicDev);

	if (FAILED(pBlock->Ready_GameObject(vPos, eType)))
	{
		Safe_Release(pBlock);
		MSG_BOX("Block Create Failed");
		return nullptr;
	}
	return pBlock;
}

void CBlock::Free()
{
	CGameObject::Free();
}
