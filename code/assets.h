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

enum asset_type
{
    TEXTURE,
    MODEL,
};

struct asset
{
    int Type;
    void *Memory; 
};

struct scene
{
    char *AssetNames[];
    memory_arena* RenderBuffers; 
};

void LoadTexture(texture_asset *Texture, memory_arena *Arena, char *Filename)
{
    int x,y,n;
    unsigned char *data = stbi_load(Filename, &x, &y, &n, 4);
    
    unsigned char *Data = (unsigned char *)PushArray(Arena, x*y, unsigned int);
    
    Texture->Memory = Data; 
    Texture->Width = x;
    Texture->Height = y;
    
    unsigned char *copy = data;
    for(unsigned int Y = 0;
        Y < y;
        ++Y)
    {
        for(unsigned int X = 0;
            X < x;
            ++X)
        {
            *Data++ = *copy++;
        }
    }
    
    //free(data);
    stbi_image_free(data);
}

void LoadAsset(asset_stadium *Stadium, int Type, char *FileName)
{
    switch (Type)
    {
        case TEXTURE:
        {
            asset *Asset = (asset *)PushStruct(Stadium->Asset, asset);
            Asset->Type = TEXTURE;
            
            texture_asset *Texture = (texture_asset *)PushStruct(Stadium->Texture, texture_asset);
            
            LoadTexture(Texture, Stadium->Texture, FileName);
        }break; 
        default: 
        {
            InvalidCodePath; 
        }
    }
}