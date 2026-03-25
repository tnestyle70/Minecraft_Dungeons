#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CCollider.h"

class CDLCBoss : public CGameObject
{
protected:
	explicit          CDLCBoss(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit          CDLCBoss(const CGameObject& rhs);
	virtual           ~CDLCBoss();
					  
public:				  
	virtual HRESULT   Ready_GameObject();							   // Add_Component 를 호출해서 컴포넌트 생성 
	virtual _int      Update_GameObject(const _float& fTimeDelta);      // 콜라이더 갱신 피격 체크 
	virtual void      LateUpdate_GameObject(const _float& fTimeDelta); // Update_AI 호출 
					  
	virtual bool	  Is_Dead()  override { return m_iHp <= 0; }       // CLayer::Delete_GameObject 에서 삭제 
	void	          Take_Damage(int iDamage);					       // 피격처리 

	int Get_HP() { return m_iHp; }
	int Get_MaxHP() { return m_iMaxHp; }

protected:			  
	virtual HRESULT   Add_Component() = 0;							   // 텍스처/버퍼/파츠를 파생 클래스에서 생성
	virtual void      Update_AI(const _float& fTimeDelta) = 0;		   // 이동/공격 패턴을 파생 클래스에서 구현
	virtual void      Render_GameObject() = 0;						   // 파츠 렌더링을 파생 클래스에서 구현


protected: 
	Engine::CTransform* m_pTransformCom = nullptr;					   // 위치, 회전, 크키 관리 
	Engine::CCollider* m_pColliderCom = nullptr;					   // 몸통 콜라이더 - 플레이어 공격 피격 판정용 
	Engine::CCollider* m_pAtkColliderCom = nullptr;					   // 공격 콜라이더 - 보스가 플레이어 공격할때 판정용 

	int  m_iHp = 0;
	int  m_iMaxHp = 0;
	float m_fMoveSpeed = 3.f;
	float m_fDetectRange = 10.f; 

	// 사망 완료 플래그 - 파생 클래스에서 true 설정
	bool  m_bDeadDone = false;

	// 사망 모션 - Transform 접근 필요하므로 CDLCBoss에서 관리
	float m_fDeadRotZ = 0.f;
	float m_fDeadVelY = 0.f;
	static constexpr float m_fDeadGravity = -15.f;

protected:
	virtual void Free();											   // 메모리 해제 - 콜라이더 Release 후 CGameObject::Free() 호출

};

