#pragma once
#include "Shader.h"
#include "Buffers.h"
#include "Texture.h"
#include <vector>
#include <array>

constexpr std::size_t MAX_SHADER_SLOTS = 5;

class Material
{
	static size_t idCounter;

	struct Slot
	{
		ID3D11SamplerState* state;
		Texture texture;
		bool isSet;
	};
	
public:
	Material();
	Material (Shader* shader);
	virtual ~Material();
	
	void SetShader(Shader* shader) { this->shader = shader; }
	void BindToContext(ID3D11DeviceContext*) const;
	
	void SetTexture(Texture texture, size_t slot, ShaderBindFlag flag);
	void SetSampler (ID3D11SamplerState* state, size_t slot, ShaderBindFlag flag);

	void SetMaterialData(const cb_Material& materialData);
	const cb_Material& GetMaterialData() const;

	void ChangeTextureBindFlags(size_t slot, ShaderBindFlag oldFlag, ShaderBindFlag newFlag);

	bool IsTransparent() const { return this->transparent; }
	void SetTransparent(bool transparent) { this->transparent = transparent; }

	void SetName(const std::string& name) { this->name = name; }
	const std::string& GetName() const { return this->name; }

	size_t GetID() const { return this->id; }

private:
	Shader* shader;
	cb_Material cb_material_data;
	size_t id;
	bool transparent;

	std::string name;
	std::unordered_map<int, std::unordered_map<size_t, Texture>> textures;
	std::unordered_map<int, std::unordered_map<size_t, ID3D11SamplerState*>> samplers;
};
