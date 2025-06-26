////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.cpp
////////////////////////////////////////////////////////////////////////////////

#include "applicationclass.h"
#include <cmath>      

ApplicationClass::ApplicationClass()
{
	m_Direct3D = 0;
	m_Camera = 0;
	m_Bitmap = 0;
	m_TextureShader = 0;
	m_Circle = 0;

	m_ballAttachedToPaddle = true;   
	m_gameStarted = false;
	m_gameWon = false;
	m_ballOffsetFromPaddle = INIT_OFFSET_BALL_FROM_PADDLE;
	
	m_FontStart = 0;
	m_FontBricks = 0;
	m_FontVictory = 0;
	m_showStartText = true;
	m_showEndText = false;

	for (int row = 0; row < BRICK_ROWS; row++)
	{
		for (int col = 0; col < BRICK_COLS; col++)
		{
			m_Bricks[row][col] = 0;
		}
	}
}

ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
}

ApplicationClass::~ApplicationClass()
{
}

bool ApplicationClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;

	m_Direct3D = new D3DClass;
	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	m_Camera = new CameraClass;
	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
	m_Camera->Render();

	m_TextureShader = new TextureShaderClass;
	result = m_TextureShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}

	char paddleTextureFilename[128];
	strcpy_s(paddleTextureFilename, "paddle.tga");
	m_Bitmap = new BitmapClass;
	result = m_Bitmap->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), screenWidth, screenHeight, paddleTextureFilename, 350, 550);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the Bitmap object.", L"Error", MB_OK);
		return false;
	}

	char circleFilename[128];
	strcpy_s(circleFilename, "circle.tga");
	m_Circle = new CircleClass;
	
	float paddleX = 600.0f;  
	float paddleY = 450.0f;
	float ballStartX = (float)paddleX + m_Bitmap->GetWidth()/2;
	float ballStartY = (float)paddleY - m_ballOffsetFromPaddle;

	result = m_Circle->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(),
		screenWidth, screenHeight, circleFilename,
		ballStartX, ballStartY, 5.0f, 0.0f, 0.0f); 
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the Circle object.", L"Error", MB_OK);
		return false;
	}

	char brickFilename[128];
	strcpy_s(brickFilename, "block.tga");
	result = InitializeBlocksGrid(screenWidth, screenHeight, brickFilename, DISTANCE_BRICK_FROM_TOP);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the blocks grid.", L"Error", MB_OK);
		return false;
	}

	// Carica "PRESS SPACE TO START"
	char startFontFile[128];
	strcpy_s(startFontFile, "font_start.tga");
	m_FontStart = new FontClass;
	result = m_FontStart->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(),
		screenWidth, screenHeight, startFontFile, 100, 300);
	if (!result)
	{
		OutputDebugStringA("Warning: Could not load font_start.tga - continuing without start text\n");
		delete m_FontStart;
		m_FontStart = 0;
	}
	else
	{
		OutputDebugStringA("SUCCESS: font_start.tga loaded\n");
	}

	// Carica "BRICKS: XX"
	char bricksFontFile[128];
	strcpy_s(bricksFontFile, "font_bricks.tga");
	m_FontBricks = new FontClass;
	result = m_FontBricks->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(),
		screenWidth, screenHeight, bricksFontFile, 20, 20);
	if (!result)
	{
		OutputDebugStringA("Warning: Could not load font_bricks.tga - continuing without brick counter\n");
		delete m_FontBricks;
		m_FontBricks = 0;
	}
	else
	{
		OutputDebugStringA("SUCCESS: font_bricks.tga loaded\n");
	}

	// Carica font per la schermata di vittoria
	char victoryFontFile[128];
	strcpy_s(victoryFontFile, "font_victory.tga");
	m_FontVictory = new FontClass;
	result = m_FontVictory->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(),
		screenWidth, screenHeight, victoryFontFile, 200, 150);
	if (!result)
	{
		OutputDebugStringA("Warning: Could not load font_victory.tga - continuing without victory screen\n");
		delete m_FontVictory;
		m_FontVictory = 0;
	}
	else
	{
		OutputDebugStringA("SUCCESS: font_victory.tga loaded\n");
	}

	return true;
}

