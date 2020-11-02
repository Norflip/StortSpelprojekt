#pragma once
#include "SkeletonMeshComponent.h"
#include "Path.h"
#include "Input.h"
// why are these global?
constexpr float HEIGHT = 1.f; //height from ground
const float OFFSET = CHUNK_SIZE / 2.0f;

namespace dx = DirectX;
class NodeWalkerComp :public Component
{
public:
	//struct Node
	//{
	//	std::string name;
	//	int id;
	//	dx::XMFLOAT3 position;
	//	int nextMiddle;
	//	int nextLeft;
	//	int nextRight;
	//	int next;
	//	Node(std::string name, int id, dx::XMFLOAT3 position, int nextMiddle, int nextLeft, int nextRight)
	//	{
	//		this->name = name;
	//		this->id = id;
	//		this->position = position;
	//		this->nextMiddle = nextMiddle;
	//		this->nextLeft = nextLeft;
	//		this->nextRight = nextRight;
	//	}
	//};
private:
	//std::vector<Node> nodes;
	Path thePath;
	float speed;
	int currentNode;
	int nextChosen;
	float nodeRadius;
	bool canWalk;
	float length; 
	dx::XMFLOAT3 lastPos;
	SkeletonMeshComponent* base;
	SkeletonMeshComponent* legs;
	RigidBodyComponent* rbComp;
public:
	NodeWalkerComp();
	virtual ~NodeWalkerComp();
	void InitializePath(Path thePath);
	void InitAnimation();
	void Reset(); //is public cus is meant to be called outside of class if needed
	void Start();
	void Stop();

	void Update(const float& deltaTime);
	dx::XMFLOAT3 GetLastPos();

	//void SetPosition(dx::XMVECTOR pos);

};

