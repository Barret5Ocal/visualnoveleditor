enum render_type
{
    CLEAR, 
    BACKGROUND,
    SPRITE,
    TEXTBOX,
    TEXT,
};

struct render_header
{
    int Type;
};

struct render_buffer 
{
    int Count; 
    memory_arena Memory;
};

struct render_clear
{
    v4 Color; 
};

void PushClear(render_buffer *Buffer, v4 Color)
{
    render_header *Header = (render_header *) PushStruct(&Buffer->Memory, render_header);
    Header->Type = CLEAR; 
    
    Buffer->Count++;
    render_clear *Clear = (render_clear *)PushStruct(&Buffer->Memory, render_clear);
    Clear->Color = Color; 
}

struct render_background 
{
    directx_buffer *Buffers;
};

void PushBackground(render_buffer *Buffer, directx_buffer *Buffers)
{
    render_header *Header = (render_header *) PushStruct(&Buffer->Memory, render_header);
    Header->Type = BACKGROUND; 
    
    Buffer->Count++;
    render_background *Background = (render_background *)PushStruct(&Buffer->Memory, render_background);
    
    Background->Buffers = Buffers; 
}

void InitializeRenderBuffer(render_buffer *Buffer, memory_arena *Main)
{
    Buffer->Memory = PushArena(Main, Megabyte(32));
    Buffer->Count = 0;
}

void ResetRenderBuffer(render_buffer *Buffer)
{
    ResetArena(&Buffer->Memory);
    Buffer->Count = 0;
}

void RunRenderBuffer(render_buffer *Buffer)
{
    uint8 *Memory = (uint8 *)Buffer->Memory.Memory; 
    for(int Index = 0;
        Index < Buffer->Count;
        Index++)
    {
        render_header *Header = (render_header *)Memory; 
        switch(Header->Type)
        {
            case CLEAR:
            {
                Memory+= sizeof(render_header);
                render_clear *Clear = (render_clear *)Memory;
                ClearScreen(Clear->Color); 
                
                Memory += sizeof(render_clear);
            }break;
            case BACKGROUND:
            {
                Memory+= sizeof(render_header);
                render_background *Background = (render_background *)Memory;
                
                
                DrawBackGround(Background->Buffers);
                
                Memory += sizeof(render_background);
            }break;
        }
    }
    
    ResetRenderBuffer(Buffer);
}

