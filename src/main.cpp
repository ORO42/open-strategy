#include "raylib.h"
#include "game_context.h"
#include "ui_helpers.h"
#include "map_helpers.h"
#include "camera_helpers.h"
#include "unit_helpers.h"
#include "ability_helpers.h"
#include "networking_helpers.h"
#include "destruction_helpers.h"

#include "resource_dir.h" // utility header for SearchAndSetResourceDir

int main()
{
	std::srand(std::time(0));

	GameContext gameContext;

	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(gameContext.screenWidth, gameContext.screenHeight, gameContext.windowTitle.c_str());
	SetTargetFPS(60);

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	gameContext.LoadAndSetConfig();
	gameContext.LoadAllTextures();
	Startup(&gameContext);

	// game loop
	while (!WindowShouldClose()) // run the loop untill the user presses ESCAPE or presses the Close button on the window
	{
		ENetEvent event;
		while (enet_host_service(enetHost, &event, 0) > 0) // Non-blocking
		{
			switch (event.type)
			{
				// case ENET_EVENT_TYPE_CONNECT:
				// 	printf("connected to server----");

			case ENET_EVENT_TYPE_RECEIVE:
				printf("Received: %s\n", (char *)event.packet->data);
				enet_packet_destroy(event.packet);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				printf("Disconnected from server.\n");
				break;

			default:
				break;
			}
		}

		// update
		sCameraKeyInput(&gameContext);
		sUnitSelection(&gameContext);
		sCycleSelectedAbility(&gameContext);
		sMoveUnits(&gameContext);

		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(BLACK);

		sDrawGameTextures(&gameContext);
		sDrawIndicatorLine(&gameContext);
		sDrawSelectedUnitIndicator(&gameContext);
		sDrawHoveredCellIndicator(&gameContext);
		sUseAbilities(&gameContext);
		sDrawPopupText(&gameContext);
		sDrawPlayerDetails(&gameContext);
		sDrawUnitDetails(&gameContext);
		sDrawHoveredCellInfo(&gameContext);
		sDrawTargetingDetails(&gameContext);
		sDrawNextTurnTip(&gameContext);
		sDrawSelectedUnitAbilities(&gameContext);
		DrawFPS(gameContext.screenWidth / 2, gameContext.screenHeight / 2);

		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();

		sDestroyGameObjects(&gameContext);

		// if (SendENetMessage(enetPeer, "hello"))
		// {
		// 	printf("Message sent successfully.\n");
		// }
		// else
		// {
		// 	printf("Failed to send message.\n");
		// }
	}

	// cleanup
	// unload our texture so it can be cleaned up
	gameContext.UnloadAllTextures();

	// cleanup enet host
	CleanupEnetHost(enetHost);

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
