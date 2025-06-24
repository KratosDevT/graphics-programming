////////////////////////////////////////////////////////////////////////////////
// Filename: circleclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "circleclass.h"

#include <cmath>        // AGGIUNGI QUESTO per fabs()
#include <algorithm>    // AGGIUNGI QUESTO per min/max

CircleClass::CircleClass()
{
    m_vertexBuffer = 0;
    m_indexBuffer = 0;
    m_Texture = 0;
    m_positionX = 0.0f;
    m_positionY = 0.0f;
    m_velocityX = 0.0f;
    m_velocityY = 0.0f;
    m_radius = 0.0f;
    m_screenWidth = 0;
    m_screenHeight = 0;
    m_prevPosX = -1;
    m_prevPosY = -1;
}

CircleClass::CircleClass(const CircleClass& other)
{
}

CircleClass::~CircleClass()
{
}

bool CircleClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext,
    int screenWidth, int screenHeight, char* textureFilename,
    float startX, float startY, float radius)
{
    bool result;

    // Store screen dimensions
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // Set initial position and properties
    m_positionX = startX;
    m_positionY = startY;
    m_radius = radius;

    // Set initial velocity (puoi modificare questi valori)
    m_velocityX = 150.0f;  // pixel al secondo
    m_velocityY = 100.0f;  // pixel al secondo

    // Initialize the vertex and index buffers
    result = InitializeBuffers(device);
    if (!result)
    {
        return false;
    }

    // Load the texture for this circle
    result = LoadTexture(device, deviceContext, textureFilename);
    if (!result)
    {
        return false;
    }

    return true;
}

void CircleClass::Shutdown()
{
    // Release the circle texture
    ReleaseTexture();

    // Release the vertex and index buffers
    ShutdownBuffers();

    return;
}

bool CircleClass::Render(ID3D11DeviceContext* deviceContext)
{
    bool result;

    // Update the buffers with the current position
    result = UpdateBuffers(deviceContext);
    if (!result)
    {
        return false;
    }

    // Put the vertex and index buffers on the graphics pipeline
    RenderBuffers(deviceContext);

    return true;
}

void CircleClass::Update(float deltaTime, int screenWidth, int screenHeight)
{
    // Update screen dimensions
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // Update position
    m_positionX += m_velocityX * deltaTime;
    m_positionY += m_velocityY * deltaTime;

    // Check for collision with screen boundaries and bounce
    // Left wall
    if (m_positionX - m_radius <= 0)
    {
        m_positionX = m_radius;
        m_velocityX = -m_velocityX;  // Reverse X velocity
    }
    // Right wall
    else if (m_positionX + m_radius >= m_screenWidth)
    {
        m_positionX = m_screenWidth - m_radius;
        m_velocityX = -m_velocityX;  // Reverse X velocity
    }

    // Top wall
    if (m_positionY - m_radius <= 0)
    {
        m_positionY = m_radius;
        m_velocityY = -m_velocityY;  // Reverse Y velocity
    }
    // Bottom wall
    else if (m_positionY + m_radius >= m_screenHeight)
    {
        m_positionY = m_screenHeight - m_radius;
        m_velocityY = -m_velocityY;  // Reverse Y velocity
    }
}

int CircleClass::GetIndexCount()
{
    return m_indexCount;
}

ID3D11ShaderResourceView* CircleClass::GetTexture()
{
    return m_Texture->GetTexture();
}

// Helper function for circle-rectangle collision detection
bool CircleClass::CheckCircleRectCollision(float circleX, float circleY, float radius,
    float rectLeft, float rectTop, float rectRight, float rectBottom)
{
    // Find the closest point on the rectangle to the circle center
    float closestX = max(rectLeft, min(circleX, rectRight));
    float closestY = max(rectTop, min(circleY, rectBottom));

    // Calculate distance between circle center and closest point
    float distanceX = circleX - closestX;
    float distanceY = circleY - closestY;
    float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);

    // Check if distance is less than radius (collision)
    return distanceSquared < (radius * radius);
}

