////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "applicationclass.h"


ApplicationClass::ApplicationClass()
{
	m_Direct3D = 0;
	m_Camera = 0;
	m_Bitmap = 0;
	//m_Model = 0;
	m_TextureShader = 0;
	m_Circle = 0;
}


ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
}


ApplicationClass::~ApplicationClass()
{
}


bool ApplicationClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	//char textureFilename[128];
	char bitmapFilename[128];
	char circleFilename[128];
	bool result;

	// Create and initialize the Direct3D object.
	m_Direct3D = new D3DClass;

	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	// Create the camera object.
	m_Camera = new CameraClass;

	// Set the initial position of the camera.
	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
	m_Camera->Render();

	// Create and initialize the texture shader object.
	m_TextureShader = new TextureShaderClass;

	result = m_TextureShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}

	// Set the name of the texture file that we will be loading.
	//strcpy_s(textureFilename, "bisio_oculus.tga");
	strcpy_s(bitmapFilename, "stone01.tga");
	strcpy_s(circleFilename, "circle.tga");

	//// Create and initialize the model object.
	//m_Model = new ModelClass;

	// Create and initialize the bitmap object.
	m_Bitmap = new BitmapClass;
	result = m_Bitmap->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), screenWidth, screenHeight, bitmapFilename, 350, 550);
	if (!result)
	{
		return false;
	}

	/*result = m_Model->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), screenWidth, screenHeight, textureFilename, 50, 50);
	
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}*/

	m_Circle = new CircleClass;
	result = m_Circle->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(),
		screenWidth, screenHeight, circleFilename,
		400.0f, 300.0f, 10.0f);  // centro schermo, raggio 30
	if (!result)
	{
		return false;
	}

	return true;
}


void ApplicationClass::Shutdown()
{

	// Release the texture shader object.
	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}

	//// Release the model object.
	//if (m_Model)
	//{
	//	m_Model->Shutdown();
	//	delete m_Model;
	//	m_Model = 0;
	//}
	// 
	// Release the bitmap object.
	if (m_Circle)
	{
		m_Circle->Shutdown();
		delete m_Circle;
		m_Circle = 0;
	}

	if (m_Bitmap)
	{
		m_Bitmap->Shutdown();
		delete m_Bitmap;
		m_Bitmap = 0;
	}

	// Release the camera object.
	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Release the Direct3D object.
	if (m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}
	return;
}


bool ApplicationClass::Frame()
{
	bool result;

	// Render the graphics scene.
	result = Render();
	if (!result)
	{
		return false;
	}
	return true;
}


bool ApplicationClass::Render()
{
	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;


	// Clear the buffers to begin the scene. black
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);

	// Turn off the Z buffer to begin all 2D rendering.
	m_Direct3D->TurnZBufferOff();

	//// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	//result = m_Model->Render(m_Direct3D->GetDeviceContext());
	//if (!result)
	//{
	//	return false;
	//}
	// Put the bitmap vertex and index buffers on the graphics pipeline to prepare them for drawing.
	result = m_Bitmap->Render(m_Direct3D->GetDeviceContext());
	if (!result)
	{
		return false;
	}

	
	result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_Bitmap->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix, m_Bitmap->GetTexture());

	if (!result)
	{
		return false;
	}

	result = m_Circle->Render(m_Direct3D->GetDeviceContext());
	if (!result)
	{
		return false;
	}

	result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_Circle->GetIndexCount(),
		worldMatrix, viewMatrix, orthoMatrix, m_Circle->GetTexture());
	if (!result)
	{
		return false;
	}

	// Turn the Z buffer back on now that all 2D rendering has completed.
	m_Direct3D->TurnZBufferOn();

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

	return true;
}

void ApplicationClass::ResizeBitmap(int width, int height)
{
	if (m_Bitmap)
	{
		m_Bitmap->SetCustomSize(width, height);
	}
}

void ApplicationClass::MoveBitmap(int x, int y)
{
	if (m_Bitmap)
	{
		m_Bitmap->SetRenderLocation(x, y);
	}
}

void ApplicationClass::UpdateCircle(float deltaTime)
{
	if (m_Circle && m_Bitmap)
	{
		// Get bitmap position and dimensions
		int bitmapX = m_Bitmap->GetRenderX();
		int bitmapY = m_Bitmap->GetRenderY();
		int bitmapWidth = m_Bitmap->GetCurrentWidth();
		int bitmapHeight = m_Bitmap->GetCurrentHeight();

		// Update circle with collision detection
		m_Circle->UpdateWithCollision(deltaTime, 800, 600,
			bitmapX, bitmapY, bitmapWidth, bitmapHeight);
	}
}