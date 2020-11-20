#include "stdafx.h"
#include "GameScene.h"
#include "GUISprite.h"
#include "GUIFont.h"
#include "Engine.h"
#include "GUICompass.h"
void GameScene::RemoveEnemy()
{
	enemy->RemoveFlag(ObjectFlag::ENABLED);
	enemy->AddFlag(ObjectFlag::REMOVED);
	srand(time(0));
	fogId = 0;
	fogCol = 0;
}

GameScene::GameScene()
{
	this->interiorPosition = { 0.0f, -100.0f, 0.0f };
}

GameScene::~GameScene()
{
}

void GameScene::Initialize()
{
	pooler->Register("test_body_cube", 10, [](ResourceManager* resources) {

		Object* object = new Object("physics_cube");

		Mesh* mesh1 = resources->GetResource<Mesh>("Test");
		Material* material1 = resources->GetResource<Material>("TestMaterial");
		
		object->AddComponent<MeshComponent>(mesh1, material1);
		object->AddComponent<BoxColliderComponent>(dx::XMFLOAT3(0.5f, 0.5f, 0.5f), dx::XMFLOAT3(0, 0, 0));
		object->AddComponent<RigidBodyComponent>(10.0f, FilterGroups::DEFAULT, FilterGroups::EVERYTHING, BodyType::DYNAMIC, true);
		return object;
	});

	InitializeGUI();
	InitializeObjects();
	InitializeInterior();
}

