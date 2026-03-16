#pragma once
#include "CDLCBoss.h"  

enum class EPufferFishState
{
    IDLE,   // 대기
    ORBIT,  // 플레이어 주변 선회
    CHARGE, // 플레이어를 향해 돌진
};

class CAncientGuardian : public CDLCBoss
{
 
private:
    explicit CAncientGuardian(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CAncientGuardian();

 
public:
    virtual HRESULT Ready_GameObject();                              
    virtual _int Update_GameObject(const _float fTimeDelta);
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta);

 
protected:
    virtual HRESULT Add_Component()                     override; 
    virtual void    Update_AI(const _float& fTimeDelta) override; 
    virtual void    Render_GameObject()                 override; 

   
private:
    void Update_Orbit(const _float& fTimeDelta);    // 선회 이동
    void Update_Charge(const _float& fTimeDelta);   // 돌진 이동

    void Fire_Beam();                               // 빔 1발 생성 → m_vecBeams에 추가
    void Update_Beams(const _float& fTimeDelta);    // 빔 갱신 + 수명 끝난 것 삭제

    void Drop_Biomine();                            // 바이오마인 1개 생성 → m_vecBiomines에 추가
    void Update_Biomines(const _float& fTimeDelta); // 바이오마인 갱신 + 폭발 끝난 것 삭제

   
private:
    // 선회
    float m_fOrbitRadius = 6.f;  // 플레이어 기준 선회 반지름
    float m_fOrbitAngle = 0.f;  // 현재 선회 각도 (라디안, 매 프레임 증가)
    float m_fOrbitSpeed = 1.f;  // 선회 속도 (라디안/초)
    float m_fOrbitHeight = 4.f;  // 플레이어 Y + 이 값 = 보스 높이

    // 돌진
    _vec3 m_vChargeTarget = {};  // 돌진 목표 위치 (돌진 시작 시점 플레이어 위치 저장)
    float m_fCurSpeed = 0.f; // 현재 속도 (가속/감속)
    float m_fAccel = 10.f;
    float m_fBrake = 8.f;

    // 쿨타임
    float m_fBeamCooldown = 0.f;
    float m_fBeamCoolMax = 3.f; // 3초마다 빔 발사
    float m_fChargeCooldown = 0.f;
    float m_fChargeCoolMax = 5.f; // 5초마다 돌진

    // 현재 상태
    EPufferFishState m_eState = EPufferFishState::IDLE;

  
public:
    static CAncientGuardian* Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos);

protected:
    virtual void Free() override;
};