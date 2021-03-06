#ifndef __DIForceFeedbackEffectImpl_HPP__
#define __DIForceFeedbackEffectImpl_HPP__


#include "amorphous/base.hpp"
#include "amorphous/Input/Win32/DirectInput.hpp"
#include "amorphous/Input/Win32/DirectInputGamepad.hpp"
#include "ForceFeedbackEffect.hpp"
#include "ForceFeedbackTargetDevice.hpp"


namespace amorphous
{


class CDIForceFeedbackEffectImpl : public CForceFeedbackEffectImpl
{
	LPDIRECTINPUTEFFECT   m_pEffect;

	LONG m_aDirection[32];

	DIEFFECT m_Effect;

//	CForceFeedbackEffectDesc m_Desc;
	std::shared_ptr<CForceFeedbackEffectDesc> m_pDesc;

	CForceFeedbackTargetDevice m_Target;

	DWORD m_adwAxis[32];

	std::weak_ptr<CDIFFParams> m_pFFParams;

	uint m_ID;

private:

	InputDevice *GetTargetInputDevice();

public:

	CDIForceFeedbackEffectImpl();

//	CDIForceFeedbackEffectImpl( const CForceFeedbackEffectDesc& desc, std::shared_ptr<CDIFFParams> pFFParams );

	~CDIForceFeedbackEffectImpl();

	Result::Name Init( const CForceFeedbackEffectDesc& desc, std::shared_ptr<CDIFFParams> pFFParams );

	Result::Name Init( const CForceFeedbackEffectDesc& desc, const CForceFeedbackTargetDevice& target );

	Result::Name Init( std::shared_ptr<CDIFFParams> pFFParams );

	void Update();

	void SetInfiniteDuration();

	/// Set CDIForceFeedbackEffectImpl::INFINITE_DURATION to set the infinite duration
	void SetDurationMS( int duration_ms );

	void SetDuration( double duration_sec ) { SetDurationMS( (int)( duration_sec * 1000.0 ) ); }

	/// \param gain [in] [0,10000]
	void SetGain( uint gain );

	void SetNumAxes( int num_axes );

	void SetDirection( int dir );

//	Result::Name Start( int target_input_device_group, int iterations, U32 start_flags );
	Result::Name Start( int iterations, U32 start_flags );

	Result::Name Stop();

	uint GetID() const { return m_ID; }

	Result::Name OnInputDevicePlugged();
};


} // namespace amorphous



#endif /* __DIForceFeedbackEffectImpl_HPP__ */
