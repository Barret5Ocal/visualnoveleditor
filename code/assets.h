struct texture_asset
{
    void *Memory;
    int Width;
    int Height;
};

struct vertex
{
    v3 Position;
    v4 Color; 
    v2 UV; 
};

struct model_asset
{
    int Count;
    vertex *Vertices;
    
    texture_asset *Texture; 
};