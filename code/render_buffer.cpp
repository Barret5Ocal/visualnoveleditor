#include "render_buffer.h"

void PushRenderStartup(render_buffer *Buffer, camera *Camera, projection Proj)
{
    Buffer->Startup = {*Camera, Proj};
}

void PushSprite(render_buffer *Buffer, v3 Position, quaternion Rotation, float Scale, texture_asset *Texture)
{
    
}