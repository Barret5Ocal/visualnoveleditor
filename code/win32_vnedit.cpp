#include <windows.h>

static int Running = 1; 

#include "b50_timing.h"

#include <float.h>

#define GB_MATH_IMPLEMENTATION
#include "include\gb_math.h"
typedef gbVec2 v2;
typedef gbVec3 v3;
typedef gbVec4 v4;
typedef gbMat4 m4;
typedef gbQuat quaternion;

#define STB_IMAGE_IMPLEMENTATION
#include "include\stb_image.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "include\stb_sprintf.h"
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"

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

#define ScreenWidth 1280
#define ScreenHeight 720

#include "memory.cpp"
#include "assets.h"

#include "directx11.cpp"

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
        HWND Window =
            CreateWindowEx(0,
                           WindowClass.lpszClassName,
                           "Visual Novel Editor",
                           WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           ScreenWidth,
                           ScreenHeight,
                           0,
                           0,
                           Instance,
                           0);
        
        
        if(Window)
        {
            FullSetup(Window);
            
            memory_arena MainArena = {};
            InitMemoryArena(&MainArena, Gigabyte(1));
            
            asset_stadium Stadium = {};
            Stadium.Assets = PushArena(&MainArena, Megabyte(265));
            Stadium.Textures = PushArena(&MainArena, Megabyte(265));
            Stadium.Models = PushArena(&MainArena, Megabyte(265));
            Stadium.AssetStrings = PushArena(&MainArena, Megabyte(64));
            
            Stadium.Table = hash_table_new(string_hash, string_equal);
            hash_table_register_free_functions(Stadium.Table, NULL, free);
            
            LoadAsset(&Stadium, TEXTURE, "bedroom.png");
            
            texture_asset *Texture = GetTexture(&Stadium, "bedroom.png");
            
            //texture_asset *Texture = (texture_asset *)Asset->Memory;
            directx_texture_asset Background = {Texture->Memory, Texture->Width, Texture->Height, 4};
            
            //int x,y,n;
            //unsigned char *data = stbi_load("bedroom.png", &x, &y, &n, 4);
            //directx_texture_asset Background = {data, x, y, n};
            
            
            LoadBuffers(&Background.Buffers, &Background);
            
            time_info TimeInfo = {};
            while(RunLoop(&TimeInfo, Running, 60))
            {
                MSG Message; 
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                
                Devcon->RSSetState(RSDefault);
                Devcon->OMSetBlendState(BS, 0, 0xffffffff);
                Devcon->PSSetSamplers(0, 1, &SamplerState);
                
                ClearScreen(Devcon, Backbuffer, ZBuffer); 
                
                DrawBackGround(&Background);
                
                RenderToScreen();
            }
            
            Swapchain->SetFullscreenState(FALSE, NULL);
        }
        
    }
    
    return 0; 
}