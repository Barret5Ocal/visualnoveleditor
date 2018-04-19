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
    
    PushClear(RenderBuffer, {1.0f,1.0f,1.0f,1.0f});
}

