#pragma once
#include "CBase.h"
#include "Engine_Define.h"

class CLoading : public CBase
{
public:
	enum LOADINGID 
	{ 
		LOADING_STAGE,
		LOADIND_SQUIDCOAST,
		LOADING_CAMP,
		LOADING_REDSTONE,
		LOADING_OBSIDIAN,
		LOADING_END
	};

public:
	explicit CLoading(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CLoading();

public:
	const _tchar* Get_String() { return m_szLoading; }
	LOADINGID		Get_LoadingID() { return m_eLoadingID; }
	_bool			Get_Finish() { return m_bFinish; }
	CRITICAL_SECTION* Get_Crt() { return &m_Crt; }

public:
	HRESULT		Ready_Loading(LOADINGID eID);
	_uint		Loading_Stage();
	_uint Loading_SquidCoast();
	_uint Loading_Camp();
	_uint Loading_RedStone();
	_uint Loading_Obsidian();

public:
	static unsigned int CALLBACK Thread_Main(void* pArg);

private:
	LPDIRECT3DDEVICE9	m_pGraphicDev;
	_tchar				m_szLoading[128];		// 로딩 상태 문자열
	HANDLE				m_hThread;				// 쓰레드 핸들
	LOADINGID			m_eLoadingID;
	CRITICAL_SECTION	m_Crt;					// 동기화 기법 사용 객체
	_bool				m_bFinish;

public:
	static CLoading* Create(LPDIRECT3DDEVICE9 pGraphicDev, LOADINGID eID);

private:
	virtual void	Free();

};



