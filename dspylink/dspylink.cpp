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
		\brief Implements a display drive that links Aqsis' sockets based system to the standard dspy system.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<iostream>
#include	<stdlib.h>

#include	"aqsis.h"
#include	"plugins.h"
#include	"ri.h"

#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
#include	<version.h>
#endif

#ifdef AQSIS_SYSTEM_WIN32

#include	<process.h>

#else // AQSIS_SYSTEM_WIN32

typedef int SOCKET;

#endif // !AQSIS_SYSTEM_WIN32

#include	"displaydriver.h"
#include	"dd.h"
#include	"tiffio.h"
#include	"sstring.h"
#include	"dspy.h"

using namespace Aqsis;

/// Static response message for a format query.
SqDDMessageFormatResponse frmt( 2 );
/// Static response message close message.
SqDDMessageCloseAcknowledge closeack;

/** Handle a query message from the manager.
 */
TqInt Query( SOCKET s, SqDDMessageBase* pMsg );
/** Handle an open message from the handler.
 */
TqInt Open( SOCKET s, SqDDMessageBase* pMsg );
/** Handle a data message from the manager.
 */
TqInt Data( SOCKET s, SqDDMessageBase* pMsg );
/** Handle a close message from the manager.
 */
TqInt Close( SOCKET s, SqDDMessageBase* pMsg );
/** Handle an abandon message from the manager.
 */
TqInt Abandon( SOCKET s, SqDDMessageBase* pMsg );

TqInt	XRes, YRes;
TqInt	SamplesPerElement, BitsPerSample;
TIFF*	pOut;
TqInt	g_CWXmin, g_CWYmin;
TqInt	g_CWXmax, g_CWYmax;
uint16	compression = COMPRESSION_NONE, quality = 0;
TqFloat	quantize_zeroval = 0.0f;
TqFloat	quantize_oneval  = 0.0f;
TqFloat	quantize_minval  = 0.0f;
TqFloat	quantize_maxval  = 0.0f;
TqFloat dither_val       = 0.0f;

CqString g_strOpenMethod("DspyImageOpen");
CqString g_strQueryMethod("DspyImageQuery");
CqString g_strDataMethod("DspyImageData");
CqString g_strCloseMethod("DspyImageClose");
CqString g_strDelayCloseMethod("DspyImageDelayClose");

DspyImageOpenMethod			g_OpenMethod = NULL;
DspyImageQueryMethod		g_QueryMethod = NULL;
DspyImageDataMethod			g_DataMethod = NULL;
DspyImageCloseMethod		g_CloseMethod = NULL;
DspyImageDelayCloseMethod	g_DelayCloseMethod = NULL;

void* g_DriverHandle = NULL;
PtDspyImageHandle g_ImageHandle;
PtFlagStuff g_Flags;
CqSimplePlugin g_DspyDriver;
PtDspyDevFormat	g_Formats[1];
CqString g_strName( "" );


/// Main loop,, just cycle handling any recieved messages.
int main( int argc, char* argv[] )
{
	int port = -1;
	char *portStr = getenv( "AQSIS_DD_PORT" );

	if( argc <= 1 )
		return(-1);

	// store the name of the dspy driver requested.
	g_strName = argv[1];

	if ( portStr != NULL )
	{
		port = atoi( portStr );
	}

	if ( DDInitialise( NULL, port ) == 0 )
	{
		DDProcessMessages();
	}
	return 0;
}

/// Storage for the output file name.
std::string	strFilename( "output.tif" );

TqInt Query( SOCKET s, SqDDMessageBase* pMsgB )
{
	switch ( pMsgB->m_MessageID )
	{
			case MessageID_FormatQuery:
			{
				if ( DDSendMsg( s, &frmt ) <= 0 )
					return ( -1 );
			}
			break;
	}
	return ( 0 );
}

TqInt Open( SOCKET s, SqDDMessageBase* pMsgB )
{
	SqDDMessageOpen * pMsg = static_cast<SqDDMessageOpen*>( pMsgB );

	XRes = ( pMsg->m_CropWindowXMax - pMsg->m_CropWindowXMin );
	YRes = ( pMsg->m_CropWindowYMax - pMsg->m_CropWindowYMin );
	SamplesPerElement = pMsg->m_SamplesPerElement;
	BitsPerSample = pMsg->m_BitsPerSample;

	g_CWXmin = pMsg->m_CropWindowXMin;
	g_CWYmin = pMsg->m_CropWindowYMin;
	g_CWXmax = pMsg->m_CropWindowXMax;
	g_CWYmax = pMsg->m_CropWindowYMax;


	g_DriverHandle = g_DspyDriver.SimpleDLOpen( &g_strName );
	if( g_DriverHandle != NULL )
	{
		g_OpenMethod = (DspyImageOpenMethod)g_DspyDriver.SimpleDLSym( g_DriverHandle, &g_strOpenMethod );
		g_QueryMethod = (DspyImageQueryMethod)g_DspyDriver.SimpleDLSym( g_DriverHandle, &g_strQueryMethod );
		g_DataMethod = (DspyImageDataMethod)g_DspyDriver.SimpleDLSym( g_DriverHandle, &g_strDataMethod );
		g_CloseMethod = (DspyImageCloseMethod)g_DspyDriver.SimpleDLSym( g_DriverHandle, &g_strCloseMethod );
		g_DelayCloseMethod = (DspyImageDelayCloseMethod)g_DspyDriver.SimpleDLSym( g_DriverHandle, &g_strDelayCloseMethod );
	}

	if( NULL != g_OpenMethod )
	{
		// \todo: need to pass the proper mode string.
		g_Formats[0].name = "rgba";
		PtDspyError err = (*g_OpenMethod)(&g_ImageHandle, "", strFilename.c_str(), XRes, YRes, 0, NULL, 1, g_Formats, &g_Flags);
		PtDspySizeInfo size;
		err = (*g_QueryMethod)(g_ImageHandle, PkSizeQuery, sizeof(size), &size);
		PtDspyOverwriteInfo owinfo;
		err = (*g_QueryMethod)(g_ImageHandle, PkOverwriteQuery, sizeof(owinfo), &owinfo);
	}

	return ( 0 );
}