void GameScene::InitializeObjects()
{
	Object* houseBaseObject = new Object("houseBase");
	Object* housesLegsObject = new Object("houseLegs");
	Object* houseDoor = resources->AssembleObject("HouseDoor", "HouseDoorMaterial");
	Object* houseExterior = resources->AssembleObject("HouseExterior", "HouseExteriorMaterial");

	houseBaseObject->GetTransform().Rotate(0, -90.0f * Math::ToRadians, 0.0);

	house = houseBaseObject;

	SkeletonMeshComponent* baseComponent = resources->GetResource<SkeletonMeshComponent>("HouseSkeleton");
	SkeletonMeshComponent* legsComponent = resources->GetResource<SkeletonMeshComponent>("HouseLegsSkeleton");

	houseBaseObject->GetTransform().SetScale({ 0.5f, 0.5f, 0.5f });

	//WALLS
	houseBaseObject->AddComponent<BoxColliderComponent>(dx::XMFLOAT3(2.0f, 3.5f, 2.3f), dx::XMFLOAT3(0.f, 0.9f, -1.0f));
	//PORCH
	houseBaseObject->AddComponent<BoxColliderComponent>(dx::XMFLOAT3(3.0f, 0.5f, 3.0f), dx::XMFLOAT3(0.f, 0.2f, 0.f));
	//FENCE BACK
	houseBaseObject->AddComponent<BoxColliderComponent>(dx::XMFLOAT3(0.125f, 0.625f, 3.375f), dx::XMFLOAT3(-3.3f, 2.f, 0.05f));
	//FENCE FRONT
	houseBaseObject->AddComponent<BoxColliderComponent>(dx::XMFLOAT3(0.1f, 3.5f, 1.8f), dx::XMFLOAT3(3.5f, 0.7f, 0.5f));
	//FENCE RIGHT
	houseBaseObject->AddComponent<BoxColliderComponent>(dx::XMFLOAT3(3.f, 0.625f, 0.1f), dx::XMFLOAT3(0.f, 3.f, -3.f));
	//FENCE LEFT
	houseBaseObject->AddComponent<BoxColliderComponent>(dx::XMFLOAT3(5.375f, 0.625f, 0.15f), dx::XMFLOAT3(0.f, 3.f, 3.f));
	// RB
	houseBaseObject->AddComponent<RigidBodyComponent>(0.0f, FilterGroups::PROPS, FilterGroups::EVERYTHING, BodyType::STATIC, true);

	baseComponent->SetTimeScale(1.0f);
	legsComponent->SetTimeScale(1.0f);
	baseComponent->SetTrack(SkeletonStateMachine::IDLE, false);
	legsComponent->SetTrack(SkeletonStateMachine::IDLE, false);

	houseBaseObject->AddComponent<SkeletonMeshComponent>(baseComponent);
	housesLegsObject->AddComponent<SkeletonMeshComponent>(legsComponent);
	
	Transform::SetParentChild(houseBaseObject->GetTransform(), housesLegsObject->GetTransform());

	nodeWalker = houseBaseObject->AddComponent<NodeWalkerComp>();
	nodeWalker->InitAnimation();
	AddObject(houseBaseObject);

	houseDoor->AddComponent<HousePartsComponent>(houseBaseObject->GetComponent<SkeletonMeshComponent>());
	houseExterior->AddComponent<HousePartsComponent>(houseBaseObject->GetComponent<SkeletonMeshComponent>());

	AddObject(houseDoor);
	AddObject(houseExterior);

	//Player & Camera
	dx::XMFLOAT3 playerSpawn = { 10,2,10 };
	dx::XMFLOAT3 zero = { 0.f, 0.f, 0.f };
	dx::XMVECTOR playerSpawnVec = dx::XMLoadFloat3(&playerSpawn);
	Object* playerObject = new Object("player", ObjectFlag::ENABLED);
	Object* cameraObject = new Object("camera", ObjectFlag::ENABLED);
	this->player = playerObject;
	camera = cameraObject->AddComponent<CameraComponent>(window->GetWidth(), window->GetHeight(), 60.0f);

	Shader forwardPlusShader;// = 

	forwardPlusShader.SetComputeShader("Shaders/ForwardPlusRendering.hlsl", "ComputeFrustums");
	forwardPlusShader.CompileCS(renderer->GetDevice());
	forwardPlusShader.BindToContext(renderer->GetContext());
	renderer->InitForwardPlus(camera, window, forwardPlusShader);
	//forwardPlusShader.Unbind(renderer->GetContext());




	cameraObject->GetTransform().SetPosition(playerSpawnVec);
	playerObject->GetTransform().SetPosition(playerSpawnVec);
	playerObject->AddComponent<CapsuleColliderComponent>(0.5f, 1.5f, zero);
	playerObject->AddComponent<RigidBodyComponent>(50.f, FilterGroups::PLAYER, (FilterGroups::EVERYTHING), BodyType::DYNAMIC, true);
	
	playerObject->AddComponent<PlayerComp>(renderer, camera, house, Engine::Instance->GetPhysics(), guiManager, 100.f, 2.f, 20.f, 50.f, 3.f);
	playerObject->AddComponent<ControllerComp>(cameraObject, houseBaseObject); 
	playerObject->GetComponent<PlayerComp>()->SetInteriorPosition(this->interiorPosition.x, this->interiorPosition.y, this->interiorPosition.z);
	
	AddObject(cameraObject, playerObject);
	AddObject(playerObject);

	Object* spotLight = new Object("body_spotLight");

	dx::XMFLOAT3 lightTranslation = dx::XMFLOAT3(0.0f, 0.f, 0.0f);
	spotLight->GetTransform().SetPosition(dx::XMLoadFloat3(&lightTranslation));

	LightComponent* sLight = spotLight->AddComponent<LightComponent>(1, dx::XMFLOAT4(0.7f, 0.7f, 0.4f, 1.0f), 7.f);
	sLight->SetEnabled(true);
	sLight->SetIntensity(0.5f);
	//sLight->SetAttenuation();
	sLight->SetRange(12.f);
	sLight->SetSpotlightAngle(14.f);
	sLight->SetDirection({ 1.f, 0.f, 0.f });
	AddObject(spotLight, playerObject);


	Object* dLight = new Object("dirLight"); //directional light

	dx::XMFLOAT3 lightTranslationD = dx::XMFLOAT3(0.0f, 0.0f, 0.0f);
	dLight->GetTransform().SetPosition(dx::XMLoadFloat3(&lightTranslationD));
	LightComponent* dLightC = dLight->AddComponent<LightComponent>(2, dx::XMFLOAT4(0.7f, 0.2f, 0.2f, 1.0f), 7.f);
	dLightC->SetEnabled(true);
	dLightC->SetIntensity(0.2f);
	dx::XMFLOAT3 sunDirection;
	dx::XMStoreFloat3(&sunDirection, dx::XMVector3Normalize(dx::XMVectorSet(0, -1, 1, 0)));
	dLightC->SetDirection(sunDirection);
	AddObject(dLight);


	Object* spotLight2 = new Object("spotLight2");

	dx::XMFLOAT3 lightTranslation2 = dx::XMFLOAT3(0.0f, 30.f, 0.0f);
	spotLight2->GetTransform().SetPosition(dx::XMLoadFloat3(&lightTranslation2));
	LightComponent* sLight2 = spotLight2->AddComponent<LightComponent>(1, dx::XMFLOAT4(0.2f, 0.6f, 1.0f, 1.0f), 7.f);
	sLight2->SetEnabled(true);
	sLight2->SetIntensity(0.8f);
	sLight2->SetRange(60.f);
	sLight2->SetSpotlightAngle(10.f);
	sLight2->SetDirection({ 0.f,-1.f,0.f });
	AddObject(spotLight2);


	/* For fuel info from playercomp */
	nodeWalker->GetPlayerInfo(playerObject->GetComponent<PlayerComp>());

	world.Initialize(root, resources, renderer);
	
	

	/* PICKUP STUFF DONT DELETE THESEEE */
	Object* healthkitObject = resources->AssembleObject("HealthKit", "HealthKitMaterial");
	healthkitObject->AddFlag(ObjectFlag::DEFAULT);
	healthkitObject->GetComponent<MeshComponent>()->SetBatchable(true);
	healthkitObject->GetTransform().SetPosition({ 23,2,50 });
	healthkitObject->AddComponent<BoxColliderComponent>(dx::XMFLOAT3{ 0.5f, 0.5f, 0.5f }, dx::XMFLOAT3{ 0, 0, 0 });
	healthkitObject->AddComponent<PickupComponent>(PickupType::Health, 20.0f);
	healthkitObject->AddComponent<RigidBodyComponent>(0.f, FilterGroups::PICKUPS, (FilterGroups::EVERYTHING &~FilterGroups::PLAYER), BodyType::DYNAMIC,true);
	AddObject(healthkitObject);

	///* Fuel pickup stuff temporary */
	Object* fuelCanObject = resources->AssembleObject("FuelCanGreen", "FuelCanGreenMaterial");
	fuelCanObject->AddFlag(ObjectFlag::DEFAULT);
	fuelCanObject->GetComponent<MeshComponent>()->SetBatchable(true);
	fuelCanObject->GetTransform().SetPosition({ 22,2,52 });
	fuelCanObject->AddComponent<BoxColliderComponent>(dx::XMFLOAT3{ 0.3f, 0.3f, 0.3f }, dx::XMFLOAT3{ 0, 0, 0 });
	fuelCanObject->AddComponent<PickupComponent>(PickupType::Fuel, 20.0f);
	fuelCanObject->AddComponent<RigidBodyComponent>(10.f, FilterGroups::HOLDABLE, (FilterGroups::EVERYTHING &~FilterGroups::PLAYER), BodyType::DYNAMIC, true);
	AddObject(fuelCanObject);

	///* Banana pickup stuff temporary */
	Shader* particleShader = resources->GetShaderResource("particleShader");
	Object* beansObject = resources->AssembleObject("Soup", "SoupMaterial");
	beansObject->AddFlag(ObjectFlag::DEFAULT);
	beansObject->GetComponent<MeshComponent>()->SetBatchable(true);
	beansObject->GetTransform().SetPosition({22, 2.0f, 53 });
	beansObject->AddComponent<BoxColliderComponent>(dx::XMFLOAT3{ 0.5f, 0.5f, 0.5f }, dx::XMFLOAT3{ 0, 0, 0 });
	beansObject->AddComponent<PickupComponent>(PickupType::Food, 20.0f);
	beansObject->AddComponent<RigidBodyComponent>(0.f, FilterGroups::PICKUPS, (FilterGroups::EVERYTHING & ~FilterGroups::PLAYER), BodyType::DYNAMIC, true);

	beansObject->AddComponent<ParticleSystemComponent>(renderer, particleShader);
	beansObject->GetComponent<ParticleSystemComponent>()->InitializeParticles(renderer->GetDevice(), L"Textures/starstar.png");
	AddObject(beansObject);		
	//Has to be made in end of objects in order to use them

	AddObject(beansObject);

	//Player Arms
	Object* playerArms = new Object("PlayerArms", ObjectFlag::DEFAULT | ObjectFlag::NO_CULL);
	SkeletonMeshComponent* armsSkeleton = resources->GetResource<SkeletonMeshComponent>("PlayerArmsSkeleton");
	playerArms->AddComponent<SkeletonMeshComponent>(armsSkeleton);
	playerArms->AddComponent<PlayerAnimHandlerComp>(playerArms->GetComponent<SkeletonMeshComponent>(), cameraObject, player);
	player->GetComponent< PlayerComp>()->InsertArms(playerArms);
	AddObject(playerArms);

	//Axe
	Object* axeObject = resources->AssembleObject("Axe", "AxeMaterial", ObjectFlag::DEFAULT | ObjectFlag::NO_CULL);
	axeObject->GetTransform().SetPosition({ 21.0f, 1.0f, -16.0f });
	axeObject->GetTransform().SetScale({ 1.0f, 1.0f, 1.0f });
	axeObject->AddComponent<WeaponComponent>(playerArms->GetComponent<SkeletonMeshComponent>());
	playerObject->GetComponent<PlayerComp>()->InsertWeapon(axeObject->GetComponent<WeaponComponent>(), axeObject->GetName());
	AddObject(axeObject);
	

	//dx::XMFLOAT3 pos3 = { thePath.GetPoint(this->currentNode).x,HEIGHT, thePath.GetPoint(this->currentNode).z };

	roadSign = new Object("Endsign");
	roadSign = resources->AssembleObject("Endsign", "EndsignMaterial");
	rightSign = new Object("LeftDirectionSign");
	rightSign = resources->AssembleObject("LeftDirectionSign", "LeftDirectionSignMaterial");
	leftSign = new Object("RightDirectionSign");
	leftSign = resources->AssembleObject("RightDirectionSign", "RightDirectionSignMaterial");
	AddObject(roadSign);
	AddObject(rightSign);
	AddObject(leftSign);

	//LOADING BASE MONSTER; ADDING SKELETONS TO IT
	enemyManager = new EnemyManager();
	enemyManager->Initialize(player, player->GetComponent<PlayerComp>(), root);
	enemyManager->InitBaseEnemy();
	enemyManager->InitChargerEnemy();
	/* PuzzleModels */
	//Object* puzzleFrog = resources->AssembleObject("PuzzleFrogStatue", "PuzzleFrogStatueMaterial", ObjectFlag::DEFAULT);
	////puzzleManager = new PuzzleManager(resources, player, house);
	//puzzleFrog->GetTransform().SetPosition({ 26, 1.5f, 50 });
	//puzzleFrog->AddComponent<BoxColliderComponent>(dx::XMFLOAT3{ 2.0f, 2.0f, 2.0f }, dx::XMFLOAT3{ 0, 0, 0 });
	//puzzleFrog->AddComponent<RigidBodyComponent>(0.0f, FilterGroups::PROPS, FilterGroups::EVERYTHING, BodyType::DYNAMIC, true);
	///*puzzleFrog->AddComponent<BoxColliderComponent>(dx::XMFLOAT3{ 2.0f, 2.0f, 2.0f }, dx::XMFLOAT3{ 0, 0, 0 });
	//puzzleFrog->AddComponent<RigidBodyComponent>(50.f, FilterGroups::PROPS, (FilterGroups::EVERYTHING), BodyType::DYNAMIC, true);*/
	//AddObject(puzzleFrog);
	
	Object* puzzleFly = resources->AssembleObject("PuzzleFlyStatue", "PuzzleFlyStatueMaterial", ObjectFlag::DEFAULT | ObjectFlag::NO_CULL);
	puzzleFly->GetTransform().SetPosition({ 28, 1.3f, 50 });
	puzzleFly->AddComponent<BoxColliderComponent>(dx::XMFLOAT3{ 1.0f, 1.0f, 1.0f }, dx::XMFLOAT3{ 0, 0, 0 });
	puzzleFly->AddComponent<RigidBodyComponent>(0.0f, FilterGroups::HOLDABLE, FilterGroups::EVERYTHING, BodyType::DYNAMIC, true);
	puzzleFly->AddComponent<ParticleSystemComponent>(renderer, particleShader);
	puzzleFly->GetComponent<ParticleSystemComponent>()->SetMaxParticles(50);
	puzzleFly->GetComponent<ParticleSystemComponent>()->SetParticleSize(0.1f);
	puzzleFly->GetComponent<ParticleSystemComponent>()->InitializeParticles(renderer->GetDevice(), L"Textures/fire1.png");
	AddObject(puzzleFly);

	/*Shader* fireShader = resources->GetShaderResource("fireShader");*/

	/* Different firestuff stuffy stuff  */
	/*Object* puzzleFlyTest2 = resources->AssembleObject("PuzzleFlyStatue", "PuzzleFlyStatueMaterial", ObjectFlag::DEFAULT | ObjectFlag::NO_CULL);
	puzzleFlyTest2->GetTransform().SetPosition({ 28, 1.3f, 48 });
	puzzleFlyTest2->AddComponent<FireTextureComponent>(renderer, fireShader);
	puzzleFlyTest2->GetComponent<FireTextureComponent>()->InitializeFire(renderer->GetDevice());
	AddObject(puzzleFlyTest2);*/

	//Object* fire = puzzleFlyTest2->GetComponent<FireTextureComponent>()->GetFireObject();
	//AddObject(fire);	
	/*FrogPuzzle* frogpuzzle = new FrogPuzzle(resources);
	frogpuzzle*/
	GUICompass* compass = new GUICompass(*renderer, window, house, player);
	guiManager->AddGUIObject(compass, "compass");
}

