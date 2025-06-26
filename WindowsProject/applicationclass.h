#pragma once

////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _APPLICATIONCLASS_H_
#define _APPLICATIONCLASS_H_

//////////////
// INCLUDES //
//////////////
//#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "d3dclass.h"
#include "cameraclass.h"
#include "textureshaderclass.h"
#include "bitmapclass.h"
#include "circleclass.h"
#include "brickclass.h"
#include "fontclass.h"

/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const int SCREEN_WINDOW_WIDTH = 800;
const int SCREEN_WINDOW_HEIGHT = 600;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.3f;

////////////////////////////////////////////////////////////////////////////////
// Class name: ApplicationClass
////////////////////////////////////////////////////////////////////////////////
// ApplicationClass is the main class that is used to render the scene by invoking all the needed class objects for the project.

class ApplicationClass
{
public:
	ApplicationClass();
	ApplicationClass(const ApplicationClass&);
	~ApplicationClass();

	bool Initialize(int, int, HWND);
	bool InitializeBlocksGrid(int screenWidth, int screenHeight, char  brickFilename[128], int distanceFromTop);

	void Update(float deltaTime);
	bool Frame();

	void Shutdown();

	void MovePaddleLeft(float deltaTime);
	void MovePaddleRight(float deltaTime);

	void LaunchBall();
	void AttachBallToPaddle();
	bool IsBallAttached() { return m_ballAttachedToPaddle; }

	void CheckCollisionsBallWithBricks();
	int GetRemainingBricks();

	bool IsGameWon() { return m_gameWon; }
	void ResetGame();

	void UpdateFontVisibility();

private:
	bool Render();
	void UpdateBallAttachedToPaddle();
	void UpdateBallInMotion(float deltaTime);
	void CheckGameWon();
	void CheckGameOver();

private:
	//prefix m_ on all class variables.

	D3DClass* m_Direct3D; 
	CameraClass* m_Camera;
	BitmapClass* m_Bitmap;
	TextureShaderClass* m_TextureShader;
	int m_remainingBricks;
	CircleClass* m_Circle;

	static const int BRICK_ROWS = 4;
	static const int BRICK_COLS = 10;
	static const int BRICK_WIDTH = 70;
	static const int BRICK_HEIGHT = 25;
	static const int BRICK_SPACING = 5;
	static const int DISTANCE_BRICK_FROM_TOP = 60;
	static const int INIT_OFFSET_BALL_FROM_PADDLE = 15;

	BrickClass* m_Bricks[BRICK_ROWS][BRICK_COLS];

	bool m_ballAttachedToPaddle;    
	bool m_gameStarted;    
	bool m_gameWon;

	float m_ballOffsetFromPaddle;

	//per il testo a schermo
	FontClass* m_FontStart;        // "PRESS SPACE TO START"
	FontClass* m_FontBricks;       // "BRICKS: XX" da fare
	FontClass* m_FontVictory;	  // "YOU WON!" press r to restart
	bool m_showStartText;
	bool m_showEndText;
};

#endif