TqInt Data( SOCKET s, SqDDMessageBase* pMsgB )
{
	SqDDMessageData * pMsg = static_cast<SqDDMessageData*>( pMsgB );
	SqDDMessageData * const message = static_cast<SqDDMessageData*>( pMsgB );

	TqInt xmin = message->m_XMin;
	TqInt ymin = message->m_YMin;
	TqInt xmaxp1 = message->m_XMaxPlus1;
	TqInt ymaxp1 = message->m_YMaxPlus1;
	TqInt xsize = xmaxp1 - xmin;
	TqInt ysize = ymaxp1 - ymin;

	TqInt	linelen = xsize * SamplesPerElement;
	char* pBucket = reinterpret_cast<char*>( &pMsg->m_Data );


	// CHeck if the beck is not at all within the crop window.
	if ( xmin > g_CWXmax || xmaxp1 < g_CWXmin ||
	     xmin > g_CWYmax || xmaxp1 < g_CWYmin )
		return ( 0 );

	unsigned char* pByteData = NULL;
	float*	pFloatData = NULL;

	if ( g_Formats[0].type == PkDspyUnsigned8 )
		pByteData = new unsigned char[ xsize * ysize * SamplesPerElement ];
	else if( g_Formats[0].type == PkDspyFloat32 )
		pFloatData = new float[ xsize * ysize * SamplesPerElement ];
	else
		return(-1);
	

	TqInt y;
	for ( y = ymin; y < ymaxp1; y++ )
	{
		TqInt x;
		for ( x = xmin; x < xmaxp1; x++ )
		{
			TqInt so = ( ( y - ymin ) * linelen ) + ( ( x - xmin ) * SamplesPerElement );

			TqInt i = 0;
			while ( i < SamplesPerElement )
			{
				TqFloat value = reinterpret_cast<TqFloat*>( pBucket ) [ i ];

				if( !( quantize_zeroval == 0.0f &&
					   quantize_oneval  == 0.0f &&
					   quantize_minval  == 0.0f &&
					   quantize_maxval  == 0.0f ) )
				{
					value = ROUND(quantize_zeroval + value * (quantize_oneval - quantize_zeroval) + dither_val );
					value = CLAMP(value, quantize_minval, quantize_maxval) ;
				}

				if ( g_Formats[0].type == PkDspyUnsigned8  )
				{
					if( BitsPerSample == 8 )
						pByteData[ so++ ] = static_cast<char>( value );
					else
						pByteData[ so++ ] = static_cast<char>( value * 255.0f );
				}
				else
					pFloatData[ so++ ] = value;
				i++;
			}
			pBucket += pMsg->m_ElementSize;
		}
	}

	// Pass the data onto the dspy driver.
	if( NULL != g_DataMethod )
	{
		PtDspyError err;
		if( g_Formats[0].type == PkDspyUnsigned8 )
			err = (*g_DataMethod)(g_ImageHandle, xmin, xmaxp1, ymin, ymaxp1, sizeof(char) * SamplesPerElement, (unsigned char*)pByteData);
		else
			err = (*g_DataMethod)(g_ImageHandle, xmin, xmaxp1, ymin, ymaxp1, sizeof(float) * SamplesPerElement, (unsigned char*)pFloatData);
	}

	delete[] ( pByteData );
	delete[] ( pFloatData );
	return ( 0 );
}


TqInt Close( SOCKET s, SqDDMessageBase* pMsgB )
{
	SqDDMessageClose *pClose = ( SqDDMessageClose * ) pMsgB;

	// Close the dspy display driver.
	if( NULL != g_DelayCloseMethod )
		(*g_DelayCloseMethod)(g_ImageHandle);
	else if( NULL != g_CloseMethod )
		(*g_CloseMethod)(g_ImageHandle);

	if ( DDSendMsg( s, &closeack ) <= 0 )
		return ( -1 );
	else
		return ( 1 );
}


TqInt Abandon( SOCKET s, SqDDMessageBase* pMsgB )
{
	return ( 1 );
}


/** Handle a general message from the manager.
 */
TqInt HandleMessage( SOCKET s, SqDDMessageBase* pMsgB )
{
	switch ( pMsgB->m_MessageID )
	{
			case MessageID_Filename:
			{
				SqDDMessageFilename * pMsg = static_cast<SqDDMessageFilename*>( pMsgB );
				strFilename = pMsg->m_String;
			}
			break;

			case MessageID_UserParam:
			{
				SqDDMessageUserParam * pMsg = static_cast<SqDDMessageUserParam*>( pMsgB );
				// Need to pass these messages through to the dspy driver.
			}
			break;
	}
	return ( 0 );
}
