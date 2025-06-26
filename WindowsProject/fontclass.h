#pragma once

#ifndef _FONTCLASS_H_
#define _FONTCLASS_H_

#include <d3d11.h>
#include <directxmath.h>
using namespace DirectX;

#include "textureclass.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: FontClass
////////////////////////////////////////////////////////////////////////////////
class FontClass
{
private:
    struct VertexType
    {
        XMFLOAT3 position;
        XMFLOAT2 texture;
    };

public:
    FontClass();
    FontClass(const FontClass&);
    ~FontClass();

    bool Initialize(ID3D11Device*, ID3D11DeviceContext*, int, int, char*, int, int);
    void Shutdown();
    bool Render(ID3D11DeviceContext*);

    int GetIndexCount();
    ID3D11ShaderResourceView* GetTexture();

    // Getters per posizione e dimensioni
    int GetRenderX() { return m_renderX; }
    int GetRenderY() { return m_renderY; }
    int GetWidth() { return m_fontWidth; }
    int GetHeight() { return m_fontHeight; }

    // Setters per posizione
    void SetRenderLocation(int x, int y);

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
    int m_screenWidth, m_screenHeight;
    int m_fontWidth, m_fontHeight;
    int m_renderX, m_renderY;
    int m_prevPosX, m_prevPosY;
    TextureClass* m_Texture;
};

#endif