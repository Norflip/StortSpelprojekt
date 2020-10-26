#include "ControllerComp.h"

bool ControllerComp::IsGrounded() const
{
	dx::XMFLOAT3 origin;
	dx::XMStoreFloat3(&origin, GetOwner()->GetTransform().GetPosition());
	//origin.z += 2.f;
	Ray ray(origin, DOWN_VEC);
	RayHit hit;

	//TERRAIN or default depending on if u can jump from on top of objects
	float distance = 2.8f;
	Physics& phy = Physics::Instance();
	phy.RaytestSingle(ray, distance, hit, FilterGroups::TERRAIN);
	
	bool result = false;
	if (hit.object != nullptr)
	{
		result = true;
		//std::cout << "picking: " << hit.object->GetName() << std::endl;
		//DShape::DrawLine(ray.origin, ray.GetPoint(distance), { 0,0,1 });
	}
	//else
		//DShape::DrawLine(ray.origin, ray.GetPoint(distance), { 1,0,0 });
	
	return result;
}

ControllerComp::ControllerComp(Object* cameraObject)
{
	this->fov = 60.f;
	this->fovTimer = 0.f;

	this->velocity = 0.f;
	this->velocityTimer = 0.f;
	this->crouchTimer = 0.f;
	this->cameraOffset = { 0.f,0.f,0.f };

	this->xClamp = 0.f;
	this->freeCam = false;
	this->showCursor = false;
	this->canRotate = true;

	this->groundQuaterion = { 0.f,0.f,0.f,1.f };
	this->cameraObject = cameraObject;
	this->rbComp = nullptr;
	this->camComp = nullptr;
	this->capsuleComp = nullptr;
}

ControllerComp::~ControllerComp()
{
}

void ControllerComp::Initialize()
{
	this->rbComp = GetOwner()->GetComponent<RigidBodyComponent>();
	this->camComp = cameraObject->GetComponent<CameraComponent>();
	this->capsuleComp = GetOwner()->GetComponent<CapsuleColliderComponent>();
	ShowCursor(!this->canRotate);

	if (this->canRotate)
		Input::Instance().SetMouseMode(dx::Mouse::MODE_RELATIVE);
	else
		Input::Instance().SetMouseMode(dx::Mouse::MODE_ABSOLUTE);

	if (this->cameraObject && this->rbComp && this->capsuleComp && this->camComp)
	{
		this->rbComp->LockRotation(true);
		this->rbComp->SetLinearDamping(0.1f);
		this->rbComp->GetRigidBody()->getCollider(0)->getMaterial().setBounciness(0.f);
		this->rbComp->EnableGravity(!this->freeCam);
		this->rbComp->SetLinearVelocity({ 0.f, 0.f, 0.f });
	}
	else 
	{
		std::cout << "component missing" << std::endl;
	}
}

