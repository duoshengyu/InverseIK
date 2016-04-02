//------------------------------------------------------------------------------
//utility functions
//------------------------------------------------------------------------------

#include <string>
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr.h>
#include <vector>
#ifndef V
#define V(x)           { hr = (x); }
#endif
#ifndef V_RETURN
#define V_RETURN(x)    { hr = (x); if( FAILED(hr) ) { return hr; } }
#endif

#ifndef HR						//easy return error message
#define HR(x)                                      \
	{                                                  \
	HRESULT hr = x;                                \
if (FAILED(hr))                                 \
		{                                              \
		DXTrace(__FILE__, __LINE__, hr, #x, TRUE); \
		}                                              \
	}
#endif         

#ifndef SAFE_DELETE					
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }       //easy to release point resource
#endif							

#ifndef SAFE_RELEASE			
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }     //easy to release COM resource
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif   
