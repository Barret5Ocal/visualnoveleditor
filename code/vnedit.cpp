#include "vnedit.h"

void ChangeScene()
{
    
}

void RunEngine(engine_state *State, render_buffer *RenderBuffer)
{
    if(!State->Initialized)
    {
        
        
        State->Initialized = 1; 
    }
    
    PushClear(RenderBuffer, {0.0f,0.0f,0.0f,1.0f});
    
    directx_buffer *Buffers = (directx_buffer *)State->CurrentScene.Assets.RenderBuffers.Memory;
    PushBackground(RenderBuffer, Buffers);
    
    directx_buffer *CBuffer = (directx_buffer *)(State->CurrentScene.Assets.RenderBuffers.Memory + sizeof(directx_buffer));
    PushSprite(RenderBuffer, CBuffer, {0.0f, 0.0f}, 3.0f);
}