void ControllerComp::Update(const float& deltaTime)
{
	Physics& phy = Physics::Instance();
	DirectX::XMFLOAT3 dir = { 0.f,0.f,0.f };
	this->fovTimer += deltaTime;
	this->velocityTimer += deltaTime; 
	this->crouchTimer += deltaTime;
	dx::XMVECTOR groundRotation;

	if (KEY_DOWN(D0))
	{
		dx::XMVECTOR resetPos = dx::XMLoadFloat3(&RESET_POS);
		GetOwner()->GetTransform().SetPosition(resetPos);
		rbComp->SetPosition(resetPos);
		rbComp->SetLinearVelocity({ 0.f, 0.f, 0.f });
	}
	if (KEY_DOWN(O))
	{
		this->showCursor = !this->showCursor;
		ShowCursor(this->showCursor);
	}
	if (KEY_DOWN(V))
	{
		this->freeCam = !this->freeCam;
		
		rbComp->SetLinearVelocity({ 0.f, 0.f, 0.f });
		rbComp->EnableGravity(!this->freeCam);
	}
	if (KEY_DOWN(F))
	{
		this->canRotate = !this->canRotate;
		rbComp->SetLinearVelocity({ 0.f, 0.f, 0.f });
		ShowCursor(!this->canRotate);
		if (this->canRotate)
			Input::Instance().SetMouseMode(dx::Mouse::MODE_RELATIVE);
		else
			Input::Instance().SetMouseMode(dx::Mouse::MODE_ABSOLUTE);
	}

	if (this->canRotate)
	{
		//Input::Instance().ConfineMouse();
		//SetCursorPos(400, 400); //set this to coordinates middle of screen? get height/width from input?

		float xPos = Input::Instance().GetMousePosRelative().x * deltaTime;
		float yPos = Input::Instance().GetMousePosRelative().y * deltaTime;
		xClamp += xPos;
		if (xClamp > CLAMP_X)
		{
			xClamp = CLAMP_X;
			xPos = 0.f;
		}
		if (xClamp < -CLAMP_X)
		{
			xClamp = -CLAMP_X;
			xPos = 0.f;
		}
		//rotate view
		Transform& transform = cameraObject->GetTransform();
		dx::XMVECTOR right = transform.TransformDirection({ 1,0,0 });
		dx::XMVECTOR eulerRotation = dx::XMQuaternionMultiply(dx::XMQuaternionRotationAxis(right, xPos), dx::XMQuaternionRotationAxis({ 0,1,0 }, yPos));

		//rotate walking direction to allign to ground
		groundRotation = dx::XMQuaternionMultiply(dx::XMQuaternionRotationAxis(right, 0), dx::XMQuaternionRotationAxis({ 0,1,0 }, yPos));
		groundRotation = dx::XMQuaternionMultiply(dx::XMLoadFloat4(&this->groundQuaterion), groundRotation);
		dx::XMStoreFloat4(&groundQuaterion, groundRotation); 

		transform.SetRotation(dx::XMQuaternionMultiply(transform.GetRotation(), eulerRotation)); //rotate view
		Input::Instance().ResetRelative();

		if (KEY_PRESSED(LeftShift)) //sprint
		{
			if (this->velocity < RUN_VELOCITY && this->velocityTimer >= VELOCITY_INC_RATE)
			{
				this->velocity += RUN_ACCELERATION;
				this->velocityTimer = 0.f;
			}
			if (this->fov < RUN_FOV && this->fovTimer >= FOV_INC_RATE)
			{
				this->fov += FOV_INC;
				this->fovTimer = 0.f;
			}
		}
		else
		{
			if (this->velocity > WALK_VELOCITY && this->velocityTimer >= VELOCITY_INC_RATE)
			{
				this->velocity -= RUN_ACCELERATION;
				this->velocityTimer = 0.f;
			}
			if (this->fov > WALK_FOV && this->fovTimer >= FOV_INC_RATE)
			{
				this->fov -= FOV_INC;
				this->fovTimer = 0.f;
			}
		}
		
		if (KEY_PRESSED(W) || KEY_PRESSED(S) || KEY_PRESSED(A) || KEY_PRESSED(D))
		{
			if (KEY_PRESSED(W))
				dir.z += 1.f;
			if (KEY_PRESSED(S))
				dir.z -= 1.f;
			if (KEY_PRESSED(A))
				dir.x -= 1.f;
			if (KEY_PRESSED(D))
				dir.x += 1.f;

			if (this->velocity < WALK_VELOCITY && velocityTimer >= VELOCITY_INC_RATE)
			{
				this->velocity += WALK_ACCELERATION;
				this->velocityTimer = 0.f;
			}
		}
		else
		{
			if (this->velocity > 0.f && velocityTimer >= VELOCITY_INC_RATE)
			{
				this->velocity -= WALK_ACCELERATION;
				this->velocityTimer = 0.f;
			}
		}
		
		if (freeCam) //flying camera
		{
			if (KEY_PRESSED(Space)) //Free cam
				dir.y += 1.f;
			if (KEY_PRESSED(C))
				dir.y -= 1.f;

			dx::XMVECTOR direction = dx::XMLoadFloat3(&dir);
			direction = dx::XMVector3Normalize(direction);

			phy.MutexLock();
			direction = cameraObject->GetTransform().TransformDirection(direction);
			direction = dx::XMVectorScale(direction, RUN_VELOCITY * deltaTime);
			dx::XMStoreFloat3(&dir, direction);
			dx::XMFLOAT3 vel = rbComp->GetLinearVelocity();
			cameraObject->GetTransform().SetPosition(rbComp->GetPosition());
			rbComp->SetLinearVelocity({ dir.x * VELOCITY_MULTIPLIER, dir.y * VELOCITY_MULTIPLIER, dir.z * VELOCITY_MULTIPLIER });
			dx::XMVECTOR capsule = dx::XMLoadFloat4(&RESET_ROT);
			rbComp->SetRotation(capsule);
			phy.MutexUnlock();
		}
		else //First Person specific actions
		{
			float jumpVelocity = 0;
			bool isCrouching = false;
			camComp->SetFOV(fov);
			if (IsGrounded() && KEY_DOWN(Space)) // FPcam //jump is  scuffed
			{
				jumpVelocity = JUMP_VELOCITY;
				//dir.y = JUMP_VELOCITY;
			}

			if (KEY_PRESSED(LeftControl)) //crouch is scuffed and outdated
			{
				isCrouching = true;
			}

			//std::cout << this->cameraOffset.y << std::endl;
			if (isCrouching)
			{
				if (this->velocity > CROUCH_VELOCITY && velocityTimer >= VELOCITY_INC_RATE)
				{
					this->velocity -= CROUCH_ACCELERATION;
					this->velocityTimer = 0.f;
				}
				if (this->cameraOffset.y > CROUCH_LIMIT && this->crouchTimer >= CROUCH_INC_RATE)
				{
					this->cameraOffset.y -= CROUCH_OFFSET_PER;
					this->crouchTimer = 0.f;
				}
			}
			else
			{
				if (this->velocity < WALK_VELOCITY && velocityTimer >= VELOCITY_INC_RATE)
				{
					this->velocity += CROUCH_ACCELERATION;
					this->velocityTimer = 0.f;
				}
				if (this->cameraOffset.y + CROUCH_OFFSET_PER < 0.0f && this->crouchTimer >= CROUCH_INC_RATE)
				{
					this->cameraOffset.y += CROUCH_OFFSET_PER;
					this->crouchTimer = 0.f;
				}
			}

			dx::XMVECTOR direction = dx::XMLoadFloat3(&dir);
			direction = dx::XMVector3Normalize(direction);

			phy.MutexLock();
			direction = cameraObject->GetTransform().TransformDirectionCustomRotation(direction, groundRotation);
			direction = dx::XMVectorScale(direction, this->velocity * deltaTime);
			dx::XMStoreFloat3(&dir, direction);
			dx::XMFLOAT3 vel = rbComp->GetLinearVelocity();
			dx::XMVECTOR cameraPos = dx::XMLoadFloat3(&this->cameraOffset);
			cameraPos = dx::XMVectorAdd(cameraPos, rbComp->GetPosition());
			cameraObject->GetTransform().SetPosition(cameraPos); //add camera-offset
			rbComp->SetLinearVelocity({ dir.x * VELOCITY_MULTIPLIER, vel.y + jumpVelocity, dir.z * VELOCITY_MULTIPLIER });
			dx::XMVECTOR capsule = dx::XMLoadFloat4(&RESET_ROT);
			//capsuleComp->SetRotation(capsule);
			

			rbComp->SetRotation(capsule);

			std::cout << "Cap, x: " << capsuleComp->GetTransform().getOrientation().x << ", y: " << capsuleComp->GetTransform().getOrientation().y << ", z: " << capsuleComp->GetTransform().getOrientation().z << ", w:" << capsuleComp->GetTransform().getOrientation().w << std::endl;
			dx::XMFLOAT4 rot;
			dx::XMStoreFloat4(&rot, rbComp->GetRotation());
			std::cout << "rb, x: " << rot.x << ", y:" << rot.y << ", z:" << rot.z << ", w:" << rot.w << std::endl;
			phy.MutexUnlock();
		}
	}
	else
	{
		phy.MutexLock();
		//	Input::Instance().FreeMouse();

		rbComp->SetLinearVelocity({ 0.f, 0.f, 0.f });
		dx::XMVECTOR capsule = dx::XMLoadFloat4(&RESET_ROT);
		rbComp->SetRotation(capsule);
		phy.MutexUnlock();
	}

#if NDEBUG 

	// fixes a bug in release where the compiler removes the variables
	// Later: fix these variables?

	//fov = 60.f;
	//fovTimer = 0.f;

	//velocity = 0.f;
	//velocityTimer = 0.f;
	//crouchTimer = 0.f;

	//xClamp = 0.f;
	//freeCam = true;
	//showCursor = true;
	//canRotate = false;

	//groundQuaterion = { 0.f,0.f,0.f,1.f };
	//cameraObject = nullptr;
	//rbComp = nullptr;
	//camComp = nullptr;
	//capsuleComp = nullptr;

#endif // NDEBUG 
}
