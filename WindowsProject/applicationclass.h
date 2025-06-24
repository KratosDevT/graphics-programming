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

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "d3dclass.h"
#include "cameraclass.h"
#include "textureshaderclass.h"
#include "bitmapclass.h"
#include "circleclass.h"
//#include "modelclass.h"


/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
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
	void Shutdown();
	bool Frame();

	void ResizeBitmap(int width, int height);
	void MoveBitmap(int x, int y);
	void UpdateCircle(float deltaTime);

private:
	bool Render();

private:
	D3DClass* m_Direct3D; //prefix m_ on all class variables.
	CameraClass* m_Camera;
	BitmapClass* m_Bitmap;
	//ModelClass* m_Model;
	TextureShaderClass* m_TextureShader;
	CircleClass* m_Circle;

};

#endif