void ApplicationClass::LaunchBall()
{
	if (!m_ballAttachedToPaddle || !m_Circle)
		return;

	// Imposta velocità di lancio
	float launchVelX = 100.0f;   // Velocità orizzontale
	float launchVelY = -400.0f;  // Velocità verticale (verso l'alto)


	// randomizza leggermente la direzione
	float randomAngle = (rand() % 60 - 30) * 0.0174532925f;  // ±30 gradi in radianti
	float speed = sqrt(launchVelX * launchVelX + launchVelY * launchVelY);
	launchVelX = speed * sin(randomAngle);
	launchVelY = -speed * cos(randomAngle);  // Sempre verso l'alto
	

	// Lancia la palla
	float currentX = m_Circle->GetX();
	float currentY = m_Circle->GetY();
	m_Circle->Reset(currentX, currentY, launchVelX, launchVelY);

	// Aggiorna stato
	m_ballAttachedToPaddle = false;
	m_gameStarted = true;

	// Debug
	char debugMsg[128];
	sprintf_s(debugMsg, "Palla lanciata! Pos:(%.1f,%.1f) Vel:(%.1f,%.1f)\n",
		currentX, currentY, launchVelX, launchVelY);
	OutputDebugStringA(debugMsg);
}



bool ApplicationClass::InitializeBlocksGrid(int screenWidth, int screenHeight, char  brickFilename[128], int distanceFromTop)
{
	bool bresult;
	int totalGridWidth = (BRICK_COLS * BRICK_WIDTH) + ((BRICK_COLS - 1) * BRICK_SPACING);
	int totalGridHeight = (BRICK_ROWS * BRICK_HEIGHT) + ((BRICK_ROWS - 1) * BRICK_SPACING);

	int offestLeft = (screenWidth - totalGridWidth) / 2;  // Margine sinistro
	int offestUp = distanceFromTop;  // Margine superiore che decido io

	m_remainingBricks = 0;

	for (int row = 0; row < BRICK_ROWS; row++)
	{
		for (int col = 0; col < BRICK_COLS; col++)
		{
			// Calculate position for this brick
			int brickX = offestLeft + col * (BRICK_WIDTH + BRICK_SPACING);
			int brickY = offestUp + row * (BRICK_HEIGHT + BRICK_SPACING);

			// Create and initialize the brick
			m_Bricks[row][col] = new BrickClass;

			bresult = m_Bricks[row][col]->Initialize(m_Direct3D->GetDevice(),m_Direct3D->GetDeviceContext(),screenWidth, screenHeight, brickFilename,
				brickX, brickY,BRICK_WIDTH, BRICK_HEIGHT);

			if (!bresult)
			{
				return false;
			}

			m_remainingBricks++;
		}
	}
	
	return true;
}

void ApplicationClass::AttachBallToPaddle()
{
	if (!m_Circle || !m_Bitmap)
		return;

	// Calcola posizione sopra il paddle
	int paddleX = m_Bitmap->GetRenderX();
	int paddleY = m_Bitmap->GetRenderY();
	int paddleWidth = m_Bitmap->GetWidth();

	float ballX = (float)paddleX + (float)paddleWidth / 2.0f;  // Centro paddle
	float ballY = (float)paddleY - m_ballOffsetFromPaddle;     // Sopra il paddle

	// Posiziona e ferma la palla
	m_Circle->Reset(ballX, ballY, 0.0f, 0.0f);  // Velocità = 0

	// Aggiorna stato
	m_ballAttachedToPaddle = true;
	m_gameStarted = false;

	OutputDebugStringA("Palla attaccata al paddle\n");
}

