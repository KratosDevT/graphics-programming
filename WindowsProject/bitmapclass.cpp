////////////////////////////////////////////////////////////////////////////////
// Filename: bitmapclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "bitmapclass.h"

BitmapClass::BitmapClass()
{
    m_vertexBuffer = 0;
    m_indexBuffer = 0;
    m_Texture = 0;
    m_useCustomSize = false;
    m_customWidth = 0;
    m_customHeight = 0;
}


BitmapClass::BitmapClass(const BitmapClass& other)
{
}


BitmapClass::~BitmapClass()
{
}


bool BitmapClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int screenWidth, int screenHeight, char* textureFilename, int renderX, int renderY)
{
    bool result;

    // Store the screen size.
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // Store where the bitmap should be rendered to.
    m_renderX = renderX;
    m_renderY = renderY;
    // Initialize the vertex and index buffer that hold the geometry for the bitmap quad.
    result = InitializeBuffers(device);
    if (!result)
    {
        return false;
    }

    // Load the texture for this bitmap.
    result = LoadTexture(device, deviceContext, textureFilename);
    if (!result)
    {
        return false;
    }

    return true;
}

void BitmapClass::Shutdown()
{
    // Release the bitmap texture.
    ReleaseTexture();

    // Release the vertex and index buffers.
    ShutdownBuffers();

    return;
}

bool BitmapClass::Render(ID3D11DeviceContext* deviceContext)
{
    bool result;


    // Update the buffers if the position of the bitmap has changed from its original position.
    result = UpdateBuffers(deviceContext);
    if (!result)
    {
        return false;
    }

    // Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
    RenderBuffers(deviceContext);

    return true;
}

int BitmapClass::GetIndexCount()
{
    return m_indexCount;
}

ID3D11ShaderResourceView* BitmapClass::GetTexture()
{
    return m_Texture->GetTexture();
}

bool BitmapClass::InitializeBuffers(ID3D11Device* device)
{
    VertexType* vertices;
    unsigned long* indices;
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
    HRESULT result;
    int i;

    // Initialize the previous rendering position to negative one.
    m_prevPosX = -1;
    m_prevPosY = -1;

    // Set the number of vertices in the vertex array.
    m_vertexCount = 6;

    // Set the number of indices in the index array.
    m_indexCount = m_vertexCount;

    // Create the vertex array.
    vertices = new VertexType[m_vertexCount];

    // Create the index array.
    indices = new unsigned long[m_indexCount];

    // Initialize vertex array to zeros at first.
    memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));

    // Load the index array with data.
    for (i = 0; i < m_indexCount; i++)
    {
        indices[i] = i;
    }

    // Set up the description of the dynamic vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // Now finally create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // Set up the description of the index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // Create the index buffer.
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // Release the arrays now that the vertex and index buffers have been created and loaded.
    delete[] vertices;
    vertices = 0;

    delete[] indices;
    indices = 0;

    return true;
}

void BitmapClass::ShutdownBuffers()
{
    // Release the index buffer.
    if (m_indexBuffer)
    {
        m_indexBuffer->Release();
        m_indexBuffer = 0;
    }

    // Release the vertex buffer.
    if (m_vertexBuffer)
    {
        m_vertexBuffer->Release();
        m_vertexBuffer = 0;
    }

    return;
}

