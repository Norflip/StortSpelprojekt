#pragma once
#include "Scene.h"
#include "AudioMaster.h"
class GUIFont;
class SpriteRenderPass;

ALIGN16
class IntroScene : public Scene
{
private:
	SoundEvent menuTest;
	SoundEvent test2, test3;

public:
	IntroScene(ResourceManager* manager);
	~IntroScene();

	void Initialize(Renderer* renderer) override;
	void InitializeObjects() override;
	void InitializeGUI() override;

	void OnActivate() override;
	void OnDeactivate() override;

	void Update(const float& deltaTime) override;
	void FixedUpdate(const float& fixedDeltaTime) override;
	void Render() override;
	
	ALIGN16_ALLOC;

};