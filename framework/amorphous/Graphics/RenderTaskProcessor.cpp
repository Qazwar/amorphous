#include "RenderTaskProcessor.hpp"
#include "RenderTask.hpp"
#include "amorphous/Support/SafeDeleteVector.hpp"


namespace amorphous
{

using namespace std;


singleton<CRenderTaskProcessor> CRenderTaskProcessor::m_obj;


void CRenderTaskProcessor::Release()
{
	SafeDeleteVector( m_vecpTask );
}


void CRenderTaskProcessor::Render()
{
	size_t i, num_tasks = m_vecpTask.size();
	for( i=0; i<num_tasks; i++ )
		m_vecpTask[i]->RenderBase();

	SafeDeleteVector( m_vecpTask );
}



} // namespace amorphous
