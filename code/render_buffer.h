struct camera 
{
    v3 Position;
    v3 Direction;
};

enum projection
{
    PERSPECTIVE,
    ORTHOGRAPHIC
};

struct buffer_startup 
{
    camera Camera;
    projection Projection; 
};

enum render_type
{
    SPRITE,
    MODEL,
    SHAPE,
    CLEAR,
};

struct render_entry_header
{
    render_type Type; 
};

struct render_entry_sprite
{
    v3 Position;
    quaternion Rotation;
    float Scale;
    
    
};

struct render_buffer
{
    void *BufferMemory;
    buffer_startup Startup;
};

