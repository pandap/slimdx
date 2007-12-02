/*
* Copyright (c) 2007 SlimDX Group
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
#include <d3d9.h>
#include <d3dx9.h>

#include "../DirectXObject.h"
#include "../Utils.h"
#include "../DataStream.h"

#include "Device.h"
#include "Texture.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "Mesh.h"
#include "PatchMesh.h"

namespace SlimDX
{
namespace Direct3D9
{
	PatchMesh::PatchMesh( Device^ device, PatchInfo info, int patchCount, int vertexCount, array<VertexElement>^ vertexDeclaration )
	{
		ID3DXPatchMesh *result;
		pin_ptr<VertexElement> pinnedElements = &vertexDeclaration[0];

		HRESULT hr = D3DXCreatePatchMesh( reinterpret_cast<D3DXPATCHINFO*>( &info ), patchCount, vertexCount, 0, 
			reinterpret_cast<D3DVERTEXELEMENT9*>( pinnedElements ), device->InternalPointer, &result );
		GraphicsException::CheckHResult( hr );

		m_Pointer = result;
	}

	PatchMesh::PatchMesh( Mesh^ mesh )
	{
		ID3DXPatchMesh *result;

		HRESULT hr = D3DXCreateNPatchMesh( mesh->MeshPointer, &result );
		GraphicsException::CheckHResult( hr );

		m_Pointer = result;
	}

	PatchMesh^ PatchMesh::FromXFile( Device^ device, XFileData^ xfile, MeshFlags flags, [Out] array<ExtendedMaterial>^% materials,
		[Out] array<EffectInstance>^% effectInstances )
	{
		ID3DXPatchMesh* mesh;
		ID3DXBuffer* materialBuffer;
		ID3DXBuffer* instanceBuffer;
		DWORD materialCount;
		
		HRESULT hr = D3DXLoadPatchMeshFromXof( xfile->InternalPointer, static_cast<DWORD>( flags ), device->InternalPointer,
			&materialBuffer, &instanceBuffer, &materialCount, &mesh );
		GraphicsException::CheckHResult( hr );
		if( FAILED( hr ) )
		{
			materials = nullptr;
			effectInstances = nullptr;
			return nullptr;
		}

		materials = ExtendedMaterial::FromBuffer( materialBuffer, materialCount );

		DWORD instanceCount = 0;
		effectInstances = EffectInstance::FromBuffer( instanceBuffer, instanceCount );

		materialBuffer->Release();
		instanceBuffer->Release();

		return gcnew PatchMesh( mesh );
	}

	PatchMesh^ PatchMesh::FromXFile( Device^ device, XFileData^ xfile, MeshFlags flags, [Out] array<ExtendedMaterial>^% materials )
	{
		ID3DXPatchMesh* mesh;
		ID3DXBuffer* materialBuffer;
		DWORD materialCount;
		
		HRESULT hr = D3DXLoadPatchMeshFromXof( xfile->InternalPointer, static_cast<DWORD>( flags ), device->InternalPointer,
			&materialBuffer, NULL, &materialCount, &mesh );
		GraphicsException::CheckHResult( hr );

		if( FAILED( hr ) )
		{
			materials = nullptr;
			return nullptr;
		}

		materials = ExtendedMaterial::FromBuffer( materialBuffer, materialCount );
		materialBuffer->Release();

		return gcnew PatchMesh( mesh );
	}

	PatchMesh^ PatchMesh::FromXFile( Device^ device, XFileData^ xfile, MeshFlags flags )
	{
		ID3DXPatchMesh* mesh;

		HRESULT hr = D3DXLoadPatchMeshFromXof( xfile->InternalPointer, static_cast<DWORD>( flags ), 
			device->InternalPointer, NULL, NULL, NULL, &mesh );
		GraphicsException::CheckHResult( hr );

		if( FAILED( hr ) )
			return nullptr;

		return gcnew PatchMesh( mesh );
	}

	PatchMesh^ PatchMesh::Clone( MeshFlags flags, array<VertexElement>^ vertexDeclaration )
	{
		ID3DXPatchMesh *result;
		pin_ptr<VertexElement> pinnedElements = &vertexDeclaration[0];

		HRESULT hr = m_Pointer->CloneMesh( static_cast<DWORD>( flags ), reinterpret_cast<D3DVERTEXELEMENT9*>( pinnedElements ), &result );
		GraphicsException::CheckHResult( hr );

		if( FAILED( hr ) )
			return nullptr;

		return gcnew PatchMesh( result );
	}

	void PatchMesh::GenerateAdjacency( float tolerance )
	{
		HRESULT hr = m_Pointer->GenerateAdjacency( tolerance );
		GraphicsException::CheckHResult( hr );
	}

	array<VertexElement>^ PatchMesh::GetDeclaration()
	{
		D3DVERTEXELEMENT9 elementBuffer[MAX_FVF_DECL_SIZE];

		HRESULT hr = m_Pointer->GetDeclaration( elementBuffer );
		GraphicsException::CheckHResult( hr );

		if( FAILED( hr ) )
			return nullptr;

		// Apparently the returned decl does not include an End element. This is bizarre and confusing,
		// not to mention completely unexpected. We patch it up here.
		int count = D3DXGetDeclLength( elementBuffer ) + 1;
		array<VertexElement>^ elements = gcnew array<VertexElement>( count );
		pin_ptr<VertexElement> pinnedElements = &elements[0];
		memcpy( pinnedElements, elementBuffer, count * sizeof(D3DVERTEXELEMENT9) );
		elements[count - 1] = VertexElement::VertexDeclarationEnd;

		return elements;
	}

	Device^ PatchMesh::GetDevice()
	{
		IDirect3DDevice9* device;

		HRESULT hr = m_Pointer->GetDevice( &device );
		GraphicsException::CheckHResult( hr );

		if( FAILED( hr ) )
			return nullptr;

		return gcnew Device( device );
	}

	IndexBuffer^ PatchMesh::GetIndexBuffer()
	{
		IDirect3DIndexBuffer9* ib;

		HRESULT hr = m_Pointer->GetIndexBuffer( &ib );
		GraphicsException::CheckHResult( hr );

		if( FAILED( hr ) )
			return nullptr;

		return gcnew IndexBuffer( ib );
	}

	VertexBuffer^ PatchMesh::GetVertexBuffer()
	{
		IDirect3DVertexBuffer9* vb;

		HRESULT hr = m_Pointer->GetVertexBuffer( &vb );
		GraphicsException::CheckHResult( hr );

		if( FAILED( hr ) )
			return nullptr;

		return gcnew VertexBuffer( vb );
	}

	PatchInfo PatchMesh::GetPatchInfo()
	{
		PatchInfo result;

		HRESULT hr = m_Pointer->GetPatchInfo( reinterpret_cast<D3DXPATCHINFO*>( &result ) );
		GraphicsException::CheckHResult( hr );

		return result;
	}

	void PatchMesh::Optimize()
	{
		HRESULT hr = m_Pointer->Optimize( 0 );
		GraphicsException::CheckHResult( hr );
	}

	DisplacementParameters PatchMesh::GetDisplacementParameters()
	{
		DisplacementParameters result;
		IDirect3DBaseTexture9 *texture;
		D3DTEXTUREFILTERTYPE minFilter;
		D3DTEXTUREFILTERTYPE magFilter;
		D3DTEXTUREFILTERTYPE mipFilter;
		D3DTEXTUREADDRESS wrap;
		DWORD lodBias;

		HRESULT hr = m_Pointer->GetDisplaceParam( &texture, &minFilter, &magFilter, &mipFilter, &wrap, &lodBias );
		GraphicsException::CheckHResult( hr );

		if( FAILED( hr ) )
			return result;

		result.Texture = gcnew Texture( reinterpret_cast<IDirect3DTexture9*>( texture ) );
		result.MinFilter = static_cast<TextureFilter>( minFilter );
		result.MagFilter = static_cast<TextureFilter>( magFilter );
		result.MipFilter = static_cast<TextureFilter>( mipFilter );
		result.Wrap = static_cast<TextureAddress>( wrap );
		result.LevelOfDetailBias = lodBias;

		return result;
	}

	void PatchMesh::SetDisplacementParameters( DisplacementParameters parameters )
	{
		HRESULT hr = m_Pointer->SetDisplaceParam( reinterpret_cast<IDirect3DTexture9*>( parameters.Texture->InternalPointer ), 
			static_cast<D3DTEXTUREFILTERTYPE>( parameters.MinFilter ), static_cast<D3DTEXTUREFILTERTYPE>( parameters.MagFilter ),
			static_cast<D3DTEXTUREFILTERTYPE>( parameters.MipFilter ), static_cast<D3DTEXTUREADDRESS>( parameters.Wrap ), parameters.LevelOfDetailBias );
		GraphicsException::CheckHResult( hr );
	}

	DataStream^ PatchMesh::LockAttributeBuffer( LockFlags flags )
	{
		DWORD *data;
		int faceCount = m_Pointer->GetNumPatches();
		
		HRESULT hr = m_Pointer->LockAttributeBuffer( static_cast<DWORD>( flags ), &data );
		GraphicsException::CheckHResult( hr );

		if( FAILED( hr ) )
			return nullptr;

		bool readOnly = (flags & LockFlags::ReadOnly) == LockFlags::ReadOnly;
		return gcnew DataStream( data, faceCount * sizeof( DWORD ), true, !readOnly, false );
	}

	void PatchMesh::UnlockAttributeBuffer()
	{
		HRESULT hr = m_Pointer->UnlockAttributeBuffer();
		GraphicsException::CheckHResult( hr );
	}

	DataStream^ PatchMesh::LockIndexBuffer( LockFlags flags )
	{
		void *data;
		IDirect3DIndexBuffer9 *indexBuffer;
		
		HRESULT hr = m_Pointer->GetIndexBuffer( &indexBuffer );
		GraphicsException::CheckHResult( hr );
		D3DINDEXBUFFER_DESC desc;
		hr = indexBuffer->GetDesc( &desc );
		GraphicsException::CheckHResult( hr );
		indexBuffer->Release();
		
		hr = m_Pointer->LockIndexBuffer( static_cast<DWORD>( flags ), &data );
		GraphicsException::CheckHResult( hr );

		if( FAILED( hr ) )
			return nullptr;

		bool readOnly = (flags & LockFlags::ReadOnly) == LockFlags::ReadOnly;
		return gcnew DataStream( data, desc.Size, true, !readOnly, false );
	}

	void PatchMesh::UnlockIndexBuffer()
	{
		HRESULT hr = m_Pointer->UnlockIndexBuffer();
		GraphicsException::CheckHResult( hr );
	}

	DataStream^ PatchMesh::LockVertexBuffer( LockFlags flags )
	{
		void *data;
		IDirect3DVertexBuffer9* vertexBuffer;

		HRESULT hr = m_Pointer->GetVertexBuffer( &vertexBuffer );
		GraphicsException::CheckHResult( hr );
		D3DVERTEXBUFFER_DESC desc;
		hr = vertexBuffer->GetDesc( &desc );
		GraphicsException::CheckHResult( hr );
		vertexBuffer->Release();
		
		hr = m_Pointer->LockVertexBuffer( static_cast<DWORD>( flags ), &data );
		GraphicsException::CheckHResult( hr );

		if( FAILED( hr ) )
			return nullptr;

		bool readOnly = (flags & LockFlags::ReadOnly) == LockFlags::ReadOnly;
		return gcnew DataStream( data, desc.Size, true, !readOnly, false );
	}

	void PatchMesh::UnlockVertexBuffer()
	{
		HRESULT hr = m_Pointer->UnlockVertexBuffer();
		GraphicsException::CheckHResult( hr );
	}

	void PatchMesh::GetTessellationSize( float tessellationLevel, bool adaptive, [Out] int% triangleCount, [Out] int% vertexCount )
	{
		pin_ptr<int> pinnedTriCount = &triangleCount;
		pin_ptr<int> pinnedVertexCount = &vertexCount;

		HRESULT hr = m_Pointer->GetTessSize( tessellationLevel, adaptive, reinterpret_cast<DWORD*>( pinnedTriCount ), 
			reinterpret_cast<DWORD*>( pinnedVertexCount ) );
		GraphicsException::CheckHResult( hr );
	}

	void PatchMesh::Tessellate( float tessellationLevel, Mesh^ mesh )
	{
		HRESULT hr = m_Pointer->Tessellate( tessellationLevel, reinterpret_cast<ID3DXMesh*>( mesh->InternalPointer ) );
		GraphicsException::CheckHResult( hr );
	}

	void PatchMesh::Tessellate( Vector4 translation, int minimumLevel, int maximumLevel, Mesh^ mesh )
	{
		HRESULT hr = m_Pointer->TessellateAdaptive( reinterpret_cast<D3DXVECTOR4*>( &translation ), maximumLevel, minimumLevel, 
			reinterpret_cast<ID3DXMesh*>( mesh->InternalPointer ) );
		GraphicsException::CheckHResult( hr );
	}

	int PatchMesh::ControlVerticesPerPatch::get()
	{
		return m_Pointer->GetControlVerticesPerPatch();
	}

	int PatchMesh::PatchCount::get()
	{
		return m_Pointer->GetNumPatches();
	}

	int PatchMesh::VertexCount::get()
	{
		return m_Pointer->GetNumVertices();
	}

	PatchMeshType PatchMesh::Type::get()
	{
		return static_cast<PatchMeshType>( m_Pointer->GetOptions() );
	}
}
}