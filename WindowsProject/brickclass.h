#pragma once
////////////////////////////////////////////////////////////////////////////////
// Filename: brickclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _BRICKCLASS_H_
#define _BRICKCLASS_H_

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

#include "textureclass.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: BrickClass
////////////////////////////////////////////////////////////////////////////////
class BrickClass
{
private:
    struct VertexType
    {
        XMFLOAT3 position;
        XMFLOAT2 texture;
    };

public:
    BrickClass();
    BrickClass(const BrickClass&);
    ~BrickClass();

    bool Initialize(ID3D11Device*, ID3D11DeviceContext*, int, int, char*, int, int, int, int);
    void Shutdown();
    bool Render(ID3D11DeviceContext*);

    int GetIndexCount();
    ID3D11ShaderResourceView* GetTexture();

    // Getters per collision detection
    int GetX() { return m_posX; }
    int GetY() { return m_posY; }
    int GetWidth() { return m_width; }
    int GetHeight() { return m_height; }
    bool IsVisible() { return m_visible; }

    // Setter per nascondere il blocchetto
    void SetVisible(bool visible) { m_visible = visible; }
    void Hide() { m_visible = false; }

private:
    bool InitializeBuffers(ID3D11Device*);
    void ShutdownBuffers();
    bool UpdateBuffers(ID3D11DeviceContext*);
    void RenderBuffers(ID3D11DeviceContext*);

    bool LoadTexture(ID3D11Device*, ID3D11DeviceContext*, char*);
    void ReleaseTexture();

private:
    ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
    int m_vertexCount, m_indexCount;
    TextureClass* m_Texture;

    // Proprietà del blocchetto
    int m_posX, m_posY;           // Posizione
    int m_width, m_height;        // Dimensioni
    int m_screenWidth, m_screenHeight;
    bool m_visible;               // Se il blocchetto è visibile

    // Per l'ottimizzazione del rendering
    int m_prevPosX, m_prevPosY;
    bool m_needsUpdate;
};

#endif