void GameScene::InitializeGUI()
{
	float windowWidth = FCAST(window->GetWidth());
	float windowHeight = FCAST(window->GetHeight());
	
	//INFO, WE ARE DRAWING BACK TO FRONT. IF YOU WANT SOMETHING TO BE IN FRONT. SET VALUE TO 0. IF YOU WANT IT IN BACK USE 0.1 -> 1

	//BUTTONS AT LEFT SIDE
	//spriteBatch = new DirectX::SpriteBatch(renderer->GetContext());
	GUISprite* equimpmentSprite1 = new GUISprite(*renderer, "Textures/EquipmentBox.png", 10, 10, 0, DrawDirection::BottomLeft, ClickFunction::NotClickable);
	GUISprite* equimpmentSprite2 = new GUISprite(*renderer, "Textures/EquipmentBox.png", 90, 10, 0, DrawDirection::BottomLeft, ClickFunction::NotClickable);
	GUISprite* equimpmentSprite3 = new GUISprite(*renderer, "Textures/EquipmentBox.png", 170, 10, 0, DrawDirection::BottomLeft, ClickFunction::NotClickable);
	GUISprite* equimpmentSprite4 = new GUISprite(*renderer, "Textures/EquipmentBox.png", 250, 10, 0, DrawDirection::BottomLeft, ClickFunction::NotClickable);

	//BARS THAT SCALING 
	GUISprite* fuelScalingBar = new GUISprite(*renderer, "Textures/DippingBar.png", 170, 10, 0.5, DrawDirection::BottomRight, ClickFunction::NotClickable);
	GUISprite* foodScalingBar = new GUISprite(*renderer, "Textures/DippingBar.png", 90, 10, 0.5, DrawDirection::BottomRight, ClickFunction::NotClickable);
	GUISprite* healthScalingBar = new GUISprite(*renderer, "Textures/DippingBar.png", 10, 10, 0.5, DrawDirection::BottomRight, ClickFunction::NotClickable);

	//BARS AR RIGHT SIDE
	GUISprite* fuelBar = new GUISprite(*renderer, "Textures/Health_Fuel_Food.png", 170, 10, 1, DrawDirection::BottomRight, ClickFunction::NotClickable);
	GUISprite* foodBar = new GUISprite(*renderer, "Textures/Health_Fuel_Food.png", 90, 10, 1, DrawDirection::BottomRight, ClickFunction::NotClickable);
	GUISprite* healthBar = new GUISprite(*renderer, "Textures/Health_Fuel_Food.png", 10, 10, 1, DrawDirection::BottomRight, ClickFunction::NotClickable);

	//ICONS ON TOP OF ITEMS
	GUISprite* equimpmentSpriteAxe = new GUISprite(*renderer, "Textures/AxeIcon2.png", 10, 10, 0, DrawDirection::BottomLeft, ClickFunction::NotClickable);
	//ICONS ON TOP OF BARS
	GUISprite* fuelSprite = new GUISprite(*renderer, "Textures/FireIcon.png", 170, 10, 0, DrawDirection::BottomRight, ClickFunction::NotClickable);
	GUISprite* foodSprite = new GUISprite(*renderer, "Textures/FoodIcon.png", 90, 10, 0, DrawDirection::BottomRight, ClickFunction::NotClickable);
	GUISprite* healthSprite = new GUISprite(*renderer, "Textures/HealthIcon.png", 10, 10, 0, DrawDirection::BottomRight, ClickFunction::NotClickable);

	//COMPASS

	//FONTS
	GUIFont* fpsDisplay = new GUIFont(*renderer, "fps",30, 30);
	//GUIFont* healthDisplay = new GUIFont(*renderer, "playerHealth", 50, 100);
	//GUIFont* enemyDisplay = new GUIFont(*renderer, "enemyHealth", 50, 150);

	//CROSSHAIR
	GUISprite* dot = new GUISprite(*renderer, "Textures/TestFix2.png", (windowWidth / 2) - 6, (windowHeight / 2) - 6, 0, DrawDirection::BottomLeft, ClickFunction::NotClickable);
	GUISprite* crosshair = new GUISprite(*renderer, "Textures/Crosshair.png", (windowWidth / 2) - 25, (windowHeight / 2) - 25, 0, DrawDirection::BottomLeft, ClickFunction::NotClickable);
	GUISprite* doorSprite = new GUISprite(*renderer, "Textures/DoorSprite.png", (windowWidth / 2) - 6, (windowHeight / 2) - 6, 0, DrawDirection::BottomLeft, ClickFunction::NotClickable);
	GUISprite* fuel = new GUISprite(*renderer, "Textures/Fuel_Icon.png", (windowWidth / 2) - 6, (windowHeight / 2) - 6, 0, DrawDirection::BottomLeft, ClickFunction::NotClickable);

	//dot->SetVisible(false);
	crosshair->SetVisible(false);
	doorSprite->SetVisible(false);
	fuel->SetVisible(false);

	// INSERTIONS
	guiManager = new GUIManager(renderer, 0);
	guiManager->AddGUIObject(fpsDisplay, "fps");
	//guiManager->AddGUIObject(healthDisplay, "playerHealth");
	//guiManager->AddGUIObject(enemyDisplay, "enemyHealth");
	//COMPASS


	//BASE OF EQUIPMENT
	guiManager->AddGUIObject(equimpmentSprite1, "equimpmentSprite1");
	guiManager->AddGUIObject(equimpmentSprite2, "equimpmentSprite2");
	guiManager->AddGUIObject(equimpmentSprite3, "equimpmentSprite3");
	guiManager->AddGUIObject(equimpmentSprite4, "equimpmentSprite4");

	//BASE OF DIPPING BARS
	foodScalingBar->SetScale(1.0f, 0.0f);
	healthScalingBar->SetScale(1.0f, 0.0f);
	fuelScalingBar->SetScale(1.0f, 0.0f);
	guiManager->AddGUIObject(foodScalingBar, "foodDippingBar");
	guiManager->AddGUIObject(healthScalingBar, "healthDippingBar");
	guiManager->AddGUIObject(fuelScalingBar, "fuelDippingBar");

	//ICON OF EQUIPMENT
	guiManager->AddGUIObject(equimpmentSpriteAxe, "equimpmentSpriteAxe");
	//BASE OF BARS
	guiManager->AddGUIObject(fuelBar, "fuelBar");
	guiManager->AddGUIObject(foodBar, "foodBar");
	guiManager->AddGUIObject(healthBar, "healthBar");

	//ICON OF BARS
	guiManager->AddGUIObject(fuelSprite, "fuelSprite");
	guiManager->AddGUIObject(foodSprite, "foodSprite");
	guiManager->AddGUIObject(healthSprite, "healthSprite");

	//CROSSHAIR
	guiManager->AddGUIObject(dot, "dot");
	guiManager->AddGUIObject(crosshair, "crosshair");
	guiManager->AddGUIObject(doorSprite, "door");
	guiManager->AddGUIObject(fuel, "fuel");

}

