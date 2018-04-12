#include "include\compare-string.c"
#include "include\hash-string.c"
#include "include\hash-table.c"

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
    int NameCount;
    char *AssetNames;
    memory_arena* RenderBuffers; 
};


struct asset_stadium
{
    HashTable *Table; 
    int Count; 
    memory_arena Assets;
    memory_arena Textures;
    memory_arena Models; 
    memory_arena AssetStrings; 
};

void LoadTexture(texture_asset *Texture, memory_arena *Arena, char *Filename)
{
    int x,y,n;
    unsigned char *data = stbi_load(Filename, &x, &y, &n, 4);
    
    unsigned char *Data = (unsigned char *)PushArray(Arena, x*y * 4, unsigned int);
    
    Texture->Memory = Data; 
    Texture->Width = x;
    Texture->Height = y;
    
    unsigned int *destination = (unsigned int *)Data;
    unsigned int *source = (unsigned int *)data;
    for(unsigned int Y = 0;
        Y < y;
        ++Y)
    {
        for(unsigned int X = 0;
            X < x;
            ++X)
        {
            *destination++ = *source++;
        }
    }
    
    //free(data);
    stbi_image_free(data);
}

int B50StringLen(char *String)
{
    int Result = 0;
    while(*String++)
    {
        Result++; 
    }
    
    return Result; 
}

int32 B50strcmp(uint8 *S1, uint8 *S2)
{
    while(*S1 == *S2)
    {
        if(!*S1)
            return 0; 
        ++S1;
        ++S2;
    }
    
    return ((*S1 < *S2) ? -1 : 1);
}

int32 B50atoi(uint8 *Str)
{
    int32 Sign = 1;
    int32 Value = 0;
    uint8 C;
    
    if(*Str == '-')
    {
        Sign = -1;
        ++Str;
    }
    
    if(Str[0] == '0' && (Str[1] == 'x' || Str[1] == 'X'))
    {
        Str += 2; 
        while(1)
        {
            C = *Str;
            ++Str;
            if(C >= '0' && C <= '9')
                Value = Value * 16 + C - '0';
            else if (C >= 'a' && C <= 'f')
                Value = Value * 16 + C - 'a' + 10;
            else if (C >= 'A' && C <= 'F')
                Value = Value * 16 + C - 'A' + 10;
            else
                return Value * Sign;
        }
    }
    
    while(1)
    {
        C = *Str;
        ++Str;
        if((C < '0') || (C > '9'))
        
            return Sign * Value;
        
        Value = Value * 10 + C - '0';
    }
}

asset *LoadAsset(asset_stadium *Stadium, int Type, char *FileName)
{
    asset *Asset;
    switch (Type)
    {
        case TEXTURE:
        {
            Asset = (asset *)PushStruct(&Stadium->Assets, asset);
            Asset->Type = TEXTURE;
            
            texture_asset *Texture = (texture_asset *)PushStruct(&Stadium->Textures, texture_asset);
            Asset->Memory = Texture;
            
            int NameLen = B50StringLen(FileName);
            
            char *Name = (char *)PushArray(&Stadium->AssetStrings, NameLen + 1, char);
            
            B50memcpy(Name, FileName, NameLen);
            
            char buffer[10];
            stbsp_sprintf(buffer, "%i", Stadium->Count++);
            hash_table_insert(Stadium->Table, Name, buffer);
            
            LoadTexture(Texture, &Stadium->Textures, FileName);
        }break; 
        default: 
        {
            InvalidCodePath; 
        }
    }
    
    return Asset;
}

texture_asset *GetTexture(asset_stadium *Stadium, char *Name)
{
    char *IndexC = (char *)hash_table_lookup(Stadium->Table, Name);
    int Index = B50atoi((uint8 *)IndexC);
    
    asset *Asset = ((asset *)Stadium->Assets.Memory) + Index;
    return (texture_asset *)Asset->Memory;
}