void ApplicationClass::Shutdown()
{
	if (m_FontVictory)
	{
		m_FontVictory->Shutdown();
		delete m_FontVictory;
		m_FontVictory = 0;
	}

	if (m_FontStart)
	{
		m_FontStart->Shutdown();
		delete m_FontStart;
		m_FontStart = 0;
	}

	if (m_FontBricks)
	{
		m_FontBricks->Shutdown();
		delete m_FontBricks;
		m_FontBricks = 0;
	}

	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}

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

	for (int row = 0; row < BRICK_ROWS; row++)
	{
		for (int col = 0; col < BRICK_COLS; col++)
		{
			if (m_Bricks[row][col])
			{
				m_Bricks[row][col]->Shutdown();
				delete m_Bricks[row][col];
				m_Bricks[row][col] = 0;
			}
		}
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

	for (int row = 0; row < BRICK_ROWS; row++)
	{
		for (int col = 0; col < BRICK_COLS; col++)
		{
			if (m_Bricks[row][col] && m_Bricks[row][col]->IsVisible())
			{
				result = m_Bricks[row][col]->Render(m_Direct3D->GetDeviceContext());
				if (!result)
				{
					return false;
				}

				result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(),
					m_Bricks[row][col]->GetIndexCount(),
					worldMatrix, viewMatrix, orthoMatrix,
					m_Bricks[row][col]->GetTexture());
				if (!result)
				{
					return false;
				}
			}
		}
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


	// Render "PRESS SPACE TO START" solo se palla è attaccata
	if (m_FontStart && m_ballAttachedToPaddle)
	{
		result = m_FontStart->Render(m_Direct3D->GetDeviceContext());
		if (result)
		{
			result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(),
				m_FontStart->GetIndexCount(),
				worldMatrix, viewMatrix, orthoMatrix,
				m_FontStart->GetTexture());
			if (!result) return false;
		}
	}

	// Render "BRICKS: XX" sempre
	if (m_FontBricks)
	{
		result = m_FontBricks->Render(m_Direct3D->GetDeviceContext());
		if (result)
		{
			result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(),
				m_FontBricks->GetIndexCount(),
				worldMatrix, viewMatrix, orthoMatrix,
				m_FontBricks->GetTexture());
			if (!result) return false;
		}
	}

	// Render font di vittoria (contiene sia "YOU WIN!" che "PRESS R TO RESTART")
	if (m_FontVictory && m_gameWon)
	{
		result = m_FontVictory->Render(m_Direct3D->GetDeviceContext());
		if (result)
		{
			result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(),
				m_FontVictory->GetIndexCount(),
				worldMatrix, viewMatrix, orthoMatrix,
				m_FontVictory->GetTexture());
			if (!result) return false;
		}
	}

	// Turn the Z buffer back on now that all 2D rendering has completed.
	m_Direct3D->TurnZBufferOn();

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();
	return true;
}

void ApplicationClass::UpdateBall(float deltaTime)
{
	if (!m_Circle || !m_Bitmap)
		return;

	if (m_ballAttachedToPaddle)
	{
		int paddleX = m_Bitmap->GetRenderX();
		int paddleY = m_Bitmap->GetRenderY();
		int paddleWidth = m_Bitmap->GetWidth();

		float ballX = (float)paddleX + (float)paddleWidth / 2.0f;
		float ballY = (float)paddleY - m_ballOffsetFromPaddle;

		m_Circle->Reset(ballX, ballY, 0.0f, 0.0f);
	}
	else
	{
		CheckCollisionsBallWithBricks();
		
		int bitmapX = m_Bitmap->GetRenderX();
		int bitmapY = m_Bitmap->GetRenderY();
		int bitmapWidth = m_Bitmap->GetWidth();
		int bitmapHeight = m_Bitmap->GetHeight();

		m_Circle->UpdateWithCollision(deltaTime, 800, 600,
			bitmapX, bitmapY, bitmapWidth, bitmapHeight);

		CheckGameWon();
		if (m_Circle->GetY() > 600)
		{
			char statusMsg[256];
			sprintf_s(statusMsg, "GAME OVER, Palla uscita! Brick rimanenti: %d\n", m_remainingBricks);
			OutputDebugStringA(statusMsg);
			ResetGame();
		}
	}
}

