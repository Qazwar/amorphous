#include "ArchiveObjectBase.hpp"
#include "BinaryArchive_Input.hpp"
#include "BinaryArchive_Output.hpp"
#include "CompressedArchive_Input.hpp"
#include "CompressedArchive_Output.hpp"

using namespace amorphous::serialization;

using namespace std;


bool IArchiveObjectBase::SaveToFile( const string& filename, unsigned int archive_option_flags )
{
	CBinaryArchive_Output archive( filename, "", archive_option_flags );
	return ( archive << *this );

//	return true;
}


bool IArchiveObjectBase::LoadFromFile( const string& filename )
{
	CBinaryArchive_Input archive( filename );
	return ( archive >> *this );

//	return true;
}


bool IArchiveObjectBase::SaveToCompressedFile( const std::string& filename )
{
	CCompressedArchive_Output archive( filename );
	return ( archive << *this );
}


bool IArchiveObjectBase::LoadFromCompressedFile( const std::string& filename )
{
	CCompressedArchive_Input archive( filename );
	return ( archive >> *this );
}