void CircleClass::UpdateWithCollision(float deltaTime, int screenWidth, int screenHeight,
    int rectX, int rectY, int rectWidth, int rectHeight)
{
    // Update screen dimensions
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // Calculate next position
    float nextX = m_positionX + m_velocityX * deltaTime;
    float nextY = m_positionY + m_velocityY * deltaTime;

    // Check collision with screen boundaries first
    bool hitScreenBoundary = false;

    // Left/Right screen boundaries
    if (nextX - m_radius <= 0)
    {
        nextX = m_radius;
        m_velocityX = -m_velocityX;
        hitScreenBoundary = true;
    }
    else if (nextX + m_radius >= screenWidth)
    {
        nextX = screenWidth - m_radius;
        m_velocityX = -m_velocityX;
        hitScreenBoundary = true;
    }

    // Top/Bottom screen boundaries
    if (nextY - m_radius <= 0)
    {
        nextY = m_radius;
        m_velocityY = -m_velocityY;
        hitScreenBoundary = true;
    }
    else if (nextY + m_radius >= screenHeight)
    {
        nextY = screenHeight - m_radius;
        m_velocityY = -m_velocityY;
        hitScreenBoundary = true;
    }

    // Check collision with rectangle (bitmap) only if not hitting screen boundary
    if (!hitScreenBoundary)
    {
        // Rectangle boundaries
        float rectLeft = (float)rectX;
        float rectRight = (float)(rectX + rectWidth);
        float rectTop = (float)rectY;
        float rectBottom = (float)(rectY + rectHeight);

        // Check if circle will intersect with rectangle
        bool willCollide = CheckCircleRectCollision(nextX, nextY, m_radius,
            rectLeft, rectTop, rectRight, rectBottom);

        if (willCollide)
        {
            // Determine which side of the rectangle was hit
            float currentCenterX = m_positionX;
            float currentCenterY = m_positionY;

            // Calculate distances to each edge
            float distToLeft = abs(currentCenterX - rectLeft);
            float distToRight = abs(currentCenterX - rectRight);
            float distToTop = abs(currentCenterY - rectTop);
            float distToBottom = abs(currentCenterY - rectBottom);

            // Find the minimum distance to determine collision side
            float minDist = min(min(distToLeft, distToRight), min(distToTop, distToBottom));

            if (minDist == distToLeft || minDist == distToRight)
            {
                // Hit left or right side - reverse X velocity
                m_velocityX = -m_velocityX;

                // Position ball outside the rectangle
                if (minDist == distToLeft)
                    nextX = rectLeft - m_radius - 1;
                else
                    nextX = rectRight + m_radius + 1;
            }
            else
            {
                // Hit top or bottom side - reverse Y velocity
                m_velocityY = -m_velocityY;

                // Position ball outside the rectangle
                if (minDist == distToTop)
                    nextY = rectTop - m_radius - 1;
                else
                    nextY = rectBottom + m_radius + 1;
            }
        }
    }

    // Update position
    m_positionX = nextX;
    m_positionY = nextY;
}

void CircleClass::Reset(float x, float y, float velX, float velY)
{
    m_positionX = x;
    m_positionY = y;
    m_velocityX = velX;
    m_velocityY = velY;

    // Force buffer update
    m_prevPosX = -1;
    m_prevPosY = -1;
}

bool CircleClass::InitializeBuffers(ID3D11Device* device)
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

void CircleClass::ShutdownBuffers()
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

bool CircleClass::UpdateBuffers(ID3D11DeviceContext* deviceContext)
{
    float left, right, top, bottom;
    VertexType* vertices;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    VertexType* dataPtr;
    HRESULT result;

    // Check if position has changed
    int currentPosX = (int)m_positionX;
    int currentPosY = (int)m_positionY;

    if ((m_prevPosX == currentPosX) && (m_prevPosY == currentPosY))
    {
        return true;
    }

    // Update previous position
    m_prevPosX = currentPosX;
    m_prevPosY = currentPosY;

    // Create the vertex array
    vertices = new VertexType[m_vertexCount];

    // Calculate the screen coordinates of the circle
    // Convert from screen coordinates to DirectX coordinates
    left = (float)((m_screenWidth / 2) * -1) + (m_positionX - m_radius);
    right = (float)((m_screenWidth / 2) * -1) + (m_positionX + m_radius);
    top = (float)(m_screenHeight / 2) - (m_positionY - m_radius);
    bottom = (float)(m_screenHeight / 2) - (m_positionY + m_radius);

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

void CircleClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
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

bool CircleClass::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
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

void CircleClass::ReleaseTexture()
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