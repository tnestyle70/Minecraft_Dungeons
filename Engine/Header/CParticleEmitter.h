#pragma once
#include "CGameObject.h"
#include "ParticleType.h"

inline DWORD FtoDW(float f)
{
	DWORD dw;
	memcpy(&dw, &f, sizeof(DWORD)); // 안전한 타입 펀닝
	return dw;
}

//ParticleEmittter
//파티클 풀 관리 + 렌더링
//Custom과 Preset으로 관리

BEGIN(Engine)

class ENGINE_DLL CParticleEmitter : public CGameObject
{
private:
	explicit CParticleEmitter(LPDIRECT3DDEVICE9 pGraphicDev);
	//explicit CParticleEmitter(const CParticleEmitter& rhs);
	virtual ~CParticleEmitter();
public:
	//Custom Particle 
	static CParticleEmitter* Create(LPDIRECT3DDEVICE9 pGraphicDev, eParticlePreset& eType,
		const ParticleDesc& desc, LPDIRECT3DTEXTURE9 pTexture);
	//Preset Particle
	static CParticleEmitter* Create(LPDIRECT3DDEVICE9 pGraphicDev,
		eParticlePreset eType, _vec3 vPos, LPDIRECT3DTEXTURE9 pTexture);
public:
	//GameObject Interface
	virtual _int Update_GameObject(const _float& fTimeDelta) override;
	virtual void LateUpdate_GameObject(const _float& fTimeDelta) override;
	virtual void Render_GameObject() override;
public:
	//External Interface
	void Set_Position(_vec3 vPos) { m_tParticleDesc.vEmitPos = vPos; }
	_bool Is_Dead() const { return m_bDead; }
private:
	HRESULT Ready_Emitter(const ParticleDesc& desc, LPDIRECT3DTEXTURE9 pTexture,
		eParticlePreset& ePresetType);

	void Reset_Particle(Particle& particle);
	void Emit_Burst();
	void Emit_ByRate(const _float& fTimeDelta);
	
	void Update_Particles(const _float& fTimeDelta);
	void Render_Particles();
	//Point Sprite Render State Setting and Restore
	void Set_RenderState();
	void Reset_RenderState();
private:
	ParticleDesc m_tParticleDesc;
	vector<Particle> m_vecPool; //고정 크기 파티클 풀
	eParticlePreset m_eParticleType;

	LPDIRECT3DVERTEXBUFFER9 m_pVB; //Dynamic Buffer
	LPDIRECT3DTEXTURE9 m_pTexture; //외부 소유

	_float m_fEmitAccTime; //emit 누적 시간
	_bool m_bBurstDone; //Burst 방식 1회 완료 여부
	_bool m_bDead; //ParticleMgr가 제거 판단
private:
	virtual void Free();
};

END