#include "Scene.h"

Scene::Scene() : input(Input::Instance())
{
	

}

Scene::~Scene()
{
	delete root;
	root = nullptr;
}

void Scene::Initialize(Renderer* renderer)
{
	this->renderer = renderer;

	// TEMP
	// Should change values on resize event
	Window* window = renderer->GetOutputWindow();
	
	root = new Object("sceneRoot", ObjectFlag::DEFAULT);

	Object* cameraObject = new Object("camera", ObjectFlag::ENABLED);
	camera = cameraObject->AddComponent<CameraComponent>(60.0f);
	camera->Resize(window->GetWidth(), window->GetHeight());

	Input::Instance().SetWindow(window->GetHWND(), window->GetHeight(), window->GetWidth());
	move = cameraObject->AddComponent<ControllerComponent>();
	AddObject(cameraObject);

	Shader shader;
	Shader skeletonShader;
	shader.SetPixelShader(L"Shaders/Default_ps.hlsl");
	shader.SetVertexShader(L"Shaders/Default_vs.hlsl");
	skeletonShader.SetVertexShader(L"Shaders/Skeleton_vs.hlsl");
	
	shader.Compile(renderer->GetDevice());


	std::vector<Mesh> zwebMeshes = ZWEBLoader::LoadMeshes(ZWEBLoadType::NoAnimation, "Models/brickSphere.ZWEB", renderer->GetDevice());
	std::vector<Material> zwebMaterials = ZWEBLoader::LoadMaterials("Models/brickSphere.ZWEB", shader, renderer->GetDevice());
	
	std::vector<Mesh> sylvanas = ZWEBLoader::LoadMeshes(ZWEBLoadType::NoAnimation, "Models/sylvanas.ZWEB", renderer->GetDevice());
	std::vector<Material> sylvanasMat = ZWEBLoader::LoadMaterials("Models/sylvanas.ZWEB", shader, renderer->GetDevice());

	std::vector<Mesh> cylinder = ZWEBLoader::LoadMeshes(ZWEBLoadType::NoAnimation, "Models/cylinder.ZWEB", renderer->GetDevice());
	std::vector<Material> cylinderMat = ZWEBLoader::LoadMaterials("Models/cylinder.ZWEB", shader, renderer->GetDevice());

	Object* testMesh = new Object("test");
	Object* testMesh2 = new Object("test2");
	Object* testMesh3 = new Object("test3");
	
	dx::XMFLOAT3 miniTranslation = dx::XMFLOAT3(0, 0, 6);
	dx::XMFLOAT3 miniTranslation2 = dx::XMFLOAT3(2, 2, 2);
	dx::XMFLOAT3 miniTranslation3 = dx::XMFLOAT3(-4, -3, -4);

	testMesh->GetTransform().SetPosition(dx::XMLoadFloat3(&miniTranslation));
	
	testMesh2->GetTransform().SetPosition(dx::XMLoadFloat3(&miniTranslation2));
	AddObject(testMesh2, testMesh);


	testMesh3->GetTransform().SetPosition(dx::XMLoadFloat3(&miniTranslation3));
	AddObject(testMesh3, testMesh2);

	testMesh2->AddComponent<MoveComponent>();

	zwebMaterials[0].SetSamplerState(renderer->GetDevice(), D3D11_TEXTURE_ADDRESS_WRAP, D3D11_FILTER_MIN_MAG_MIP_LINEAR);
	sylvanasMat[0].SetSamplerState(renderer->GetDevice(), D3D11_TEXTURE_ADDRESS_WRAP, D3D11_FILTER_MIN_MAG_MIP_LINEAR);
	cylinderMat[0].SetSamplerState(renderer->GetDevice(), D3D11_TEXTURE_ADDRESS_WRAP, D3D11_FILTER_MIN_MAG_MIP_LINEAR);
	
	testMesh->AddComponent<MeshComponent>(zwebMeshes[0], zwebMaterials[0]);
	testMesh2->AddComponent<MeshComponent>(sylvanas[0], sylvanasMat[0]);
	testMesh3->AddComponent<MeshComponent>(cylinder[0], cylinderMat[0]);
	
	AddObject(testMesh);
	//AddObject(testMesh2);
	//AddObject(testMesh3);


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

	/*Object* skybox;
	Skybox* skybox = new Skybox(skybox);*/
	/* test skybox */

	Shader skyboxShader;
	skyboxShader.SetPixelShader(L"Shaders/Sky_ps.hlsl");
	skyboxShader.SetVertexShader(L"Shaders/Sky_vs.hlsl");
	skyboxShader.Compile(renderer->GetDevice());

	std::vector<Mesh> zwebSkybox = ZWEBLoader::LoadMeshes(ZWEBLoadType::NoAnimation, "Models/skybox.ZWEB", renderer->GetDevice());
	std::vector<Material> zwebSkyboxMaterials = ZWEBLoader::LoadMaterials("Models/skybox.ZWEB", skyboxShader, renderer->GetDevice());
	testSkybox = new Object("skybox", ObjectFlag::NO_CULL | ObjectFlag::ENABLED);

	dx::XMFLOAT3 skyboxTrans = zwebSkybox[0].GetT();
	dx::XMFLOAT3 skyboxScale = zwebSkybox[0].GetS();

	testSkybox->AddComponent<MeshComponent>(zwebSkybox[0], zwebSkyboxMaterials[0]);
	testSkybox->AddFlag(ObjectFlag::NO_CULL);

	//AddObject(testSkybox);
	//objects.push_back(testSkybox);
	

	Log::Add("PRINTING SCENE HIERARCHY ----");
	PrintSceneHierarchy(root, 0);
	Log::Add("----");
}

