#include "render_buffer.h"

void ResetRenderBuffer(render_buffer *Buffer)
{
    Buffer->Count = 0;
    ResetArena(Buffer->Memory);
    Buffer->Startup = {};
}

void PushRenderStartup(render_buffer *Buffer, camera *Camera, projection Proj)
{
    Buffer->Startup = {*Camera, Proj};
}

void PushClear(render_buffer *Buffer, v4 Color)
{
    Buffer->Count++;
    render_entry_header *Header = (render_entry_header *)PushStruct(Buffer->Memory, render_entry_header);
    
    Header->Type = CLEAR; 
    
    
    render_entry_clear *Clear = (render_entry_clear *)PushStruct(Buffer->Memory, render_entry_clear);
    
    Clear->Color = Color; 
}


void PushSprite(render_buffer *Buffer, v3 Position, quaternion Rotation, float Scale, texture_asset *Texture)
{
    Buffer->Count++;
    render_entry_header *Header = (render_entry_header *)PushStruct(Buffer->Memory, render_entry_header);
    
    Header->Type = SPRITE; 
    
    render_entry_sprite *Sprite = (render_entry_sprite *)PushStruct(Buffer->Memory, render_entry_sprite);
    
    Sprite->Position = Position; 
    Sprite->Rotation = Rotation; 
    Sprite->Scale = Scale; 
    Sprite->Texture = Texture; 
}

struct render_state
{
#if defined(_WIN32)
    directx11_state D11State;
    
#elif defined(__APPLE__)
    InvalidCodePath; 
#else
    InvalidCodePath; 
#endif
};

void InitRenderer(render_state *State, int Width, int Height, HWND Window)
{
#if defined(_WIN32)
    InitializeD3D(&State->D11State.Swapchain, &State->D11State.Dev, &State->D11State.Devcon, &State->D11State.Backbuffer, &State->D11State.ZBuffer, Width, Height, Window);
    
    InitPipeline(State->D11State.Dev, State->D11State.Devcon, &State->D11State.VS, &State->D11State.PS, &State->D11State.Layout, &State->D11State.CBuffer);
    
#elif defined(__APPLE__)
    InvalidCodePath; 
#else
    InvalidCodePath; 
#endif
}

void LoadVertices(render_state *State, vertex *Vertices, int Count)
{
#if defined(_WIN32)
    D11LoadVertices(&State->D11State, Vertices, Count);
#elif 
    InvalidCodePath;
#endif
}


void LoadIndices(render_state *State, DWORD *Indices, int Count)
{
#if defined(_WIN32)
    D11LoadIndices(&State->D11State, Indices, Count);
#elif 
    InvalidCodePath;
#endif
}


void LoadTexture(render_state *State, texture_asset *Texture)
{
#if defined(_WIN32)
    D11LoadTexture(&State->D11State, Texture->Memory, Texture->Width, Texture->Height);
#elif 
    InvalidCodePath;
#endif
}

void PresentFrame(render_state *State)
{
#if defined(_WIN32)
    D11PresentFrame(&State->D11State);
#elif 
    InvalidCodePath;
#endif
}

void DrawIndexed(render_state *State, int Count, m4 *Final)
{
#if defined(_WIN32)
    D11DrawIndexed(&State->D11State, Count, Final);
#elif 
    InvalidCodePath;
#endif
}

void ClearColor(render_state *State, v4 Color)
{
#if defined(_WIN32)
    D11Clear(&State->D11State, Color.e);
#elif 
    InvalidCodePath;
#endif
}

void RunRenderBuffer(render_state *State, render_buffer *Buffer, int Width, int Height)
{
    unsigned char *BufferMemory = Buffer->Memory->Memory; 
    for(int Index = 0;
        Index < Buffer->Count;
        ++Index)
    {
        render_entry_header *Header = (render_entry_header *)BufferMemory;
        BufferMemory += sizeof(render_entry_header);
        switch(Header->Type)
        {
            case CLEAR:
            {
                render_entry_clear *Clear = (render_entry_clear *)BufferMemory;
                BufferMemory += sizeof(render_entry_clear);
                
                ClearColor(State, Clear->Color);
                
            }break;
            case SPRITE: 
            {
                render_entry_sprite *Sprite = (render_entry_sprite *)BufferMemory;
                BufferMemory += sizeof(render_entry_sprite);
                
                vertex Vertices[]
                {
                    {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // Top Left
                    {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // Bottom Right 
                    {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // Bottom Left 
                    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // Top Right 
                };
                
                LoadVertices(State, &Vertices[0], 4);
                DWORD Indices[] = 
                {
                    0, 1, 2, 
                    0, 3, 1,
                };
                LoadIndices(State, &Indices[0], 6);
                
                LoadTexture(State, Sprite->Texture);
                
                m4 MatModel, MatTrans, MatRotate, MatScale, MatView, MatProjection, MatFinal;
                
                gb_mat4_translate(&MatTrans, Sprite->Position);
                gb_mat4_from_quat(&MatRotate, Sprite->Rotation);
                gb_mat4_scale(&MatScale, {Sprite->Scale, Sprite->Scale, Sprite->Scale});
                
                MatModel = MatScale * MatRotate * MatTrans;
                
                gb_mat4_perspective(&MatProjection, gb_to_radians(90.0f), (float)Width/(float)Height, 0.1f, 100.0f);
                
                
                gb_mat4_look_at(&MatView, Buffer->Startup.Camera.Position,
                                //Buffer->Startup.Camera.Position + Buffer->Startup.Camera.Direction
                                {0.0f, 0.0f, 0.0f}
                                , {0.0f, 1.0f, 0.0f});
                
                MatFinal = MatProjection * MatView * MatModel;
                
                DrawIndexed(State, 6, &MatFinal);
            }break; 
            default:
            {
                InvalidCodePath;
            }break ;
        }
    }
    
    PresentFrame(State);
    
    // NOTE(Barret5Ocal): Make sure this is the last thing
    ResetRenderBuffer(Buffer);
}