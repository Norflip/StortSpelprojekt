#include "Physics.h"

Physics::Physics() : collisionConfiguration(nullptr), dispatcher(nullptr), overlappingPairCache(nullptr)
{
}

Physics::~Physics()
{
}

void Physics::Initialize(dx::XMFLOAT3 gravity)
{
	CreateDynamicWorld();
	SetGravity(gravity.x, gravity.y, gravity.z);
}

void Physics::SetGravity(float x, float y, float z)
{
	this->gravity = btVector3(x, y, z);
    dynamicsWorld->setGravity(this->gravity);
}

void Physics::CreateDynamicWorld()
{
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	overlappingPairCache = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
}

void Physics::RegisterRigidBody(RigidBodyComp* rigidBodyComp)
{
	assert(dynamicsWorld != nullptr);

	rigidBodyComp->m_InitializeBody();
	int group = static_cast<int>(rigidBodyComp->GetGroup());
	dynamicsWorld->addRigidBody(rigidBodyComp->GetRigidBody(), group, 0);

	bodyMap.insert({ rigidBodyComp->GetOwner()->GetID(), rigidBodyComp });
}

void Physics::UnregisterRigidBody(Object* object)
{
	auto find = bodyMap.find(object->GetID());
	if (find != bodyMap.end())
		bodyMap.erase(find);
}

void Physics::UnregisterRigidBody(RigidBodyComp* rigidBodyComp)
{
	UnregisterRigidBody(rigidBodyComp->GetOwner());
}

void Physics::FixedUpdate(const float& fixedDeltaTime)
{
	
	

	const float internalTimeStep = 1. / 240.f;
	dynamicsWorld->stepSimulation(fixedDeltaTime, 4, internalTimeStep);

	// neccesary? 
	dynamicsWorld->updateAabbs();
	dynamicsWorld->computeOverlappingPairs();

	for (auto i : bodyMap)
	{
		if (i.second->GetOwner()->HasFlag(ObjectFlag::ENABLED))
		{
			i.second->UpdateWorldTransform(dynamicsWorld);
		}
	}

	CheckForCollisions();
}

RayHit Physics::RaytestSingle(const Ray& ray, float maxDistance, PhysicsGroup layer) const
{
	//std::cout << "origin: " << ray.origin.x << ", " << ray.origin.y << ", " << ray.origin.z << std::endl;
	//std::cout << "direction: " << ray.direction.x << ", " << ray.direction.y << ", " << ray.direction.z << std::endl;

	Ray r = ray;
	//r.direction = { 0,0,1 };

	dx::XMFLOAT3 furthestPoint = r.GetPoint(maxDistance);
	btVector3 from = btVector3(r.origin.x, r.origin.y, r.origin.z);
	btVector3 to = btVector3(furthestPoint.x, furthestPoint.y, furthestPoint.z);

	//btCollisionWorld::AllHitsRayResultCallback allResults(from, to);
	//allResults.m_flags |= btTriangleRaycastCallback::kF_KeepUnflippedNormal;

	btCollisionWorld::ClosestRayResultCallback closestResults(from, to);
	closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
	closestResults.m_collisionFilterGroup = static_cast<int>(layer);
	closestResults.m_collisionFilterMask = -1;

	dynamicsWorld->rayTest(from, to, closestResults);
	bool didHit = closestResults.hasHit();
	RayHit hit;
	hit.ray = ray;

	if (didHit)
	{
		btVector3 position = from.lerp(to, closestResults.m_closestHitFraction);
		btVector3 normal = closestResults.m_hitNormalWorld;
		//dynamicsWorld->getDebugDrawer()->drawSphere(p, 0.1, btVector3(0,0,1));
		//dynamicsWorld->getDebugDrawer()->drawLine(p, p + closestResults.m_hitNormalWorld, btVector3(0, 0, 1));
			
		void* userPointer = closestResults.m_collisionObject->getUserPointer();
		hit.body = static_cast<RigidBodyComp*>(userPointer);
		hit.position = ToXMFLOAT3(position);// { position.getX(), position.getY(), position.getZ() };
		hit.normal = ToXMFLOAT3(normal);// { normal.getX(), normal.getY(), normal.getZ() };
	}

	hit.hit = didHit;
	return hit;
}

void Physics::CheckForCollisions()
{
	collisions.clear();
	int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
	for (int i = 0; i < numManifolds; i++)
	{
		btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);

		const btCollisionObject* objA = contactManifold->getBody0();
		const btCollisionObject* objB = contactManifold->getBody1();

		CollisionInfo tmp;
		RigidBodyComp* rbA = static_cast<RigidBodyComp*>(objA->getUserPointer());
		RigidBodyComp* rbB = static_cast<RigidBodyComp*>(objB->getUserPointer());

		if (rbA)
		{
			tmp.other = rbB;
			rbA->m_OnCollision(tmp);
		}

		if (rbB)
		{
			tmp.other = rbA;
			rbB->m_OnCollision(tmp);
		}


		/*int numContacts = contactManifold->getNumContacts();
		for (int j = 0; j < numContacts; j++)
		{
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			
			const btVector3& ptA = pt.getPositionWorldOnA();
			const btVector3& ptB = pt.getPositionWorldOnB();
			const btVector3& normalOnB = pt.m_normalWorldOnB;
			collisions.push_back(ptA);
			collisions.push_back(ptB);
			collisions.push_back(normalOnB);
		}*/
	}
}