void Scene::Update(const float& deltaTime)
{

	std::vector<Object*> toRemove;


	input.UpdateInputs();
	root->Update(deltaTime);

	//for (auto i = objects.begin(); i < objects.end(); i++)
	//{
	//	Object* obj = (*i);

	//	if (obj->HasFlag(ObjectFlag::ENABLED))
	//		obj->Update(deltaTime);

	//	if (obj->HasFlag(ObjectFlag::REMOVED))
	//		toRemove.push_back(obj);
	//}

	GameClock::Instance().Update();
}

void Scene::FixedUpdate(const float& fixedDeltaTime)
{
	//Log::Add(std::to_string(fixedDeltaTime));
//	root->FixedUpdate(fixedDeltaTime);
}

void Scene::Render()
{	
	
	renderer->BeginFrame();
	renderer->SetBackbufferRenderTarget();

	root->Draw(renderer, camera);

	//auto extractedPlanes = camera->GetFrustumPlanes();

	//for (auto i = objects.begin(); i < objects.end(); i++)
	//{
	//	Object* obj = (*i);
	//	if (obj->HasFlag(ObjectFlag::NO_CULL))
	//	{
	//		obj->Draw(renderer, camera);
	//	}
	//	else
	//	{
	//		dx::XMStoreFloat3(&tempPos, obj->GetTransform().GetPosition());//If object is rotating around something else, then maybe decompose world matrix.
	//		if (!camera->CullAgainstAABB(extractedPlanes, obj->GetComponent<MeshComponent>()->GetBoundingBoxes().GetAABB(), tempPos))
	//		{
	//			obj->Draw(renderer, camera);
	//		}
	//	}
	//}

	testSkybox->Draw(renderer, camera);
	renderer->DrawItemsToTarget();

	renderer->EndFrame();

	//RenderSceneToTexture();

	//for (auto i = objects.begin(); i < objects.end(); i++)
	//{
	//	Object* obj = (*i);
	//	//if (obj->HasFlag(ObjectFlag::ENABLED | ObjectFlag::RENDER))
	//	obj->Draw(renderer, camera);
	//}

	/* Render screenquad with rendered scene-texture */


//	quad->Draw(renderer, camera);
//	renderer->EndFrame();
}

void Scene::RenderSceneToTexture()
{
	//renderer->Unbind();

	//renderer->SetRenderTarget(screenquadTex->GetRtv());
	//renderer->ClearRenderTarget(screenquadTex->GetRtv(), dx::XMFLOAT4(0, 1, 0, 1));

	//camera->GetFrustumPlanes(extractedPlanes);
	//
	//for (auto i = objects.begin(); i < objects.end(); i++)
	//{
	//	
	//	Object* obj = (*i);
	//	if (obj->HasFlag(ObjectFlag::NO_CULL))
	//	{
	//		
	//		obj->Draw(renderer, camera);
	//	}
	//	else
	//	{
	//		dx::XMStoreFloat3(&tempPos, obj->GetTransform().GetPosition());//If object is rotating around something else, then maybe decompose world matrix.
	//		if (!camera->CullAgainstAABB(extractedPlanes, obj->GetComponent<MeshComponent>()->GetBoundingBoxes().GetAABB(), tempPos))
	//		{
	//			obj->Draw(renderer, camera);
	//		}
	//		else
	//		{
	//			//std::cout << "THIS IS NOT VISIBLE";
	//		}
	//		
	//	}
	//		
	//}

	//testSkybox->Draw(renderer, camera);
	//renderer->DrawItemsToTarget();

	//renderer->Unbind();	// needed?
	//renderer->SetBackbufferRenderTarget();
}

void Scene::AddObject(Object* object)
{
	Transform::SetParentChild(root->GetTransform(), object->GetTransform());
}

void Scene::AddObject(Object* object, Object* parent)
{
	Transform::SetParentChild(parent->GetTransform(), object->GetTransform());
}

void Scene::RemoveObject(Object* object)
{
	// remove the the connection and traverse downwards and remove / destroy all objects
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

