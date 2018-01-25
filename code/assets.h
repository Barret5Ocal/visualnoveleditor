struct texture_asset
{
    unsigned char *Memory;
    int Width;
    int Height;
};

struct vertex
{
    v3 Position;
    v3 Normal; 
    v2 UV; 
};

struct model_asset
{
    int VertexCount;
    vertex *Vertices;
    
    int IndexCount;
    DWORD *Indices;
    
    texture_asset *Texture; 
};

void LoadTexture(texture_asset *Texture, memory_arena *Arena, char *Filename)
{
    int x,y,n;
    unsigned char *data = stbi_load(Filename, &x, &y, &n, 4);
    
    unsigned char *Data = (unsigned char *)PushArray(Arena, x*y, unsigned int);
    
    Texture->Memory = Data; 
    Texture->Width = x;
    Texture->Height = y;
    
    for(unsigned int Y = 0;
        Y < y;
        ++Y)
    {
        for(unsigned int X = 0;
            X < x;
            ++X)
        {
            *Data++ = *data++;
        }
    }
    
    stbi_image_free(data);
}

void LoadCardModel(model_asset *Model, memory_arena *Arena)
{
    vertex Vertices[]
    {
        {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // Top Left
        {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // Bottom Right 
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // Bottom Left 
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // Top Right 
        
    };
    
    vertex *VertexBuffer = (vertex *)PushArray(Arena, 4, vertex);
    B50memcpy(VertexBuffer, Vertices, sizeof(Vertices));
    
    Model->Vertices = VertexBuffer;
    Model->VertexCount = 4; 
    
    DWORD Indices[] = 
    {
        0, 1, 2, 
        0, 3, 1,
    };
    
    DWORD *IndexBuffer = (DWORD *)PushArray(Arena, 6, DWORD);
    B50memcpy(IndexBuffer, Indices, sizeof(Indices));
    
    Model->Indices = IndexBuffer; 
    Model->IndexCount = 6; 
    
}