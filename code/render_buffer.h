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
    
    texture_asset Texture; 
};


struct render_entry_model
{
    v3 Position;
    quaternion Rotation;
    float Scale;
    
    model_asset *Model; 
};

enum shape_type
{
    SQUARE,
    CIRCLE,
    TRIANGLE,
    CUBE,
    SPHERE,
    CONE,
};

struct render_entry_shape
{
    v3 Position;
    quaternion Rotation;
    float Scale;
    
    shape_type Shape; 
};

struct render_entry_clear
{
    v4 Color; 
};

struct render_buffer
{
    int Count;
    memory_arena *Memory;
    buffer_startup Startup;
};

