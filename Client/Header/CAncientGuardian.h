#pragma once
#include "CDLCBoss.h" 
#include "CAGBody.h"
#include "CBeam.h"
#include "CBiomine.h"

enum class EPufferFishState
{
    IDLE,        
    ORBIT,       
    CHARGE,      
    REPOSITION,  
};

class CAncientGuardian : public CDLCBoss
{
private:
    explicit CAncientGuardian(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CAncientGuardian();
public:
    virtual HRESULT Ready_GameObject();
    virtual _int    Update_GameObject(const _float& fTimeDelta);
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta);
public:
    EPufferFishState             Get_State()    { return m_eState; }
    const vector<CBeam*>&        Get_Beams()    const { return m_vecBeams; }
    const vector<CBiomine*>&     Get_Biomines() const { return m_vecBiomines; } 
    virtual bool Is_Dead() override { return m_bDeadDone; }
protected:
    virtual HRESULT Add_Component()                     override; 
    virtual void    Update_AI(const _float& fTimeDelta) override; 
    virtual void    Render_GameObject()                 override; 
private:
    void Update_Orbit(const _float& fTimeDelta);                  
    void Update_Charge(const _float& fTimeDelta);                 
    void Update_Reposition(const _float& fTimeDelta);             
    void Fire_Beam();                                             
    void Update_Beams(const _float& fTimeDelta);                  
    void Drop_Biomine();                                          
    void Update_Biomines(const _float& fTimeDelta);      

private:
    CAGBody* m_pBodyCom = nullptr;              
    Engine::CTexture* m_pTextureCom = nullptr;  

    
    float m_fOrbitRadius = 6.f;                 
    float m_fOrbitAngle = 0.f;                  
    float m_fOrbitSpeed = 1.f;                  
    float m_fOrbitHeight = 0.5f;                

    
    _vec3 m_vChargeTarget = {};                 
    float m_fCurSpeed = 0.f;                    
    float m_fAccel = 30.f;                      
    float m_fBrake = 8.f;                       
    bool  m_bDropped = false;                   

    
    float m_fDropTimer = 0.f;                   
    float m_fDropInterval = 0.3f;               

   
    float m_fChargeCooldown = 5.f;              
    float m_fChargeCoolMax = 5.f;               

    float m_fRepoTimer = 0.f;    
    float m_fRepoMax = 1.5f;     

    float m_fIdleSoundTimer = 0.f;
    static constexpr float m_fIdleSoundInterval = 5.f;


    EPufferFishState m_eState = EPufferFishState::IDLE;
    float            m_fHoverTime = 0.f;                    

private:
    vector<CBeam*>    m_vecBeams;                        
    float m_fFireTimer = 0.f;                            
    float m_fFireInterval = 0.1f;                        
    int   m_iFireCount = 0;                              
    int   m_iFireMax = 10;                               
    bool  m_bFiring = false;                             

    vector<CBiomine*> m_vecBiomines;                     

public:
    static CAncientGuardian* Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos);
protected:
    virtual void Free() override;
};