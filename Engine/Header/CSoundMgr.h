#pragma once

#include "CBase.h"
#include "Engine_Define.h"

BEGIN(Engine)

class ENGINE_DLL CSoundMgr : public CBase
{
	DECLARE_SINGLETON(CSoundMgr)

private:
	explicit CSoundMgr();
	virtual ~CSoundMgr();
public:
	void Initialize();
public:
	void PlaySound(const TCHAR* pSoundKey, CHANNELID eID, float fVolume);
	void PlayEffect(const TCHAR* pSoundKey, float fVolume);
	void PlayBGM(const TCHAR* pSoundKey, float fVolume);
	void StopSound(CHANNELID eID);
	void StopAll();
	void SetChannelVolume(CHANNELID eID, float fVolume);

private:
	void LoadSoundFile();
	void LoadSoundFileRecursive(const char* szFolderPath,
		const char* szRelativePath);
private:
	FMOD::System* m_pSystem;
	FMOD::Channel* m_pChannelArr[MAXCHANNEL];
	map<TCHAR*, FMOD::Sound*> m_mapSound;
private:
	virtual void		Free();
};

END