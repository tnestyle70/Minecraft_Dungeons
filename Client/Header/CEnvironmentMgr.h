#pragma once
#include "CBase.h"
#include "CProtoMgr.h"
#include "CBox.h"

class CEnvironmentMgr : public CBase
{
	DECLARE_SINGLETON(CEnvironmentMgr)

private:
	explicit CEnvironmentMgr();
	virtual ~CEnvironmentMgr();

public:
	void Add_Box(CBox* pBox);
	vector<CBox*> Get_Box() { return m_vecBox; }

	void Clear_Boxes();

private:
	LPDIRECT3DDEVICE9 m_pGraphicDev;

	vector<CBox*> m_vecBox;

private:
	virtual void Free();
};