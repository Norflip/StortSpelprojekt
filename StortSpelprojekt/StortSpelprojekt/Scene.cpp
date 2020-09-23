#include "Scene.h"

Scene::Scene() : input(Input::Instance())
{
	

}

Scene::~Scene()
{
}

void Scene::Initialize(Renderer* renderer)
{
	this->renderer = renderer;

	// TEMP
	// Should change values on resize event
	Window* window = renderer->GetOutputWindow();
	
	Object* cameraObject = new Object("camera", ObjectFlag::ENABLED);
	camera = cameraObject->AddComponent<CameraComponent>(60.0f);
	camera->Resize(window->GetWidth(), window->GetHeight());
	move = cameraObject->AddComponent<ControllerComponent>();
	objects.push_back(cameraObject);
	
	Shader shader;
	Shader skeletonShader;
	shader.SetPixelShader(L"Shaders/Default_ps.hlsl");
	shader.SetVertexShader(L"Shaders/Default_vs.hlsl");
	skeletonShader.SetVertexShader(L"Shaders/Skeleton_vs.hlsl");
	
	shader.Compile(renderer->GetDevice());

	
	std::vector<Mesh> zwebMeshes = ZWEBLoader::LoadMeshes(ZWEBLoadType::SkeletonAnimation, "Models/cubeWithTexture.ZWEB", renderer->GetDevice());	
	std::vector<Material> zwebMaterials = ZWEBLoader::LoadMaterials("Models/cubeWithTexture.ZWEB", shader, renderer->GetDevice());
	Object* testMesh = new Object("test");
	dx::XMFLOAT3 miniTranslation = dx::XMFLOAT3(0, 0, 10);
	testMesh->GetTransform().SetPosition(dx::XMLoadFloat3(&miniTranslation));

	//
	zwebMaterials[0].SetSamplerState(renderer->GetDevice(), D3D11_TEXTURE_ADDRESS_WRAP, D3D11_FILTER_MIN_MAG_MIP_LINEAR);
	testMesh->AddComponent<MeshComponent>(zwebMeshes[0], zwebMaterials[0]);
	objects.push_back(testMesh);


	
	

	/* old stuff */

	Mesh mesh = ShittyOBJLoader::Load("Models/Cube.obj", renderer->GetDevice());
	Material material = Material(shader);

	/* Loading a texture */
	Texture diffuseTexture;
	diffuseTexture.LoadTexture(renderer->GetDevice(), L"Textures/Gorilla.png");
	Texture randomNormal;
	randomNormal.LoadTexture(renderer->GetDevice(), L"Textures/RandomNormal.png");

	/* Setting texture to correct slot in material*/
	material.SetTexture(diffuseTexture, TEXTURE_DIFFUSE_SLOT, ShaderBindFlag::PIXEL);
	material.SetTexture(randomNormal, TEXTURE_NORMAL_SLOT, ShaderBindFlag::PIXEL);
	material.SetSamplerState(renderer->GetDevice(), D3D11_TEXTURE_ADDRESS_WRAP, D3D11_FILTER_MIN_MAG_MIP_LINEAR);


	Object* tmp_obj = new Object("cube1");
	tmp_obj->GetTransform().SetPosition({ 0, 0, 10 });
	
	tmp_obj->AddFlag(ObjectFlag::ENABLED | ObjectFlag::RENDER);
	tmp_obj->AddComponent<MeshComponent>(mesh, material);
	objects.push_back(tmp_obj);


	Object* tmp_obj2 = new Object("cube2");
	tmp_obj2->GetTransform().SetPosition({ 0, 0, 4 });

	tmp_obj2->AddFlag(ObjectFlag::ENABLED | ObjectFlag::RENDER);
	tmp_obj2->AddComponent<MeshComponent>(mesh, material);
	tmp_obj2->AddComponent<MoveComponent>();
	
	Transform::SetParentChild(tmp_obj->GetTransform(), tmp_obj2->GetTransform());

	objects.push_back(tmp_obj2);


	/* * * * * * * * ** * * * * */
	/* Render to texture test */	
	screenquadTex = new Texture;
	renderer->RenderToTexture(screenquadTex, renderer->GetDevice(), window->GetWidth(), window->GetHeight());

	/* Screenquad shader */
	Shader screenquadShader;
	screenquadShader.SetPixelShader(L"Shaders/ScreenQuad_ps.hlsl");
	screenquadShader.SetVertexShader(L"Shaders/ScreenQuad_vs.hlsl");
	screenquadShader.Compile(renderer->GetDevice());

	/* Screenquad mat */
	screenquadmat = Material(screenquadShader);

	/* Screenquad mesh */
	Mesh screenquadMesh = Mesh::CreateScreenQuad(renderer->GetDevice());
	screenquadmat.SetTexture(*screenquadTex, TEXTURE_DIFFUSE_SLOT, ShaderBindFlag::PIXEL);
	screenquadmat.SetSamplerState(renderer->GetDevice(), D3D11_TEXTURE_ADDRESS_WRAP, D3D11_FILTER_MIN_MAG_MIP_LINEAR);

	/* Screenquad object */
	quad = new Object("Screenquad");
	quad->AddComponent<MeshComponent>(screenquadMesh, screenquadmat);	

	/* * * * * * * * ** * * * * */


	/* test skybox */
	Shader skyboxShader;
	skyboxShader.SetPixelShader(L"Shaders/Sky_ps.hlsl");
	skyboxShader.SetVertexShader(L"Shaders/Sky_vs.hlsl");
	skyboxShader.Compile(renderer->GetDevice());

	std::vector<Mesh> zwebSkybox = ZWEBLoader::LoadMeshes(ZWEBLoadType::NoAnimation, "Models/skybox.ZWEB", renderer->GetDevice());
	std::vector<Material> zwebSkyboxMaterials = ZWEBLoader::LoadMaterials("Models/skybox.ZWEB", skyboxShader, renderer->GetDevice());
	testSkybox = new Object("skybox");

	dx::XMFLOAT3 skyboxTrans = zwebSkybox[0].GetT();
	dx::XMFLOAT3 skyboxScale = zwebSkybox[0].GetS();

	testSkybox->AddComponent<MeshComponent>(zwebSkybox[0], zwebSkyboxMaterials[0]);
	//objects.push_back(testSkybox);


	//PrintSceneHierarchy();

}

