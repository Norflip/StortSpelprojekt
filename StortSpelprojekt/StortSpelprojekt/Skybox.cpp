#include "Skybox.h"

Skybox::Skybox(ID3D11Device* device, ID3D11DeviceContext* context, Object* object)
{
	this->object = object;

	skyboxShader.SetPixelShader(L"Shaders/Sky_ps.hlsl");
	skyboxShader.SetVertexShader(L"Shaders/Sky_vs.hlsl");
	skyboxShader.Compile(device);

	skyboxMesh = ZWEBLoader::LoadMeshes(ZWEBLoadType::NoAnimation, "Models/Skybox.ZWEB", device)[0];
	skyboxMaterial = Material(skyboxShader);
	
	// Load all textures 
	LoadAllTextures(context, device);

	// Add components
	this->object->AddComponent<MeshComponent>(skyboxMesh, skyboxMaterial);
		
	this->object->GetTransform().SetScale(dx::XMVECTOR(dx::XMVectorSet(20,20,20, 1.0f)));	
}

Skybox::~Skybox()
{
}

void Skybox::LoadAllTextures(ID3D11DeviceContext* context, ID3D11Device* device)
{
	hr = dx::CreateWICTextureFromFile(device, L"Textures/Day.png", nullptr, &srv);
	if (FAILED(hr))	
		MessageBox(0, L"Failed to 'Load DDS Texture' - (skymap.dds).", L"Graphics scene Initialization Message", MB_ICONERROR);
	
	srvs.push_back(srv);
	
	hr = dx::CreateWICTextureFromFile(device, L"Textures/Sunset.png", nullptr, &srv);
	if (FAILED(hr))	
		MessageBox(0, L"Failed to 'Load DDS Texture' - (skymap.dds).", L"Graphics scene Initialization Message", MB_ICONERROR);
	
	srvs.push_back(srv);

	hr = dx::CreateWICTextureFromFile(device, L"Textures/Night.png", nullptr, &srv);
	if (FAILED(hr))
		MessageBox(0, L"Failed to 'Load DDS Texture' - (skymap.dds).", L"Graphics scene Initialization Message", MB_ICONERROR);

	srvs.push_back(srv);

	hr = dx::CreateWICTextureFromFile(device, L"Textures/End.png", nullptr, &srv);
	if (FAILED(hr))
		MessageBox(0, L"Failed to 'Load DDS Texture' - (skymap.dds).", L"Graphics scene Initialization Message", MB_ICONERROR);

	srvs.push_back(srv);
	

	// Set first texture
	texture.SetTexture(srvs[0]);		
	skyboxMaterial.SetTexture(texture, TEXTURE_DIFFUSE2_SLOT, ShaderBindFlag::PIXEL);

	texture2.SetTexture(srvs[1]);
	skyboxMaterial.SetTexture(texture2, TEXTURE_DIFFUSE3_SLOT, ShaderBindFlag::PIXEL);

	texture3.SetTexture(srvs[2]);
	skyboxMaterial.SetTexture(texture3, TEXTURE_DIFFUSE4_SLOT, ShaderBindFlag::PIXEL);

	texture4.SetTexture(srvs[3]);
	skyboxMaterial.SetTexture(texture4, TEXTURE_DIFFUSE5_SLOT, ShaderBindFlag::PIXEL);

	skyboxMaterial.SetSampler(DXHelper::CreateSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, device), 0, ShaderBindFlag::PIXEL);
}