#pragma once
#include "CoreMinimal.h"
#include "IInputComm.generated.h"

UINTERFACE(MinimalAPI,meta=(CannotImplementInterfaceInBlueprint))
class UIInputComm : public UInterface
{
	GENERATED_BODY()
};

class IIInputComm
{
	GENERATED_BODY()
public:
	// Inputs for comm from controller
	virtual void I_StartRun()=0;
	virtual void I_StopRun()=0;
	virtual void I_MoveForward(float Value)=0;
	virtual void I_MoveRight(float Value)=0;
	virtual void I_TurnRate(float Value)=0;
	virtual void I_LookUpRate(float Value)=0;
	virtual void I_StartADS()=0;
	virtual void I_StopADS()=0;
	virtual void I_StartCrouch()=0;
	virtual void I_StopCrouch()=0;
	virtual void I_StartFire()=0;
	virtual void I_StopFire()=0;
	virtual void I_Reload()=0;
	virtual void I_ChangeWeapon()=0;
	virtual void I_StartThrow()=0;
	virtual void I_StopThrow()=0;
	virtual void I_Jump()=0;
};