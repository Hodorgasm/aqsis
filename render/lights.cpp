// Aqsis
// Copyright � 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


/** \file
		\brief Implements the classes for handling RenderMan lightsources, plus any built in sources.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"
#include	"lights.h"
#include	"file.h"

START_NAMESPACE( Aqsis )

CqList<CqLightsource>	Lightsource_stack;

//---------------------------------------------------------------------
/** Default constructor.
 */

CqLightsource::CqLightsource( CqShader* pShader, TqBool fActive ) :
		m_pShader( pShader ),
		m_pAttributes( 0 )
{
	// Set a reference with the current attributes.
	m_pAttributes = const_cast<CqAttributes*>( QGetRenderContext() ->pattrCurrent() );
	m_pAttributes->AddRef();

	// Link into the lightsource stack.
	Lightsource_stack.LinkFirst( this );
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqLightsource::~CqLightsource()
{
	// Release our reference on the current attributes.
	if ( m_pAttributes )
		m_pAttributes->Release();
	m_pAttributes = 0;

	// Unlink from the stack.
	UnLink();
}


//---------------------------------------------------------------------
/** Initialise the environment for the specified grid size.
 * \param iGridRes Integer grid resolution.
 * \param iGridRes Integer grid resolution.
 */
void CqLightsource::Initialise( TqInt uGridRes, TqInt vGridRes )
{
	if ( m_pShader )
		m_pShader->Initialise( uGridRes, vGridRes, *this );

	TqInt Uses = gDefLightUses;
	if ( m_pShader ) Uses |= m_pShader->Uses();
	CqShaderExecEnv::Initialise( uGridRes, vGridRes, 0, Uses );

	L()->Initialise( uGridRes, vGridRes );
	Cl()->Initialise( uGridRes, vGridRes );

	// Initialise the geometric parameters in the shader exec env.
	P()->SetValue( CqVMStackEntry( QGetRenderContext() ->matSpaceToSpace( "shader", "current", m_pShader->matCurrent() ) * CqVector3D( 0.0f, 0.0f, 0.0f ) ) );
	if ( USES( Uses, EnvVars_u ) ) u()->SetValue( CqVMStackEntry( 0.0f ) );
	if ( USES( Uses, EnvVars_v ) ) v()->SetValue( CqVMStackEntry( 0.0f ) );
	if ( USES( Uses, EnvVars_du ) ) du()->SetValue( CqVMStackEntry( 0.0f ) );
	if ( USES( Uses, EnvVars_du ) ) dv()->SetValue( CqVMStackEntry( 0.0f ) );
	if ( USES( Uses, EnvVars_s ) ) s()->SetValue( CqVMStackEntry( 0.0f ) );
	if ( USES( Uses, EnvVars_t ) ) t()->SetValue( CqVMStackEntry( 0.0f ) );
	if ( USES( Uses, EnvVars_N ) ) N()->SetValue( CqVMStackEntry( CqVector3D( 0.0f, 0.0f, 0.0f ) ) );
}


//---------------------------------------------------------------------
/** Generate shadow map for this lightsource.
 */

/*void CqLightsource::GenerateShadowMap(const char* strShadowName)
{
	if(m_pShader->fAmbient())	return;
 
	// Store the current renderer state.
	CqOptions	Options=QGetRenderContext()->optCurrent();
	CqImageBuffer* pBuffer=QGetRenderContext()->pImage();
	QGetRenderContext()->SetImage(0);
	CqMatrix	matScreen(QGetRenderContext()->matSpaceToSpace("world","screen"));
	CqMatrix	matNDC(QGetRenderContext()->matSpaceToSpace("world","NDC"));
	CqMatrix	matRaster(QGetRenderContext()->matSpaceToSpace("world","raster"));
	CqMatrix	matCamera(QGetRenderContext()->matSpaceToSpace("world","camera"));
 
	// Get the attributes from the lightsource describing the shadowmap.	
	TqInt ShadowXSize=256;
	TqInt ShadowYSize=256;
	const TqInt* pattrShadowSize=m_pAttributes->GetIntegerAttribute("light","shadowmapsize");
	if(pattrShadowSize!=0)
	{
		ShadowXSize=pattrShadowSize[0];
		ShadowYSize=pattrShadowSize[1];
	}
	
	TqFloat ShadowAngle=90;
	const TqFloat* pattrShadowAngle=m_pAttributes->GetFloatAttribute("light","shadowangle");
	if(pattrShadowAngle!=0)
		ShadowAngle=pattrShadowAngle[0];
 
	
	// Set up the shadow render options through the Ri interfaces.
	RiFormat(ShadowXSize,ShadowYSize,1);
	RiFrameAspectRatio(1);
	RiPixelSamples(1,1);
	RiPixelFilter(RiBoxFilter,1,1);
	RiScreenWindow(-1,1,-1,1);
	RiProjection("perspective","fov",&ShadowAngle,RI_NULL);
	RiDisplay(strShadowName,"shadowmap",RI_Z,RI_NULL);
 
	// Equivalent to RiWorldBegin
	QGetRenderContext()->SetmatCamera(m_matWorldToLight);
	QGetRenderContext()->optCurrent().InitialiseCamera();
 
	QGetRenderContext()->SetfSaveGPrims(TqTrue);
	
	// Equivalent to RiWorldEnd
	QGetRenderContext()->pImage()->SetImage();
	QGetRenderContext()->pImage()->InitialiseSurfaces(QGetRenderContext()->Scene());
	QGetRenderContext()->pImage()->RenderImage();
	RiMakeShadow(strShadowName, strShadowName);
	QGetRenderContext()->pImage()->DeleteImage();
 
	// Restore renderer options.
	QGetRenderContext()->optCurrent()=Options;
	QGetRenderContext()->SetImage(pBuffer);
	QGetRenderContext()->SetmatScreen(matScreen);
	QGetRenderContext()->SetmatNDC(matNDC);
	QGetRenderContext()->SetmatRaster(matRaster);
	QGetRenderContext()->SetmatCamera(matCamera);
 
	QGetRenderContext()->SetfSaveGPrims(TqFalse);
}
*/


//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
// These are the built in shaders, they will be registered as "builtin_<name>"
// these should be used where speed is an issue.

CqShaderLightsourceAmbient::CqShaderLightsourceAmbient() : m_intensity(NULL), m_lightcolor(NULL)
{
	m_intensity = CqShaderVM::CreateVariable(type_float, class_uniform, "intensity" );
	m_lightcolor = CqShaderVM::CreateVariable(type_color, class_uniform, "lightcolor" );

	// Set up the default values for the parameters.
	m_intensity->SetValue( CqVMStackEntry( 1.0f ) );
	m_lightcolor->SetValue( CqVMStackEntry( CqColor(1,1,1) ) );
}


CqShaderLightsourceAmbient::~CqShaderLightsourceAmbient()
{
	delete(m_intensity);
	delete(m_lightcolor);
}


void CqShaderLightsourceAmbient::SetValue( const char* name, TqPchar val )
{
	if ( strcmp( name, "intensity" ) == 0 ) 
		m_intensity->SetValue( CqVMStackEntry( *reinterpret_cast<TqFloat*>( val ) ) );
	else
		if ( strcmp( name, "lightcolor" ) == 0 ) 
			m_lightcolor->SetValue( CqVMStackEntry( CqColor ( reinterpret_cast<TqFloat*>( val ) ) ) );
}


void CqShaderLightsourceAmbient::Evaluate( CqShaderExecEnv& Env )
{
	TqFloat fTemp;
	CqColor cTemp;
	CqVMStackEntry color, intensity;
	
	if(m_lightcolor)
		m_lightcolor->GetValue( 0, color );
	else
		color.SetValue( 0, CqColor(1,1,1) );

	if(m_intensity)
		m_intensity->GetValue( 0, intensity );
	else
		intensity.SetValue( 0, 1.0f );

	color.Value( cTemp );
	intensity.Value( fTemp );
	Env.Cl()->SetValue( CqVMStackEntry( cTemp * fTemp ) );
}



//---------------------------------------------------------------------

END_NAMESPACE( Aqsis )


