#include "bsCube.h"

void bsCube::create(bsDx11Renderer* renderer, bsShaderManager* shaderManager, bsMeshManager* meshManager)
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout;
	D3D11_INPUT_ELEMENT_DESC d;
	d.SemanticName = "POSITION";
	d.SemanticIndex = 0;
	d.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	d.InputSlot = 0;
	d.AlignedByteOffset = 0;
	d.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	d.InstanceDataStepRate = 0;
	inputLayout.push_back(d);

	d.SemanticName = "COLOR";
	d.AlignedByteOffset = 12;
	inputLayout.push_back(d);

	mVertexShader = shaderManager->getVertexShader("Wireframe.fx", inputLayout);
	
	mPixelShader = shaderManager->getPixelShader("Wireframe.fx");
	
	mMesh = meshManager->getMesh("cube.obj");
}

void bsCube::draw(bsDx11Renderer* renderer)
{
	//This should contain every state change needed to render something properly.
	//Update: probably won't work anymore. Removed a constant buffer.
	auto context = renderer->getDeviceContext();
	
	context->IASetInputLayout(mVertexShader->getInputLayout());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	context->VSSetShader(mVertexShader->getVertexShader(), nullptr, 0);
	context->PSSetShader(mPixelShader->getPixelShader(), nullptr, 0);

	context->VSSetConstantBuffers(2, 1, &mEveryFrame);
	context->PSSetConstantBuffers(2, 1, &mEveryFrame);
	
	mMesh->draw(renderer);
}
