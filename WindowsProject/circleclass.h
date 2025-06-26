#pragma once

#ifndef _CIRCLECLASS_H_
#define _CIRCLECLASS_H_

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

#include "textureclass.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: CircleClass
////////////////////////////////////////////////////////////////////////////////
class CircleClass
{
private:
    struct VertexType
    {
        XMFLOAT3 position;
        XMFLOAT2 texture;
    };

public:
    CircleClass();
    CircleClass(const CircleClass&);
    ~CircleClass();

    bool Initialize(ID3D11Device*, ID3D11DeviceContext*, int, int, char*, float, float, float, float ,float);

    void UpdateWithCollision(float deltaTime, int screenWidth, int screenHeight, int rectX, int rectY, int rectWidth, int rectHeight);
    void BounceFromPaddle(float& nextX, float& nextY);
    bool CheckCollisionWithScreenBoundary(float& nextX, float& nextY, int screenWidth);

    bool Render(ID3D11DeviceContext*);

    void Reset(float x, float y, float velX = 150.0f, float velY = -100.0f);
    void Shutdown();

    int GetIndexCount();
    ID3D11ShaderResourceView* GetTexture();

    float GetX() { return m_positionX; }
    float GetY() { return m_positionY; }
    float GetRadius() { return m_radius; }
    
    void ReverseVelocityX() { m_velocityX = -m_velocityX; }
    void ReverseVelocityY() { m_velocityY = -m_velocityY; }

    float GetVelocityX() { return m_velocityX; }
    float GetVelocityY() { return m_velocityY; }

private:
    bool InitializeBuffers(ID3D11Device*);
    void ShutdownBuffers();
    bool UpdateBuffers(ID3D11DeviceContext*);
    void RenderBuffers(ID3D11DeviceContext*);
    bool LoadTexture(ID3D11Device*, ID3D11DeviceContext*, char*);
    void ReleaseTexture();
    bool CheckCircleRectCollision(float circleX, float circleY, float radius, float rectLeft, float rectTop, float rectRight, float rectBottom);

private:
    ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
    int m_vertexCount, m_indexCount;
    TextureClass* m_Texture;

    // Proprietà del cerchio
    float m_positionX, m_positionY;    // Posizione del centro
    float m_velocityX, m_velocityY;    // Velocità di movimento
    float m_radius;                    // Raggio del cerchio
    int m_screenWidth, m_screenHeight; // Dimensioni dello schermo
    int m_prevPosX, m_prevPosY;
};

#endif