void ApplicationClass::CheckCollisionsBallWithBricks()
{
	if (!m_Circle)
		return;

	float ballX = m_Circle->GetX();
	float ballY = m_Circle->GetY();
	float ballRadius = m_Circle->GetRadius();

	float velX = m_Circle->GetVelocityX();
	float velY = m_Circle->GetVelocityY();

	for (int row = 0; row < BRICK_ROWS; row++)
	{
		for (int col = 0; col < BRICK_COLS; col++)
		{
			BrickClass* brick = m_Bricks[row][col];

			if (brick && brick->IsVisible())
			{
				// Get brick boundaries
				float brickLeft = (float)brick->GetX();
				float brickRight = brickLeft + (float)brick->GetWidth();
				float brickTop = (float)brick->GetY();
				float brickBottom = brickTop + (float)brick->GetHeight();
				
				// Check collision
				float closestX = max(brickLeft, min(ballX, brickRight));
				float closestY = max(brickTop, min(ballY, brickBottom));

				float distanceX = ballX - closestX;
				float distanceY = ballY - closestY;
				float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);

				if (distanceSquared < (ballRadius * ballRadius))
				{
					// Nascondi mattone
					brick->Hide();
					m_remainingBricks--;

					// Calcola dove era la palla nel frame precedente
					float deltaTime = 0.016f;  // Approssimazione a 60fps
					float prevBallX = ballX - velX * deltaTime;
					float prevBallY = ballY - velY * deltaTime;

					// Determina da che lato è ARRIVATA la palla
					bool comingFromLeft = (prevBallX + ballRadius <= brickLeft) && (velX > 0);
					bool comingFromRight = (prevBallX - ballRadius >= brickRight) && (velX < 0);
					bool comingFromTop = (prevBallY + ballRadius <= brickTop) && (velY > 0);
					bool comingFromBottom = (prevBallY - ballRadius >= brickBottom) && (velY < 0);

					// Debug per capire cosa succede
					char debugMsg[256];
					sprintf_s(debugMsg, "Collisione! Vel:(%.1f,%.1f) From: L:%d R:%d T:%d B:%d",
						velX, velY, comingFromLeft, comingFromRight, comingFromTop, comingFromBottom);
					OutputDebugStringA(debugMsg);

					// Applica rimbalzo basato sulla direzione di arrivo
					if (comingFromLeft || comingFromRight)
					{
						// Colpito dai lati → rimbalza orizzontalmente
						m_Circle->ReverseVelocityX();
						sprintf_s(debugMsg, "Rimbalzo ORIZZONTALE (dai lati)\n");
						OutputDebugStringA(debugMsg);
					}
					else if (comingFromTop || comingFromBottom)
					{
						// Colpito da sopra/sotto → rimbalza verticalmente  
						m_Circle->ReverseVelocityY();
						sprintf_s(debugMsg, "Rimbalzo VERTICALE (da sopra/sotto)\n");
						OutputDebugStringA(debugMsg);
					}
					else
					{
						// Collisione ambigua o angolare
						float distToLeft = std::fabs(ballX - brickLeft);
						float distToRight = std::fabs(ballX - brickRight);
						float distToTop = std::fabs(ballY - brickTop);
						float distToBottom = std::fabs(ballY - brickBottom);

						float minDistHoriz = min(distToLeft, distToRight);
						float minDistVert = min(distToTop, distToBottom);

						if (minDistHoriz < minDistVert)
						{
							m_Circle->ReverseVelocityX();
							sprintf_s(debugMsg, "Rimbalzo ORIZZONTALE (fallback)\n");
						}
						else
						{
							m_Circle->ReverseVelocityY();
							sprintf_s(debugMsg, "Rimbalzo VERTICALE (fallback)\n");
						}
						OutputDebugStringA(debugMsg);
					}

					return;
				}
			}
		}
	}
}

bool ApplicationClass::IsGameOver()
{
	if (!m_Circle)
		return false;

	return (m_gameStarted && !m_ballAttachedToPaddle && m_Circle->GetY() > 600);
}

void ApplicationClass::CheckGameWon()
{
	bool won = (m_remainingBricks <= 0);
	if (won && !m_gameWon)
	{
		// Prima volta che vince - imposta lo stato
		m_gameWon = true;
		OutputDebugStringA("VITTORIA! Tutti i brick sono stati distrutti!\n");
	}
}

int ApplicationClass::GetRemainingBricks()
{
	return m_remainingBricks;
}

void ApplicationClass::ResetGame()
{
	// Reset mattoni
	m_remainingBricks = 0;
	for (int row = 0; row < BRICK_ROWS; row++)
	{
		for (int col = 0; col < BRICK_COLS; col++)
		{
			if (m_Bricks[row][col])
			{
				m_Bricks[row][col]->SetVisible(true);
				m_remainingBricks++;
			}
		}
	}

	// Reset palla al paddle
	AttachBallToPaddle();
	m_gameWon = false;
	OutputDebugStringA("Gioco resettato - premi SPAZIO per iniziare\n");
}

void ApplicationClass::MovePaddleLeft()
{
	if (!m_Bitmap) return;

	int currentX = m_Bitmap->GetRenderX();
	int newX = currentX - 10;  

	if (newX >= 0)
	{
		m_Bitmap->SetRenderLocation(newX, m_Bitmap->GetRenderY());
	}
}

void ApplicationClass::MovePaddleRight()
{
	if (!m_Bitmap) return;

	int currentX = m_Bitmap->GetRenderX();
	int currentWidth = m_Bitmap->GetWidth();
	int newX = currentX + 10;

	if (newX + currentWidth <= 800)  
	{
		m_Bitmap->SetRenderLocation(newX, m_Bitmap->GetRenderY());
	}
}

void ApplicationClass::UpdateFontVisibility()
{
	m_showStartText = m_ballAttachedToPaddle;

	char debugMsg[128];
	sprintf_s(debugMsg, "Font visibility: Start=%s, Bricks=%d\n",
		m_showStartText ? "VISIBLE" : "HIDDEN", m_remainingBricks);
	OutputDebugStringA(debugMsg);
}