void GameScene::InitializeInterior()
{
	//_____________________________________________________________________________________________________________________________________
	// Inside house
	Object* houseInterior = resources->AssembleObject("HouseInterior", "HouseInteriorMaterial");
	houseInterior->GetTransform().SetPosition({ this->interiorPosition.x, this->interiorPosition.y, this->interiorPosition.z, 0 });
	houseInterior->AddComponent<BoxColliderComponent>(dx::XMFLOAT3(10.0f, 0.5f, 10.0f), dx::XMFLOAT3(0, 0, 0)); // Floor
	houseInterior->AddComponent<BoxColliderComponent>(dx::XMFLOAT3(1.5f, 10.0f, 10.0f), dx::XMFLOAT3(5.0f, 0, 0)); // Right wall
	houseInterior->AddComponent<BoxColliderComponent>(dx::XMFLOAT3(1.5f, 10.0f, 10.0f), dx::XMFLOAT3(-9.0f, 0, 0)); // Left wall
	houseInterior->AddComponent<BoxColliderComponent>(dx::XMFLOAT3(10.0f, 10.0f, 1.5f), dx::XMFLOAT3(0, 0, 7.0f)); // Front wall
	houseInterior->AddComponent<BoxColliderComponent>(dx::XMFLOAT3(10.0f, 10.0f, 1.5f), dx::XMFLOAT3(0, 0, -8.0f)); // Back wall
	houseInterior->AddComponent<RigidBodyComponent>(0.0f, FilterGroups::PROPS, FilterGroups::EVERYTHING, BodyType::STATIC, true); // RB
	AddObject(houseInterior);

	Object* fireplace = resources->AssembleObject("Fireplace", "FireplaceMaterial");
	fireplace->GetTransform().SetPosition({ this->interiorPosition.x, this->interiorPosition.y, this->interiorPosition.z });
	fireplace->AddComponent<BoxColliderComponent>(dx::XMFLOAT3(1.5f, 5.0f, 1.0f), dx::XMFLOAT3(-8.1f, 0, -1.3f));
	fireplace->AddComponent<RigidBodyComponent>(0.0f, FilterGroups::PROPS, FilterGroups::EVERYTHING, BodyType::STATIC, true);
	AddObject(fireplace);

	Object* logs = resources->AssembleObject("Logs", "LogsMaterial");
	logs->GetTransform().SetPosition({ this->interiorPosition.x, this->interiorPosition.y, this->interiorPosition.z });
	logs->AddComponent<BoxColliderComponent>(dx::XMFLOAT3(1.0f, 1.0f, 1.0f), dx::XMFLOAT3(-8.1f, 1.0f, -1.3f));
	logs->AddComponent<RigidBodyComponent>(0.0f, FilterGroups::FIRE, FilterGroups::EVERYTHING, BodyType::STATIC, true);
	AddObject(logs);

	Object* flowerpot = resources->AssembleObject("Flowerpot", "FlowerpotMaterial");
	AddObject(flowerpot, houseInterior);

	Object* curtains = resources->AssembleObject("Curtains", "CurtainsMaterial");
	AddObject(curtains, houseInterior);

	Object* bed = resources->AssembleObject("Bed", "BedMaterial");
	bed->GetTransform().SetPosition({ this->interiorPosition.x, this->interiorPosition.y, this->interiorPosition.z });
	bed->AddComponent<BoxColliderComponent>(dx::XMFLOAT3(2.5f, 1.0f, 2.0f), dx::XMFLOAT3(-2.0f, 1.0f, 5.f));
	bed->AddComponent<RigidBodyComponent>(0.0f, FilterGroups::PROPS, FilterGroups::EVERYTHING, BodyType::STATIC, true);
	AddObject(bed);

	Object* bookShelf = resources->AssembleObject("BookShelf", "BookShelfMaterial");
	bookShelf->GetTransform().SetPosition({ this->interiorPosition.x, this->interiorPosition.y, this->interiorPosition.z });
	bookShelf->AddComponent<BoxColliderComponent>(dx::XMFLOAT3(1.0f, 5.0f, 1.5f), dx::XMFLOAT3(1.0f, 1.0f, -7.f));
	bookShelf->AddComponent<RigidBodyComponent>(0.0f, FilterGroups::PROPS, FilterGroups::EVERYTHING, BodyType::STATIC, true);
	AddObject(bookShelf);

	Object* chair = resources->AssembleObject("Chair", "ChairMaterial");
	chair->GetTransform().SetPosition({ this->interiorPosition.x, this->interiorPosition.y, this->interiorPosition.z });
	chair->AddComponent<BoxColliderComponent>(dx::XMFLOAT3(0.5f, 2.0f, 1.0f), dx::XMFLOAT3(-4.0f, 1.0f, 1.5f));
	chair->AddComponent<RigidBodyComponent>(0.0f, FilterGroups::PROPS, FilterGroups::EVERYTHING, BodyType::STATIC, true);
	AddObject(chair);

	Object* sink = resources->AssembleObject("Sink", "SinkMaterial");
	sink->GetTransform().SetPosition({ this->interiorPosition.x, this->interiorPosition.y, this->interiorPosition.z });
	sink->AddComponent<BoxColliderComponent>(dx::XMFLOAT3(0.5f, 2.0f, 1.5f), dx::XMFLOAT3(-1.5f, 1.0f, -7.f));
	sink->AddComponent<RigidBodyComponent>(0.0f, FilterGroups::PROPS, FilterGroups::EVERYTHING, BodyType::STATIC, true);
	AddObject(sink);

	Object* stove = resources->AssembleObject("Stove", "StoveMaterial");
	stove->GetTransform().SetPosition({ this->interiorPosition.x, this->interiorPosition.y, this->interiorPosition.z });
	stove->AddComponent<BoxColliderComponent>(dx::XMFLOAT3(0.5f, 2.0f, 1.5f), dx::XMFLOAT3(-2.5f, 1.0f, -7.f));
	stove->AddComponent<RigidBodyComponent>(0.0f, FilterGroups::PROPS, FilterGroups::EVERYTHING, BodyType::STATIC, true);
	AddObject(stove);

	Object* insideDoor = resources->AssembleObject("InsideDoor", "InsideDoorMaterial");
	insideDoor->GetTransform().SetPosition({ this->interiorPosition.x, this->interiorPosition.y, this->interiorPosition.z });
	insideDoor->AddComponent<BoxColliderComponent>(dx::XMFLOAT3(0.2f, 2.0f, 1.5f), dx::XMFLOAT3(3.0f, 2.0f, -3.0f));
	insideDoor->AddComponent<RigidBodyComponent>(0.0f, FilterGroups::DOOR, FilterGroups::EVERYTHING, BodyType::STATIC, true);
	AddObject(insideDoor);

	Object* fireLight = new Object("fireLight");
	LightComponent* fLight = fireLight->AddComponent<LightComponent>(LightType::POINT_LIGHT,dx::XMFLOAT4(1.0f, 0.29f, 0.0f, 1.0f), 1.7f);
	fireLight->GetTransform().SetPosition({ -7.0f, -99.f, -1.36 });
	fireLight->AddComponent<ParticleSystemComponent>(renderer, Engine::Instance->GetResources()->GetShaderResource("particleShader"));
	fireLight->GetComponent<ParticleSystemComponent>()->InitializeFirelikeParticles(renderer->GetDevice(), L"Textures/fire1.png");
	fireLight->AddFlag(ObjectFlag::DEFAULT | ObjectFlag::NO_CULL);
	fLight->SetEnabled(true);
	fLight->SetIntensity(1.f);
	AddObject(fireLight);

	Object* windowLight = new Object("windowLight");
	windowLight->GetTransform().SetPosition({ 3.0f, -98.f, 3 });
	LightComponent * wLight1 = windowLight->AddComponent<LightComponent>(LightType::POINT_LIGHT,dx::XMFLOAT4(0.3f, 0.41f, 0.8f, 1.0f), 5.0f);
	wLight1->SetEnabled(true);
	AddObject(windowLight);

	Object* windowLight2 = new Object("windowLight2");
	windowLight2->GetTransform().SetPosition({ -7, -98.f, 3 });
	LightComponent* wLight2 = windowLight2->AddComponent<LightComponent>(LightType::POINT_LIGHT,dx::XMFLOAT4(0.3f, 0.41f, 0.8f, 1.0f), 5.0f);
	wLight2->SetEnabled(true);
	AddObject(windowLight2);
}

