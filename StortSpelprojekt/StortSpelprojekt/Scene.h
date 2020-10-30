#pragma once

enum NEXT_SCENE { INTRO = 0, LOSE = 1, GAME = 2, WIN = 3 };
#include <string>
#include <vector>
#include "Object.h"
#include "HeightMap.h"
#include "ShittyOBJLoader.h"
#include "MoveComponent.h"
#include "ControllerComp.h"
#include "SkeletonMeshComponent.h"
#include "PointLightComponent.h"
#include "NodeWalkerComp.h"
#include "EnemyStatsComp.h"
#include "EnemySMComp.h"
#include "PlayerAttackComp.h"

#include "Skybox.h"
#include "SaveState.h"
#include "GUIManager.h"


#include "Physics.h"
#include "RigidBodyComponent.h"
#include "SphereColliderComponent.h"
#include "CapsuleColliderComponent.h"
#include "DebugBoxShapeComponent.h"
#include "PickupComponent.h"


#include "ResourceManager.h"
#include "ObjectPooler.h"
#include "WeaponComponent.h"

class GUIFont;
class SpriteRenderPass;
class PlayerComp;

#define immortal 0

class Scene
{
	
public:
	Scene(ResourceManager* manager);
	virtual ~Scene();

	virtual void Initialize(Renderer* renderer) = 0;
	virtual void InitializeObjects() = 0;
	virtual void InitializeGUI() = 0;
	
	// What is this for?
	virtual void OnActivate() = 0;
	virtual void OnDeactivate() = 0;
	
	virtual void Update(const float& deltaTime);
	virtual void FixedUpdate(const float& fixedDeltaTime);
	virtual void Render();
	
	void AddObject(Object* object);
	void AddObject(Object* object, Object* parent);

	void RemoveObject(Object* object);
	Object* GetRoot() const { return this->root; }

	void PrintSceneHierarchy(Object* object, size_t level) const;

	bool Quit();
	NEXT_SCENE nextScene;
	
private:

protected:
	Object* root;
	CameraComponent* camera;
	Renderer* renderer;
	GameClock clock;
	Input& input;
	Object* enemy;
	Object* player;

	//EnemyStatsComp* enemyStatsComp;
	//PlayerComp* playerStatsComp;
	dx::SpriteBatch* spriteBatch;
	SpriteRenderPass* spritePass;	

	ObjectPooler pooler;
	GUIManager* guiManager;		
	
	Skybox* skyboxClass;		
	ResourceManager* resourceManager;

	float windowHeight, windowWidth;
	bool quit;	

};