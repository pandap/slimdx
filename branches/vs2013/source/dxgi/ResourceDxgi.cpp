#include "stdafx.h"
/*
* Copyright (c) 2007-2014 SlimDX Group
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

#include <dxgi.h>

#include "DXGIException.h"

#include "ResourceDxgi.h"

using namespace System;

namespace SlimDX
{
namespace DXGI
{
	Resource::Resource( IComObject^ object )
	{
		IDXGIResource* result = 0;

		IUnknown *ptr = reinterpret_cast<IUnknown*>(object->ComPointer.ToPointer());
		if( RECORD_DXGI( ptr->QueryInterface( IID_IDXGIResource, reinterpret_cast<void**>( &result ) ) ).IsFailure )
			throw gcnew DXGIException( Result::Last );

		ComObject^ other = ObjectTable::Find(IntPtr(result));
		Construct(result, other, other == nullptr);
	}

	ResourcePriority Resource::EvictionPriority::get()
	{
		UINT priority = 0;
		if( RECORD_DXGI( InternalPointer->GetEvictionPriority( &priority ) ).IsFailure )
			return static_cast<ResourcePriority>( 0 );
		
		return static_cast<ResourcePriority>( priority );
	}
	
	void Resource::EvictionPriority::set( ResourcePriority value )
	{
		RECORD_DXGI( InternalPointer->SetEvictionPriority( static_cast<UINT>( value ) ) );
	}
	
	DXGI::Usage Resource::Usage::get()
	{
		DXGI_USAGE usage = 0;
		if( RECORD_DXGI( InternalPointer->GetUsage( &usage ) ).IsFailure )
			return static_cast<DXGI::Usage>( 0 );
		
		return static_cast<DXGI::Usage>( usage );
	}

	IntPtr Resource::SharedHandle::get()
	{
		HANDLE handle;

		HRESULT hr = InternalPointer->GetSharedHandle(&handle);
		if (RECORD_DXGI(hr).IsFailure)
			return IntPtr::Zero;

		return IntPtr(handle);
	}
}
}