void GameScene::OnActivate()
{
	SaveState state;
	
	state.seed = 1337;
	state.segment = 0;

	player->GetComponent<PlayerComp>()->Reset();
	world.ConstructSegment(state);

	//PrintSceneHierarchy(root, 0);

	house->GetComponent<NodeWalkerComp>()->InitializePath(world.GetPath());

	InitializeSigns();
	
	std::cout << "X: " << rightSign->GetTransform().GetPosition().m128_f32[0] << " Y: " << rightSign->GetTransform().GetPosition().m128_f32[1] << " Z: " << rightSign->GetTransform().GetPosition().m128_f32[2] << std::endl;

	if (house != nullptr && player != nullptr)
	{
		std::vector<dx::XMINT2> indexes = world.GetPath().GetIndexes();
		dx::XMINT2 spawnIndex = indexes[0];

		dx::XMVECTOR position = dx::XMVectorAdd(Chunk::IndexToWorld(spawnIndex, 0.0f), dx::XMVectorSet(CHUNK_SIZE / 2.0f, 0, CHUNK_SIZE / 2.0f, 0));
		house->GetTransform().SetPosition(position);
		
		if (house->HasComponent<RigidBodyComponent>())
			house->GetComponent<RigidBodyComponent>()->SetPosition(position);

		position = dx::XMVectorAdd(position, dx::XMVectorSet(0, 12, -5, 0));

		player->GetTransform().SetPosition(position);
		player->GetComponent<RigidBodyComponent>()->SetPosition(position);
	}

	renderer->AddRenderPass(guiManager);

	Input::Instance().ConfineMouse();
	Input::Instance().SetMouseMode(dx::Mouse::Mode::MODE_RELATIVE);
	ShowCursor(false);

	AudioMaster::Instance().PlaySoundEvent("wind");
	//this->PrintSceneHierarchy(root, 0);
	enemyManager->SpawnEnemies();
	
	LightManager::Instance().ForceUpdateBuffers(renderer->GetContext(),camera);
}

