#include "Mesh.h"

Mesh::Mesh(SimpleVertex *vertices, WORD *indices, Dx11Renderer *renderer)
{
	assert (vertices && "Vertices cannot be a null pointer");
	assert (indices && "Indices cannot be a null pointer");
	assert (renderer && "Renderer cannot be a null pointer");

	D3D11_BUFFER_DESC bufferDescription;
	ZeroMemory(&bufferDescription, sizeof(bufferDescription));

	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(SimpleVertex) * 24;
	bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDescription.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));

	initData.pSysMem = vertices;

	Log::log("Creating back buffer", LOG_SEV_INFORMATIONAL);
	if (!SUCCEEDED(renderer->getDevice()->CreateBuffer(&bufferDescription, &initData, &mVertexBuffer)))
		throw std::exception("Failed to create vertex buffer");

	unsigned int stride(sizeof(SimpleVertex));
	unsigned int offset(0);
	
	//Set vertex buffer
	renderer->getDeviceContext()->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);


	bufferDescription.Usage = D3D11_USAGE_DEFAULT;
	bufferDescription.ByteWidth = sizeof(WORD) * 36;
	bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDescription.CPUAccessFlags = 0;

	initData.pSysMem = indices;

	Log::log("Creating index buffer", LOG_SEV_INFORMATIONAL);
	if (!SUCCEEDED(renderer->getDevice()->CreateBuffer(&bufferDescription, &initData, &mIndexBuffer)))
		throw std::exception("Failed to create index buffer.");

	//Set index buffer
	renderer->getDeviceContext()->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
}