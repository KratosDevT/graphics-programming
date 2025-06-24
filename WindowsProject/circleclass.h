#pragma once

#ifndef _BALLCLASS_H_
#define _BALLCLASS_H_

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

    bool Initialize(ID3D11Device*, ID3D11DeviceContext*, int, int, char*, float, float, float);
    void Shutdown();
    bool Render(ID3D11DeviceContext*);
    void Update(float deltaTime, int screenWidth, int screenHeight);

    int GetIndexCount();
    ID3D11ShaderResourceView* GetTexture();

    void UpdateWithCollision(float deltaTime, int screenWidth, int screenHeight,
        int rectX, int rectY, int rectWidth, int rectHeight);

    void Reset(float x, float y, float velX = 150.0f, float velY = -100.0f);

    // Getters per posizione e dimensioni
    float GetX() { return m_positionX; }
    float GetY() { return m_positionY; }
    float GetRadius() { return m_radius; }

    // Metodi per controllare le collisioni con i blocchetti
    void ReverseVelocityX() { m_velocityX = -m_velocityX; }
    void ReverseVelocityY() { m_velocityY = -m_velocityY; }

    // Getters per velocity (utili per debug)
    float GetVelocityX() { return m_velocityX; }
    float GetVelocityY() { return m_velocityY; }

private:
    bool InitializeBuffers(ID3D11Device*);
    void ShutdownBuffers();
    bool UpdateBuffers(ID3D11DeviceContext*);
    void RenderBuffers(ID3D11DeviceContext*);

    bool LoadTexture(ID3D11Device*, ID3D11DeviceContext*, char*);
    void ReleaseTexture();
    bool CheckCircleRectCollision(float circleX, float circleY, float radius,
        float rectLeft, float rectTop, float rectRight, float rectBottom);


private:
    ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
    int m_vertexCount, m_indexCount;
    TextureClass* m_Texture;

    // Proprietà del cerchio
    float m_positionX, m_positionY;    // Posizione del centro
    float m_velocityX, m_velocityY;    // Velocità di movimento
    float m_radius;                    // Raggio del cerchio
    int m_screenWidth, m_screenHeight; // Dimensioni dello schermo

    // Variabili per l'aggiornamento del buffer
    int m_prevPosX, m_prevPosY;
};

#endif