void GameScene::OnDeactivate()
{
	world.DeconstructSegment();
	renderer->RemoveRenderPass(guiManager);
	enemyManager->DespawnEnemies();
	
	//renderer->ClearParticles();

	ShowCursor(true);
	//this->PrintSceneHierarchy(root, 0);
}

void GameScene::SwitchScene()
{
	OnDeactivate();
	ShowCursor(false);
	fogId += 0.5f;
	fogCol += 0.5f; 

	SaveState state;
	state.seed = rand();
	state.segment = 0;
	world.ConstructSegment(state);
	//PrintSceneHierarchy(root, 0);
	renderer->SetIdAndColor(fogId, fogCol);

	house->GetComponent<NodeWalkerComp>()->InitializePath(world.GetPath());

	//Place signs
	InitializeSigns();


	if (house != nullptr && player != nullptr)
	{
		std::vector<dx::XMINT2> indexes = world.GetPath().GetIndexes();
		dx::XMINT2 spawnIndex = indexes[0];

		dx::XMVECTOR position = dx::XMVectorAdd(Chunk::IndexToWorld(spawnIndex, 0.0f), dx::XMVectorSet(CHUNK_SIZE / 2.0f, 0, CHUNK_SIZE / 2.0f, 0));
		house->GetTransform().SetPosition(position);

		if (house->HasComponent<RigidBodyComponent>())
			house->GetComponent<RigidBodyComponent>()->SetPosition(position);

		position = dx::XMVectorAdd(position, dx::XMVectorSet(0, 12, -5, 0));

		player->GetTransform().SetPosition(position);
		player->GetComponent<RigidBodyComponent>()->SetPosition(position);
	}

	renderer->AddRenderPass(guiManager);

	/*
	Input::Instance().ConfineMouse();
	Input::Instance().SetMouseMode(dx::Mouse::Mode::MODE_RELATIVE);
	ShowCursor(false);

	AudioMaster::Instance().PlaySoundEvent("wind");*/
	//this->PrintSceneHierarchy(root, 0);

	//LOADING BASE MONSTER; ADDING SKELETONS TO IT

	enemyManager->SpawnEnemies();

	LightManager::Instance().ForceUpdateBuffers(renderer->GetContext(),camera);
}

