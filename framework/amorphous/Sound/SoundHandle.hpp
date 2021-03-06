#ifndef  __SoundHandle_H__
#define  __SoundHandle_H__


#include <string>
#include "fwd.hpp"
#include "../Support/prealloc_pool.hpp"
#include "../Support/Serialization/ArchiveObjectBase.hpp"
#include "../Support/Serialization/Archive.hpp"


namespace amorphous
{
using namespace serialization;


/// only for non-streamed sound
/// - Use this as a sound argument of GetSoundManager().Play(), PlayAt(), etc.
/// - cache the pointer to the sound buffer when the sound is played for the first time
///   - Sound manager can skip the search in subsequent calls of GetSoundManager().Play(), PlayAt(), etc.
class SoundHandle : public IArchiveObjectBase
{
	pooled_object_handle m_Handle;

	/// name of a sound
	std::string m_ResourceName;

public:

	SoundHandle() {}

	SoundHandle( const std::string& sound_name )
		:
	m_ResourceName(sound_name)
	{}

	inline const std::string &GetResourceName() const { return m_ResourceName; }

	inline void SetResourceName( const std::string& sound_resource_name )
	{
		m_ResourceName = sound_resource_name;
		m_Handle = pooled_object_handle(); // reset the handle
	}

	void Serialize( IArchive& ar, const unsigned int version )
	{
		ar & m_ResourceName;

		if( ar.GetMode() == IArchive::MODE_INPUT )
		{
			// initialize the handle
			m_Handle = pooled_object_handle();
		}
	}

//	friend class SoundManager;
	friend class SoundManagerImpl;
};

} // namespace amorphous



#endif		/*  __SoundHandle_H__  */
