////////////////////////////////////////////////////////////////////////////////
// Filename: fontclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "fontclass.h"

FontClass::FontClass()
{
    m_vertexBuffer = 0;
    m_indexBuffer = 0;
    m_Texture = 0;
    m_fontWidth = 0;
    m_fontHeight = 0;
    m_screenWidth = 0;
    m_screenHeight = 0;
    m_renderX = 0;
    m_renderY = 0;
    m_prevPosX = -1;
    m_prevPosY = -1;
}

FontClass::FontClass(const FontClass& other)
{
}

FontClass::~FontClass()
{
}

bool FontClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext,
    int screenWidth, int screenHeight, char* textureFilename,
    int renderX, int renderY)
{
    bool result;

    // Store the screen size and render position
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    m_renderX = renderX;
    m_renderY = renderY;

    // Initialize the vertex and index buffers
    result = InitializeBuffers(device);
    if (!result)
    {
        return false;
    }

    // Load the font texture
    result = LoadTexture(device, deviceContext, textureFilename);
    if (!result)
    {
        return false;
    }

    return true;
}

void FontClass::Shutdown()
{
    // Release the font texture
    ReleaseTexture();

    // Release the vertex and index buffers
    ShutdownBuffers();

    return;
}

bool FontClass::Render(ID3D11DeviceContext* deviceContext)
{
    bool result;

    // Update the buffers if the position has changed
    result = UpdateBuffers(deviceContext);
    if (!result)
    {
        return false;
    }

    // Put the vertex and index buffers on the graphics pipeline
    RenderBuffers(deviceContext);

    return true;
}

int FontClass::GetIndexCount()
{
    return m_indexCount;
}

ID3D11ShaderResourceView* FontClass::GetTexture()
{
    return m_Texture->GetTexture();
}

void FontClass::SetRenderLocation(int x, int y)
{
    m_renderX = x;
    m_renderY = y;
    return;
}

bool FontClass::InitializeBuffers(ID3D11Device* device)
{
    VertexType* vertices;
    unsigned long* indices;
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
    HRESULT result;
    int i;

    // Initialize the previous rendering position to negative one
    m_prevPosX = -1;
    m_prevPosY = -1;

    // Set the number of vertices in the vertex array
    m_vertexCount = 6;

    // Set the number of indices in the index array
    m_indexCount = m_vertexCount;

    // Create the vertex array
    vertices = new VertexType[m_vertexCount];

    // Create the index array
    indices = new unsigned long[m_indexCount];

    // Initialize vertex array to zeros at first
    memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));

    // Load the index array with data
    for (i = 0; i < m_indexCount; i++)
    {
        indices[i] = i;
    }

    // Set up the description of the dynamic vertex buffer
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the vertex data
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // Create the vertex buffer
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // Set up the description of the index buffer
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the index data
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // Create the index buffer
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    // Release the arrays now that the buffers have been created
    delete[] vertices;
    vertices = 0;

    delete[] indices;
    indices = 0;

    return true;
}

void FontClass::ShutdownBuffers()
{
    // Release the index buffer
    if (m_indexBuffer)
    {
        m_indexBuffer->Release();
        m_indexBuffer = 0;
    }

    // Release the vertex buffer
    if (m_vertexBuffer)
    {
        m_vertexBuffer->Release();
        m_vertexBuffer = 0;
    }

    return;
}

bool FontClass::UpdateBuffers(ID3D11DeviceContext* deviceContext)
{
    float left, right, top, bottom;
    VertexType* vertices;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    VertexType* dataPtr;
    HRESULT result;

    // If the position hasn't changed then don't update the vertex buffer
    if ((m_prevPosX == m_renderX) && (m_prevPosY == m_renderY))
    {
        return true;
    }

    // Update the previous position
    m_prevPosX = m_renderX;
    m_prevPosY = m_renderY;

    // Create the vertex array
    vertices = new VertexType[m_vertexCount];

    // Calculate the screen coordinates of the font
    left = (float)((m_screenWidth / 2) * -1) + (float)m_renderX;
    right = left + (float)m_fontWidth;
    top = (float)(m_screenHeight / 2) - (float)m_renderY;
    bottom = top - (float)m_fontHeight;

    // Load the vertex array with data for the two triangles of the quad
    // First triangle
    vertices[0].position = XMFLOAT3(left, top, 0.0f);      // Top left
    vertices[0].texture = XMFLOAT2(0.0f, 0.0f);

    vertices[1].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right
    vertices[1].texture = XMFLOAT2(1.0f, 1.0f);

    vertices[2].position = XMFLOAT3(left, bottom, 0.0f);   // Bottom left
    vertices[2].texture = XMFLOAT2(0.0f, 1.0f);

    // Second triangle
    vertices[3].position = XMFLOAT3(left, top, 0.0f);      // Top left
    vertices[3].texture = XMFLOAT2(0.0f, 0.0f);

    vertices[4].position = XMFLOAT3(right, top, 0.0f);     // Top right
    vertices[4].texture = XMFLOAT2(1.0f, 0.0f);

    vertices[5].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right
    vertices[5].texture = XMFLOAT2(1.0f, 1.0f);

    // Lock the vertex buffer
    result = deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        delete[] vertices;
        return false;
    }

    // Get a pointer to the data in the vertex buffer
    dataPtr = (VertexType*)mappedResource.pData;

    // Copy the data into the vertex buffer
    memcpy(dataPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));

    // Unlock the vertex buffer
    deviceContext->Unmap(m_vertexBuffer, 0);

    // Release the vertex array
    delete[] vertices;
    vertices = 0;

    return true;
}

void FontClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
    unsigned int stride;
    unsigned int offset;

    // Set vertex buffer stride and offset
    stride = sizeof(VertexType);
    offset = 0;

    // Set the vertex buffer to active in the input assembler
    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return;
}

bool FontClass::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
    bool result;

    // Create and initialize the texture object
    m_Texture = new TextureClass;

    result = m_Texture->Initialize(device, deviceContext, filename);
    if (!result)
    {
        return false;
    }

    // Store the size in pixels that this font should be rendered at
    m_fontWidth = m_Texture->GetWidth();
    m_fontHeight = m_Texture->GetHeight();

    return true;
}

void FontClass::ReleaseTexture()
{
    // Release the texture object
    if (m_Texture)
    {
        m_Texture->Shutdown();
        delete m_Texture;
        m_Texture = 0;
    }

    return;
}