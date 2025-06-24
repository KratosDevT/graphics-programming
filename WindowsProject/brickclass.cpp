////////////////////////////////////////////////////////////////////////////////
// Filename: brickclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "brickclass.h"

BrickClass::BrickClass()
{
    m_vertexBuffer = 0;
    m_indexBuffer = 0;
    m_Texture = 0;
    m_posX = 0;
    m_posY = 0;
    m_width = 0;
    m_height = 0;
    m_screenWidth = 0;
    m_screenHeight = 0;
    m_visible = true;
    m_prevPosX = -1;
    m_prevPosY = -1;
    m_needsUpdate = true;
}

BrickClass::BrickClass(const BrickClass& other)
{
}

BrickClass::~BrickClass()
{
}

bool BrickClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext,
    int screenWidth, int screenHeight, char* textureFilename,
    int posX, int posY, int width, int height)
{
    bool result;

    // Store screen dimensions and brick properties
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    m_posX = posX;
    m_posY = posY;
    m_width = width;
    m_height = height;
    m_visible = true;

    // Initialize the vertex and index buffers
    result = InitializeBuffers(device);
    if (!result)
    {
        return false;
    }

    // Load the texture for this brick
    result = LoadTexture(device, deviceContext, textureFilename);
    if (!result)
    {
        return false;
    }

    return true;
}

void BrickClass::Shutdown()
{
    // Release the brick texture
    ReleaseTexture();

    // Release the vertex and index buffers
    ShutdownBuffers();

    return;
}

bool BrickClass::Render(ID3D11DeviceContext* deviceContext)
{
    // Don't render if not visible
    if (!m_visible)
    {
        return true;
    }

    bool result;

    // Update the buffers if needed
    result = UpdateBuffers(deviceContext);
    if (!result)
    {
        return false;
    }

    // Put the vertex and index buffers on the graphics pipeline
    RenderBuffers(deviceContext);

    return true;
}

int BrickClass::GetIndexCount()
{
    return m_indexCount;
}

ID3D11ShaderResourceView* BrickClass::GetTexture()
{
    return m_Texture->GetTexture();
}

bool BrickClass::InitializeBuffers(ID3D11Device* device)
{
    VertexType* vertices;
    unsigned long* indices;
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
    HRESULT result;
    int i;

    // Set the number of vertices in the vertex array
    m_vertexCount = 6;  // Two triangles for a quad

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

void BrickClass::ShutdownBuffers()
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

bool BrickClass::UpdateBuffers(ID3D11DeviceContext* deviceContext)
{
    // Only update if we need to (first time or position changed)
    if (!m_needsUpdate)
    {
        return true;
    }

    float left, right, top, bottom;
    VertexType* vertices;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    VertexType* dataPtr;
    HRESULT result;

    // Create the vertex array
    vertices = new VertexType[m_vertexCount];

    // Calculate the screen coordinates of the brick
    // Convert from screen coordinates to DirectX coordinates
    left = (float)((m_screenWidth / 2) * -1) + (float)m_posX;
    right = left + (float)m_width;
    top = (float)(m_screenHeight / 2) - (float)m_posY;
    bottom = top - (float)m_height;

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

    // Mark as updated
    m_needsUpdate = false;

    return true;
}

void BrickClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
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

bool BrickClass::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
    bool result;

    // Create and initialize the texture object
    m_Texture = new TextureClass;

    result = m_Texture->Initialize(device, deviceContext, filename);
    if (!result)
    {
        return false;
    }

    return true;
}

void BrickClass::ReleaseTexture()
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