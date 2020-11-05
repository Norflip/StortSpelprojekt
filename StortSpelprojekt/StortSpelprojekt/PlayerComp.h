#pragma once
#include "Component.h"
#include "GUIManager.h"
#include "GameClock.h"
#include "GUISprite.h"
#include "Scene.h"

class WeaponComponent;

class PlayerComp :
    public Component
{
public:
	//enum NEXT_SCENE { INTRO, LOSE, GAME, WIN };


	//PlayerComp();
	PlayerComp(Renderer* renderer, CameraComponent* camComp, Physics& phys, GUIManager* guimanager, float health, float movementSpeed, float radius, float attack, float attackSpeed);
	virtual ~PlayerComp();
	void Update(const float& deltaTime) override;

	float GetHealth() const { return health; }
	float GetSpeed() const { return movementSpeed; }
	float GetAttack() const { return attack; }
	float GetAttackSpeed() const { return attackSpeed; }
	float GetFuel()const { return fuel; }
	float GetFood()const { return food; }
	//int GetCurrentWeapon()const { return currentWeapon; }
	float GetSprintSpeed()const { return SprintSpeed; }
	float GetCrouchSpeed()const { return crouchSpeed; }
	float GetMovementspeed()const { return movementSpeed; }

	float GetRadius() const { return radius; }
	void LoseHealth(float damage) { health -= damage; }	
	void AddHealth(float health) { this->health += health; }	
	void AddAttack(float attack) { this->attack += attack; }
	void SetAttackSpeed(float attackSpeed) { this->attackSpeed = attackSpeed; }
	void SetAttack(float attack) { this->attack = attack; }
	void SetHealth(float health) { this->health = health; }
	void SetMovementSpeed(float movementSpeed) { this->movementSpeed = movementSpeed; }
	void SetCrouchSpeed(float crouchSpeed) { this->crouchSpeed = crouchSpeed; }
	void SetSprintSpeed(float SprintSpeed) { this->crouchSpeed = SprintSpeed; }
	void SetFuel(float fuel) { this->fuel = fuel; }
	void SetFood(float food) { this->fuel = food; }

	
	//void SetCurrentWeapon(int currentWeapon) { this->currentWeapon = currentWeapon; }// some ui stuff here?
	void SetguiMan(GUIManager* guiMan) { this->guiMan = guiMan; }
	void InsertWeapon(WeaponComponent* weapon, std::string name);
	NEXT_SCENE GetNextScene() { return this->swapScene; }
private:
	void HoldObject();
	void DropObject();
	float health, attack, attackSpeed, fuel, food;
	//int currentWeapon;
	float movementSpeed, crouchSpeed, SprintSpeed;
	float foodLossPerSecond, fuelBurnPerMeter, healthLossPerSecond;
	float radius;
	GUIManager* guiMan;
	GUISprite* fuelDippingBar, *foodDippingBar,* healthDippingBar;
	NEXT_SCENE swapScene;
	Renderer* renderer;
	POINT p;
	std::unordered_map<std::string, WeaponComponent*> weaponsList;
	EnemyStatsComp* enemyStatsComp;
	float rayDistance;	
	Object* currentWeapon;
	Object* holding;
	RayHit hit;
	Physics& phy;
	CameraComponent* cam;
	//GameClock attackTimer;

	bool foodEmpty;
	bool gg;
	float ReverseAndClamp(float inputValue);
	float test = 0;
	void RayCast(const float& deltaTime);
	dx::XMVECTOR toYAW;
	dx::XMMATRIX inverseViewMatrix, wepOffTrans, wepOffRot, wepWorld;
	dx::XMVECTOR	weaponScale, weaponRot, weaponPos, up;

};