void GameScene::InitializeSigns()
{
	//skapa riktningsvektor från näst sista noden och sista noden för att flytta fram skyltarna lite så de hamnar i grenen.

	dx::XMVECTOR signPosition;
	dx::XMVECTOR lastNodePos = { nodeWalker->GetLastNodePos().x, nodeWalker->GetLastNodePos().y - 1, nodeWalker->GetLastNodePos().z };
	dx::XMVECTOR secondLastNodePos = { nodeWalker->GetSecondLastNodePos().x, nodeWalker->GetSecondLastNodePos().y - 1, nodeWalker->GetSecondLastNodePos().z };
	//dx::XMVECTOR nodeVector = { 
	// secondLastNodePos.m128_f32[0] - lastNodePos.m128_f32[0],
	// secondLastNodePos.m128_f32[1] - lastNodePos.m128_f32[1] ,
	// secondLastNodePos.m128_f32[2] - lastNodePos.m128_f32[2] };

	//signPosition = { nodeVector.m128_f32[0] * 1.5f, nodeVector.m128_f32[1] * 1.5f, nodeVector.m128_f32[2] * 1.5f, };

	//signPosition = { nodeWalker->Getpos3().x, nodeWalker->Getpos3().y - 1, nodeWalker->Getpos3().z }; //FIRST NODE
	//signPosition = { nodeWalker->GetLastNodePos().x, nodeWalker->GetLastNodePos().y - 1, nodeWalker->GetLastNodePos().z }; //LAST NODE
	//signPosition = { nodeWalker->GetSecondLastNodePos().x, nodeWalker->GetSecondLastNodePos().y - 1, nodeWalker->GetSecondLastNodePos().z }; //SECOND LAST NODE

	signPosition = { world.GetPath().GetSignPosition().x , 1.0f ,world.GetPath().GetSignPosition().y };

	roadSign->GetTransform().SetPosition({signPosition}); 

	//Right Sign
	rightSign->GetTransform().SetPosition({ roadSign->GetTransform().GetPosition().m128_f32[0] - 1.0f, roadSign->GetTransform().GetPosition().m128_f32[1], roadSign->GetTransform().GetPosition().m128_f32[2] });
	rightSign->AddComponent<BoxColliderComponent>(dx::XMFLOAT3{ 1.0f, 1.0f, 1.0f }, dx::XMFLOAT3{ 0, 0, 0 });
	rightSign->AddComponent<SelectableComponent>();
	rightSign->AddComponent<RigidBodyComponent>(0.0f, FilterGroups::CLICKABLE, (FilterGroups::EVERYTHING & ~FilterGroups::PLAYER), BodyType::STATIC, true);

	//Left Sign
	leftSign->GetTransform().SetPosition({ roadSign->GetTransform().GetPosition().m128_f32[0] + 1.0f, roadSign->GetTransform().GetPosition().m128_f32[1], roadSign->GetTransform().GetPosition().m128_f32[2] });
	leftSign->AddComponent<BoxColliderComponent>(dx::XMFLOAT3{ 1.0f, 1.0f, 1.0f }, dx::XMFLOAT3{ 0, 0, 0 });
	leftSign->AddComponent<SelectableComponent>();
	leftSign->AddComponent<RigidBodyComponent>(0.0f, FilterGroups::CLICKABLE, (FilterGroups::EVERYTHING & ~FilterGroups::PLAYER), BodyType::STATIC, true);
}

