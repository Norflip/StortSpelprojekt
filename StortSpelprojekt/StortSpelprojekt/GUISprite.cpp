#include "GUISprite.h"
GUISprite::GUISprite(Renderer& renderer , std::string filePath, float xPos, float yPos)
{
	this->renderer = &renderer;
	// position
	this->xPos = xPos;
	this->yPos = yPos;
	this->position = dx::XMVectorSet(this->xPos, this->yPos, 0, 0);
	//Scale
	this->xScale = 1.0;
	this->yScale = 1.0;
	this->scale = dx::XMVectorSet(this->xScale, this->yScale, 0, 0);
	this->width = 0;
	this->height = 0;
	this->rotation = 0.0f;
	this->color = dx::XMVectorSet(1.f, 1.f, 1.f, 1.f);
	this->origin = dx::XMVectorSet(1.f, 1.f, 1.f, 1.f);
	this->SRV = nullptr;

	this->spriteBatch = nullptr;
	this->filePath = filePath;

	Microsoft::WRL::ComPtr<ID3D11Resource> res;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;

	HRESULT result;
	std::wstring wsConvert(filePath.begin(), filePath.end());
	spriteBatch = std::make_unique<dx::SpriteBatch>(renderer.GetContext());

	result = DirectX::CreateWICTextureFromFile(renderer.GetDevice(), wsConvert.c_str(), &res, &SRV);

	assert(SUCCEEDED(result));
	result = res.As(&tex);

	D3D11_TEXTURE2D_DESC desc;
	tex->GetDesc(&desc);

	width = desc.Width;
	height = desc.Height;
	std::cout << width << "  " << height << std::endl;
	assert(SUCCEEDED(result));
}

GUISprite::~GUISprite()
{
	if (SRV != nullptr) {
		SRV->Release();
		SRV = 0;
	}

	if (spriteBatch != nullptr) {
		spriteBatch = nullptr;
		spriteBatch = 0;
	}
}

void GUISprite::Draw()
{
	spriteBatch->Begin(dx::SpriteSortMode_Immediate);
	spriteBatch->Draw(SRV, this->position, nullptr, this->color, rotation, origin, scale, DirectX::SpriteEffects::SpriteEffects_None, 0.0f);
	spriteBatch->End();
	
}

void GUISprite::SetPosition(float xPos, float yPos)
{
	this->xPos = xPos;
	this->yPos = yPos;
	this->position = dx::XMVectorSet(this->xPos, this->yPos, 0, 0);
}

void GUISprite::SetWICSprite(ID3D11Device* device,std::string spriteFile)
{
	if (this->SRV != nullptr)
		this->SRV->Release();

	HRESULT result;
	std::wstring wsConvert(spriteFile.begin(), spriteFile.end());
	result = DirectX::CreateWICTextureFromFile(device, wsConvert.c_str(), nullptr, &SRV);
	assert(SUCCEEDED(result));
}

void GUISprite::SetDDSSprite(ID3D11Device* device,  std::string spriteFile)
{
	//if (this->SRV != nullptr)
	//	this->SRV->Release();

	//HRESULT result;
	//std::wstring wsConvert(spriteFile.begin(), spriteFile.end());
	//result = DirectX::CreateDDSTextureFromFile(device, wsConvert.c_str(), nullptr, &SRV);
	//assert(SUCCEEDED(result));
}