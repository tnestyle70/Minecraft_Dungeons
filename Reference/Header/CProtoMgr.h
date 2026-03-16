#pragma once
#include "CBase.h"

#include "CTriCol.h"
#include "CRcCol.h"
#include "CRcTex.h"
#include "CTerrainTex.h"
#include "CCubeTex.h"
#include "CCubeBodyTex.h"

// RedStoneGolem
#include "CRedStoneGolemBodyTex.h"
#include "CRedStoneGolemHeadTex.h"
#include "CRedStoneGolemShoulderTex.h"
#include "CRedStoneGolemHipTex.h"
#include "CRedStoneGolemCoreTex.h"
#include "CRedStoneGolemArmTex.h"
#include "CRedStoneGolemLegTex.h"

//Effect
#include "CParticleEmitter.h"

#include "CTransform.h"
#include "CTexture.h"
#include "CCalculator.h"
#include "CCollider.h"

#include "CCamera.h"

BEGIN(Engine)

class ENGINE_DLL CProtoMgr : public CBase
{
	DECLARE_SINGLETON(CProtoMgr)

private:
	CProtoMgr();
	virtual ~CProtoMgr();

public:
	HRESULT		Ready_Prototype(const _tchar* pComponentTag, CComponent* pComponent);
	CComponent* Clone_Prototype(const _tchar* pComponentTag);

private:
	CComponent* Find_Prototype(const _tchar* pComponentTag);

private:
	map<const _tchar*, CComponent*>		m_mapPrototype;

private:
	virtual void	Free();

};

END