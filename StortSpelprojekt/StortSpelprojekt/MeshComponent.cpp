#include "MeshComponent.h"

MeshComponent::MeshComponent(Mesh mesh, Material material) : mesh(mesh), material(material), texture(material.GetTexture()),boundingBoxes(mesh) {}
MeshComponent::~MeshComponent() {}

void MeshComponent::Update(const float& deltaTime)
{
	float rotationDegree = 0.4f * deltaTime;
	GetOwner()->GetTransform().Rotate(0.0f, rotationDegree, 0.0f);
}

void MeshComponent::Draw(Renderer* renderer, CameraComponent* camera, DrawType drawType)
{
	if (drawType == DrawType::STANDARD)
	{
		renderer->GetContext()->VSSetShader(0, 0, 0);
		renderer->GetContext()->PSSetShader(0, 0, 0);
		material.BindToContext(renderer->GetContext());
		renderer->Draw(this->mesh, this->material.GetMaterialData(), GetOwner()->GetTransform().GetWorldMatrix(), camera->GetViewMatrix(), camera->GetProjectionMatrix(), camera->GetOwner()->GetTransform().GetPosition());
	}
	else if (drawType == DrawType::INSTANCED)
	{
		renderer->GetContext()->VSSetShader(0, 0, 0);
		renderer->GetContext()->PSSetShader(0, 0, 0);
		material.BindToContext(renderer->GetContext());
		renderer->DrawInstanced(this->mesh, this->material.GetMaterialData(), mesh.GetInstanceNr(), camera->GetViewMatrix(), camera->GetProjectionMatrix(), camera->GetOwner()->GetTransform().GetPosition());
	}
	else if (drawType == DrawType::INSTANCEDALPHA)
	{
		renderer->GetContext()->VSSetShader(0, 0, 0);
		renderer->GetContext()->PSSetShader(0, 0, 0);
		material.BindToContext(renderer->GetContext());
		renderer->DrawAlphaInstanced(this->mesh, this->material.GetMaterialData(), mesh.GetInstanceNr(), camera->GetViewMatrix(), camera->GetProjectionMatrix(), camera->GetOwner()->GetTransform().GetPosition());
	}
}


