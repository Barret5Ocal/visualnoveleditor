#include <windows.h>

static int Running = 1; 

#include "b50_timing.h"

#define GB_MATH_IMPLEMENTATION
#include "gb_math.h"
typedef gbVec2 v2;
typedef gbVec3 v3;
typedef gbVec4 v4;
typedef gbMat4 m4;
typedef gbQuat quaternion;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdint.h>
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

#define global_variable static 
#define local_persist static 
#define internal static 

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#define InvalidCodePath Assert(false)
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Kilobyte(Value) 1024 * Value
#define Megabyte(Value) 1024 * Kilobyte(Value)
#define Gigabyte(Value) 1024 * Megabyte(Value)
#define Terabyte(Value) 1024 * Gigabyte(Value)



#include "memory.cpp"
#include "assets.h"
#include "directx11.cpp"
#include "render_buffer.cpp"

LRESULT CALLBACK
MainWindowProc(HWND Window,
               UINT Message,
               WPARAM WParam,
               LPARAM LParam)
{
    LRESULT Result = {0};
    
    switch(Message)
    {
        case WM_DESTROY:
        {
            Running = 0;
        }break; 
        case WM_CLOSE:
        {
            Running = 0; 
        }break;
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        }break;
    }
    
    return Result; //DefWindowProc(Window, Message, WParam, LParam);
}

int CALLBACK 
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CmdLine,
        int ShowCode)
{
    WNDCLASS WindowClass = {0};
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowProc;
    WindowClass.hInstance = Instance; 
    WindowClass.lpszClassName = "VNEDIT";
    
    if(RegisterClass(&WindowClass))
    {
        int Width = 1280;
        int Height = 720;
        HWND Window =
            CreateWindowEx(0,
                           WindowClass.lpszClassName,
                           "Visual Novel Editor",
                           WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           Width,
                           Height,
                           0,
                           0,
                           Instance,
                           0);
        
        
        if(Window)
        {
            
            //Setup here 
            memory_arena MainMemory = {};
            InitMemoryArena(&MainMemory, Gigabyte(2));
            
            memory_arena RenderBufferMemory = {};
            PushArena(&MainMemory, Megabyte(256));
            
            render_buffer RenderBuffer = {};
            RenderBuffer.Memory = &RenderBufferMemory; 
            
            texture_asset Texture = {};
            LoadTexture(&Texture, &MainMemory, "Wood.png");
            
            render_state RenderState = {};
            InitRenderer(&RenderState, Width, Height, Window);
            
            time_info TimeInfo = {};
            while(RunLoop(&TimeInfo, Running, 60))
            {
                MSG Message; 
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                
                static float Time = 0.0f; Time += 0.05f; 
                
                quaternion Rotation = gb_quat_axis_angle({0.0f, 1.0f, 0.0f}, Time);
                
                // Render Here
                camera Camera = {{0.0f, 0.0f, -2.0f}, {0.0f, 0.0f, 1.0f}};
                PushRenderStartup(&RenderBuffer, &Camera, PERSPECTIVE);
                
                PushSprite(&RenderBuffer, {0,0,0}, Rotation, 1.0f, &Texture);
                
                RunRenderBuffer(&RenderState, &RenderBuffer, Width, Height);
            }
        }
    }
    
    return 0; 
}