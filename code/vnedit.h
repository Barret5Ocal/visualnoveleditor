struct location 
{
    directx_buffer Buffers;
};

struct sequence
{
    location Location; 
    
};

struct scene
{
    sequence Sequence; 
    scene_assets Assets; 
};

struct engine_state
{
    scene CurrentScene; 
    int Initialized;
};

