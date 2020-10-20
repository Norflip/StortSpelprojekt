#pragma once
#include "Component.h"
#include "GUIManager.h"
#include "GameClock.h"
class PlayerComp :
    public Component
{
public:
	PlayerComp();
	PlayerComp(float health, float movementSpeed, float radius, float attack, float attackSpeed);
	virtual ~PlayerComp();
	void Update(const float& deltaTime) override;
	float GetHealth() const { return health; }
	float GetSpeed() const { return movementSpeed; }
	float GetAttack() const { return attack; }
	float GetAttackSpeed() const { return attackSpeed; }
	float GetFuel()const { return fuel; }
	float GetFood()const { return food; }
	float GetCurrentWeapon()const { return currentWeapon; }
	float GetSprintSpeed()const { return SprintSpeed; }
	float GetCrouchSpeed()const { return crouchSpeed; }
	float GetMovementspeed()const { return movementSpeed; }


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
	void SetCurrentWeapon(int currentWeapon) { this->currentWeapon = currentWeapon; }// some ui stuff here?
	void SetguiMan(GUIManager* guiMan) { this->guiMan = guiMan; }
	void SetScenePtr(int* guiMan) { this->nextScene = nextScene; }
	void UpdatePayer();
private:
	float health, attack, attackSpeed, fuel, food;
	int currentWeapon;
	float movementSpeed, crouchSpeed, SprintSpeed;
	float foodLossPerSecond, fuelBurnPerMeter;
	int* nextScene;
	GUIManager* guiMan;
};

