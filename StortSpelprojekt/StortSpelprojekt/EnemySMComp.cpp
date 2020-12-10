#include "stdafx.h"
#include "EnemySMComp.h"

EnemySMComp::EnemySMComp(EnemyState state) :
	currentState(state)
{
}

EnemySMComp::~EnemySMComp()
{
}

void EnemySMComp::InitAnimation(Object* playerObj)
{
	skeletonComponent = GetOwner()->GetComponent<SkeletonMeshComponent>();
	attackComponent = GetOwner()->GetComponent<EnemyAttackComp>();
	statsComponent = GetOwner()->GetComponent<EnemyStatsComp>();
	skeletonComponent->CreateCombinedAnimation(SkeletonStateMachine::RUN, SkeletonStateMachine::ATTACK, 5, 36);
	enemyPatrolComp = GetOwner()->GetComponent<EnemyPatrolComp>();
	player = playerObj;
}

void EnemySMComp::SetState(EnemyState state)
{
	if (currentState != EnemyState::NONE)
	{
		stateMap[currentState]->SetEnabled(false);
	}
	currentState = state;
	if (currentState != EnemyState::NONE)
	{
		stateMap[currentState]->SetEnabled(true);
	}
}

void EnemySMComp::Start()
{
	//enemyAttackComp = GetOwner()->GetComponent<EnemyAttackComp>();
	//skeletonComponent->CreateBlendedAnimation();
	
}

void EnemySMComp::Initialize()
{
	
}

void EnemySMComp::Animate()
{
	float length = 0;
	dx::XMVECTOR playerVec = player->GetTransform().GetPosition();
	dx::XMVECTOR enemyVec = GetOwner()->GetTransform().GetPosition();
	dx::XMFLOAT3 enemyPos;
	dx::XMStoreFloat3(&enemyPos, enemyVec);
	dx::XMStoreFloat(&length, dx::XMVector3Length(dx::XMVectorSubtract(playerVec, enemyVec)));

	if (statsComponent->GetHealth() <= 0.0f || length >= ENEMY_RADIUS_LIMIT)
	{
		SetState(EnemyState::IDLE);
		GetOwner()->GetComponent<Pathfinding>()->SetEnabled(false);
		
		skeletonComponent->SetTrack(SkeletonStateMachine::DEATH, true);

		if (skeletonComponent->GetDoneDeath())
		{
			if (statsComponent->GetHealth() <= 0.0f)
				MetaProgress::Instance().EnemyKilled(GetOwner()->GetName());

			std::cout << "enemy died.. health: " << statsComponent->GetHealth() << ", y: " << enemyPos.y <<", length from player: "<<length<< std::endl;
			statsComponent->GetManager()->RemoveEnemy(GetOwner());
		}


	}
	else
	{
		if (currentState == EnemyState::ATTACK)
		{
			skeletonComponent->PlayBlendAnimations(SkeletonStateMachine::RUN, SkeletonStateMachine::ATTACK, 1.0f);
			//skeletonComponent->SetTrack(SkeletonStateMachine::RUN, false);

			if (attackComponent->GetIsAttacking())
			{
				skeletonComponent->SetTrack(SkeletonStateMachine::ATTACK, false);
			}
		}
		else if (currentState == EnemyState::PATROL)
		{
			if (skeletonComponent->GetEnemyType() == EnemyType::BASE)
			{
				skeletonComponent->SetTrack(SkeletonStateMachine::WALK, false);
			}
			else
			{
				skeletonComponent->SetTrack(SkeletonStateMachine::RUN, false);
			}
			
		}

		else
		{
			skeletonComponent->SetTrack(SkeletonStateMachine::IDLE, false);
		}
	}
	

}

void EnemySMComp::Update(const float& deltaTime)
{
	
	
	if (currentState != EnemyState::ATTACK && attackComponent->ChasePlayer())
	{
		SetState(EnemyState::ATTACK);
		GetOwner()->GetComponent<Pathfinding>()->SetEnabled(true);
	}
	else if (currentState != EnemyState::IDLE && !attackComponent->ChasePlayer() && !enemyPatrolComp->GetIsMoving())
	{
		SetState(EnemyState::IDLE);
		GetOwner()->GetComponent<Pathfinding>()->SetEnabled(false);
	}
	else if (currentState != EnemyState::PATROL && !attackComponent->ChasePlayer() && enemyPatrolComp->GetIsMoving())
	{
		SetState(EnemyState::PATROL);
		GetOwner()->GetComponent<Pathfinding>()->SetEnabled(false);
	}
	
	if (skeletonComponent)
	{
		Animate();
	}
}

void EnemySMComp::RegisterState(EnemyState state, Component* comp)
{
	comp->SetEnabled(false);
	stateMap.insert({ state, comp });
}