bool BitmapClass::UpdateBuffers(ID3D11DeviceContext* deviceContent)
{
    float left, right, top, bottom;
    VertexType* vertices;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    VertexType* dataPtr;
    HRESULT result;

    // Se la posizione e la dimensione non sono cambiate, non aggiornare
    if ((m_prevPosX == m_renderX) && (m_prevPosY == m_renderY) && !m_useCustomSize)
    {
        return true;
    }

    // Aggiorna la posizione precedente
    m_prevPosX = m_renderX;
    m_prevPosY = m_renderY;

    // Crea l'array di vertici
    vertices = new VertexType[m_vertexCount];

    // Calcola le coordinate dello schermo
    left = (float)((m_screenWidth / 2) * -1) + (float)m_renderX;

    // Usa dimensioni personalizzate se impostate, altrimenti usa quelle originali
    int currentWidth = m_useCustomSize ? m_customWidth : m_bitmapWidth;
    int currentHeight = m_useCustomSize ? m_customHeight : m_bitmapHeight;

    right = left + (float)currentWidth;
    top = (float)(m_screenHeight / 2) - (float)m_renderY;
    bottom = top - (float)currentHeight;

    // Carica l'array di vertici con i dati
    // Primo triangolo
    vertices[0].position = XMFLOAT3(left, top, 0.0f);      // Top left
    vertices[0].texture = XMFLOAT2(0.0f, 0.0f);

    vertices[1].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right
    vertices[1].texture = XMFLOAT2(1.0f, 1.0f);

    vertices[2].position = XMFLOAT3(left, bottom, 0.0f);   // Bottom left
    vertices[2].texture = XMFLOAT2(0.0f, 1.0f);

    // Secondo triangolo
    vertices[3].position = XMFLOAT3(left, top, 0.0f);      // Top left
    vertices[3].texture = XMFLOAT2(0.0f, 0.0f);

    vertices[4].position = XMFLOAT3(right, top, 0.0f);     // Top right
    vertices[4].texture = XMFLOAT2(1.0f, 0.0f);

    vertices[5].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right
    vertices[5].texture = XMFLOAT2(1.0f, 1.0f);

    // Lock del vertex buffer
    result = deviceContent->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        delete[] vertices;
        return false;
    }

    // Ottieni un puntatore ai dati nel buffer
    dataPtr = (VertexType*)mappedResource.pData;

    // Copia i dati nel vertex buffer
    memcpy(dataPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));

    // Unlock del vertex buffer
    deviceContent->Unmap(m_vertexBuffer, 0);

    // Pulisci
    dataPtr = 0;
    delete[] vertices;
    vertices = 0;

    return true;
}

void BitmapClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
    unsigned int stride;
    unsigned int offset;


    // Set vertex buffer stride and offset.
    stride = sizeof(VertexType);
    offset = 0;

    // Set the vertex buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return;
}

bool BitmapClass::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
    bool result;


    // Create and initialize the texture object.
    m_Texture = new TextureClass;

    result = m_Texture->Initialize(device, deviceContext, filename);
    if (!result)
    {
        return false;
    }

    // Store the size in pixels that this bitmap should be rendered at.
    m_bitmapWidth = m_Texture->GetWidth()/8;
    m_bitmapHeight = m_Texture->GetHeight()/16;

    return true;
}

void BitmapClass::ReleaseTexture()
{
    // Release the texture object.
    if (m_Texture)
    {
        m_Texture->Shutdown();
        delete m_Texture;
        m_Texture = 0;
    }

    return;
}

void BitmapClass::SetRenderLocation(int x, int y)
{
    if(x < 0 || x >= m_screenWidth - 100)
    {
        // Se le coordinate sono fuori dallo schermo, non aggiornare
        return;
	}
    m_renderX = x;
    m_renderY = y;
    return;
}

void BitmapClass::SetCustomSize(int width, int height)
{
    m_customWidth = width;
    m_customHeight = height;
    m_useCustomSize = true;

    // Forza l'aggiornamento del buffer alla prossima chiamata di Render
    m_prevPosX = -1;
    m_prevPosY = -1;
}

void BitmapClass::SetRenderSize(int width, int height, int x, int y)
{
    SetCustomSize(width, height);
    SetRenderLocation(x, y);
}

void BitmapClass::ResetToOriginalSize()
{
    m_useCustomSize = false;

    // Forza l'aggiornamento del buffer
    m_prevPosX = -1;
    m_prevPosY = -1;
}

int BitmapClass::GetCurrentWidth()
{
    return m_useCustomSize ? m_customWidth : m_bitmapWidth;
}

int BitmapClass::GetCurrentHeight()
{
    return m_useCustomSize ? m_customHeight : m_bitmapHeight;
}