void Scene::Update(const float& deltaTime)
{
	std::vector<Object*> toRemove;
	input.UpdateInputs();


	for (auto i = objects.begin(); i < objects.end(); i++)
	{
		Object* obj = (*i);

		if (obj->HasFlag(ObjectFlag::ENABLED))
			obj->Update(deltaTime);

		if (obj->HasFlag(ObjectFlag::REMOVED))
			toRemove.push_back(obj);
	}
}

void Scene::FixedUpdate(const float& fixedDeltaTime)
{
	//Log::Add(std::to_string(fixedDeltaTime));
}

void Scene::Render()
{	

	renderer->BeginFrame();
	RenderSceneToTexture();

	//for (auto i = objects.begin(); i < objects.end(); i++)
	//{
	//	Object* obj = (*i);
	//	//if (obj->HasFlag(ObjectFlag::ENABLED | ObjectFlag::RENDER))
	//	obj->Draw(renderer, camera);
	//}

	/* Render screenquad with rendered scene-texture */



	quad->Draw(renderer, camera);
	renderer->EndFrame();
}

void Scene::RenderSceneToTexture()
{
	renderer->Unbind();

	renderer->SetRenderTarget(renderer->GetContext(), screenquadTex->GetRtv());
	renderer->ClearRenderTarget(renderer->GetContext(), screenquadTex->GetRtv(), dx::XMFLOAT4(0, 1, 0, 1));

	for (auto i = objects.begin(); i < objects.end(); i++)
	{
		Object* obj = (*i);
		//if (obj->HasFlag(ObjectFlag::ENABLED | ObjectFlag::VISIBLE))
		obj->Draw(renderer, camera);	
	}
	
	testSkybox->Draw(renderer, camera);

	renderer->Unbind();	// needed?

	renderer->SetBackbufferRenderTarget();
}

void Scene::PrintSceneHierarchy() const
{
	Log::Add("PRINTING SCENE HIERARCHY ----");

	for (auto i = objects.cbegin(); i < objects.cend(); i++)
		PrintSceneHierarchy(*i, 0);

	Log::Add("----");
}

void Scene::PrintSceneHierarchy(Object* object, size_t level) const
{
	std::string indent = "";

	if (level > 0)
	{
		for (size_t i = 0; i < level; i++)
			indent += "  ";

		indent += "L  ";		
	}

	
	Log::Add(indent + object->GetName());

	if (object->GetTransform().CountChildren() > 0)
	{
		auto children = object->GetTransform().GetChildren();

		for (size_t i = 0; i < children.size(); i++)
		{
			PrintSceneHierarchy(children[i]->GetOwner(), level + 1);
		}
	}
}
