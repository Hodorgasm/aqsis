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
		\brief Interface to a parse tree, used by external backends to output.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef IPARSENODE_H_INCLUDED
#define IPARSENODE_H_INCLUDED 1

#include	<iostream>

#include	"aqsis.h"
#include	"ifuncdef.h"
#include	"ivardef.h"
#include	"sstring.h"

START_NAMESPACE( Aqsis )

enum EqMathOp
{
    Op_Nil = 0,

    Op_Add,
    Op_Sub,
    Op_Mul,
    Op_Div,
    Op_Dot,
    Op_Crs,

    Op_Mod,
    Op_Lft,
    Op_Rgt,
    Op_And,
    Op_Xor,
    Op_Or,
};


enum EqRelOp
{
    Op_EQ = 100,
    Op_NE,
    Op_L,
    Op_G,
    Op_GE,
    Op_LE,
};


enum EqUnaryOp
{
    Op_Plus = 200,
    Op_Neg,
    Op_BitwiseComplement,
    Op_LogicalNot,
};


enum EqLogicalOp
{
    Op_LogAnd = 300,
    Op_LogOr,
};

enum EqTextureType
{
    Type_Texture = 0,
    Type_Environment,
    Type_Bump,
    Type_Shadow,
};

enum EqCommType
{
    CommTypeAtmosphere = 0,
    CommTypeDisplacement,
    CommTypeLightsource,
    CommTypeSurface,
    CommTypeAttribute,
    CommTypeOption,
    CommTypeRendererInfo,
    CommTypeIncident,
    CommTypeOpposite,
    CommTypeTextureInfo
};


enum EqParseNodeType
{
    ParseNode_Base = 0,
    ParseNode_Shader,
    ParseNode_FunctionCall,
    ParseNode_UnresolvedCall,
    ParseNode_Variable,
    ParseNode_ArrayVariable,
    ParseNode_VariableAssign,
    ParseNode_ArrayVariableAssign,
    ParseNode_Operator,
    ParseNode_MathOp,
    ParseNode_RelationalOp,
    ParseNode_UnaryOp,
    ParseNode_LogicalOp,
    ParseNode_DiscardResult,
    ParseNode_ConstantFloat,
    ParseNode_ConstantString,
    ParseNode_WhileConstruct,
    ParseNode_IlluminateConstruct,
    ParseNode_IlluminanceConstruct,
    ParseNode_SolarConstruct,
    ParseNode_Conditional,
    ParseNode_ConditionalExpression,
    ParseNode_TypeCast,
    ParseNode_Triple,
    ParseNode_SixteenTuple,
    ParseNode_MessagePassingFunction,
};


template <class T>
const T* const	QueryNodeType( const T* const pNode, EqParseNodeType type )
{
	if ( T::m_ID == type )
		return ( pNode );
	else
		return ( 0 );
}


struct IqParseNode
{
	virtual	IqParseNode* pChild() const = 0;
	virtual	IqParseNode* pParent() const = 0;
	virtual	IqParseNode* pNextSibling() const = 0;
	virtual	IqParseNode* pPrevSibling() const = 0;
	virtual	TqInt	LineNo() const = 0;
	virtual	const char*	strFileName() const = 0;
	virtual	TqBool	IsVariableRef() const = 0;
	virtual	TqInt	ResType() const = 0;
	virtual	TqBool	fVarying() const = 0;

	virtual	TqBool	GetInterface( EqParseNodeType type, void** pNode ) const = 0;
	virtual	TqInt	NodeType() const = 0;

	const static EqParseNodeType m_ID;
};



struct IqParseNodeShader
{
	virtual	const char*	strName() const = 0;
	virtual	const char*	strShaderType() const = 0;
	virtual	const EqShaderType ShaderType() const = 0;

	const static EqParseNodeType m_ID;
};


struct IqParseNodeFunctionCall
{
	virtual	const char*	strName() const = 0;
	virtual	const IqFuncDef* pFuncDef() const = 0;

	const static EqParseNodeType m_ID;
};


struct IqParseNodeUnresolvedCall
{
	virtual	const char*	strName() const = 0;
	virtual	const IqFuncDef* pFuncDef() const = 0;

	const static EqParseNodeType m_ID;
};


struct IqParseNodeVariable
{
	virtual	const char*	strName() const = 0;
	virtual	SqVarRef	VarRef() const = 0;
	virtual	TqBool	IsLocal() const = 0;

	const static EqParseNodeType m_ID;
};


struct IqParseNodeArrayVariable
{
	const static EqParseNodeType m_ID;
};


struct IqParseNodeVariableAssign
{
	virtual	TqBool fDiscardResult() const = 0;

	const static EqParseNodeType m_ID;
};


struct IqParseNodeArrayVariableAssign
{
	const static EqParseNodeType m_ID;
};


struct IqParseNodeOperator
{
	virtual	TqInt	Operator() const = 0;

	const static EqParseNodeType m_ID;
};


struct IqParseNodeMathOp
{
	const static EqParseNodeType m_ID;
};


struct IqParseNodeRelationalOp
{
	const static EqParseNodeType m_ID;
};


struct IqParseNodeUnaryOp
{
	const static EqParseNodeType m_ID;
};


struct IqParseNodeLogicalOp
{
	const static EqParseNodeType m_ID;
};

struct IqParseNodeDiscardResult
{
	const static EqParseNodeType m_ID;
};

struct IqParseNodeConstantFloat
{
	virtual TqFloat Value() const = 0;

	const static EqParseNodeType m_ID;
};

struct IqParseNodeConstantString
{
	virtual	const char* strValue() const = 0;

	const static EqParseNodeType m_ID;
};


struct IqParseNodeWhileConstruct
{
	const static EqParseNodeType m_ID;
};


struct IqParseNodeIlluminateConstruct
{
	virtual	TqBool	fHasAxisAngle() const = 0;

	const static EqParseNodeType m_ID;
};


struct IqParseNodeIlluminanceConstruct
{
	virtual	TqBool	fHasAxisAngle() const = 0;

	const static EqParseNodeType m_ID;
};


struct IqParseNodeSolarConstruct
{
	virtual	TqBool	fHasAxisAngle() const = 0;

	const static EqParseNodeType m_ID;
};


struct IqParseNodeConditional
{
	const static EqParseNodeType m_ID;
};


struct IqParseNodeConditionalExpression
{
	const static EqParseNodeType m_ID;
};


struct IqParseNodeTypeCast
{
	virtual	TqInt CastTo() const = 0;

	const static EqParseNodeType m_ID;
};


struct IqParseNodeTriple
{
	const static EqParseNodeType m_ID;
};


struct IqParseNodeSixteenTuple
{
	const static EqParseNodeType m_ID;
};

struct IqParseNodeMessagePassingFunction
{
	virtual	SqVarRef	VarRef() const = 0;
	virtual	TqInt	CommType() const = 0;
	virtual CqString Extra() const = 0;

	const static EqParseNodeType m_ID;
};


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !IPARSENODE_H_INCLUDED
