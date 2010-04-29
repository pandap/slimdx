#include "stdafx.h"
/*
* Copyright (c) 2007-2010 SlimDX Group
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#include <d3d10.h>

#include "Direct3D10Exception.h"

#include "EffectResourceVariable.h"
#include "ShaderResourceView.h"

using namespace System;

namespace SlimDX
{
namespace Direct3D10
{ 
	EffectResourceVariable::EffectResourceVariable( ID3D10EffectShaderResourceVariable* pointer )
	: EffectVariable( pointer )
	{
		m_Pointer = pointer;
	}
	
	EffectResourceVariable::EffectResourceVariable( IntPtr pointer )
	: EffectVariable( pointer )
	{
		m_Pointer = reinterpret_cast<ID3D10EffectShaderResourceVariable*>( pointer.ToPointer() );
	}
	
	Result EffectResourceVariable::SetResource( ShaderResourceView^ view )
	{
		if( view == nullptr )
		{
			return RECORD_D3D10( m_Pointer->SetResource( 0 ) );
		}
		else
		{
			return RECORD_D3D10( m_Pointer->SetResource( static_cast<ID3D10ShaderResourceView*>( view->InternalPointer ) ) );
		}
	}
	
	ShaderResourceView^ EffectResourceVariable::GetResource()
	{
		ID3D10ShaderResourceView* view = 0;
		if( RECORD_D3D10( m_Pointer->GetResource( &view ) ).IsFailure )
			return nullptr;
			
		return ShaderResourceView::FromPointer( view );
	}
}
}