void GameScene::Update(const float& deltaTime)
{
	Scene::Update(deltaTime);
	world.UpdateRelevantChunks(player->GetTransform(), camera);
	//world.DrawDebug();

	// Something CP with controllerComp/player wont allow this to happen inside the playerComp
	if (player->GetComponent<ControllerComp>()->GetInRange())
	{
		static_cast<GUISprite*>(guiManager->GetGUIObject("door"))->SetVisible(true);
		static_cast<GUISprite*>(guiManager->GetGUIObject("dot"))->SetVisible(false);
	}
	else
	{
		static_cast<GUISprite*>(guiManager->GetGUIObject("door"))->SetVisible(false);
	}

	if (rightSign->GetComponent<SelectableComponent>()->GetActive())
	{
		SwitchScene();
		rightSign->GetComponent<SelectableComponent>()->SetActive(false);

	}
	else if (leftSign->GetComponent<SelectableComponent>()->GetActive())
	{
		SwitchScene();
		leftSign->GetComponent<SelectableComponent>()->SetActive(false);

	}

	static_cast<GUIFont*>(guiManager->GetGUIObject("fps"))->SetString(std::to_string((int)GameClock::Instance().GetFramesPerSecond()));
	guiManager->UpdateAll();

}

void GameScene::FixedUpdate(const float& fixedDeltaTime)
{
	Scene::FixedUpdate(fixedDeltaTime);
}

void GameScene::Render()
{
	camera->UpdateView();

	root->Draw(renderer, camera);
	//worldGenerator.DrawShapes();
	//world.DrawDebug();
	
	renderer->RenderFrame(camera, (float)clock.GetSeconds(),player->GetComponent<PlayerComp>()->GetDangerDistance() );
}