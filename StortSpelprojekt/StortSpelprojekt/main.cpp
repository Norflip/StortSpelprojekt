#include "Engine.h"
#include "BusNode.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	Engine engine (hInstance);
	engine.Run();


	// kolla �ver
	//MessageBus messageBus;
	//ComponentA compA(&messageBus);
	//ComponentB compB(&messageBus);

	//// This is supposed to act like a game loop.
	//for (int ctr = 0; ctr < 10; ctr++) 
	//{
	//	compA.Update();
	//	compB.Update();
	//
	//	messageBus.Notify();
	//}	

	return 0;
}
