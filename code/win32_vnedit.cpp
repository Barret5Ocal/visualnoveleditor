#include <windows.h>

static int Running = 1; 

#include "b50_timing.h"

#include <float.h>

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

void ClearScreen(ID3D11DeviceContext *Devcon, ID3D11RenderTargetView *Backbuffer,    // the pointer to our back buffer
                 ID3D11DepthStencilView *ZBuffer)
{
    float Color[] = {0.0f, 0.2f, 0.4f, 1.0f};
    Devcon->ClearRenderTargetView(Backbuffer, Color);
    
    // clear the depth buffer
    Devcon->ClearDepthStencilView(ZBuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);
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
            
            IDXGISwapChain *Swapchain;             // the pointer to the swap chain interface
            ID3D11Device *Dev;                     // the pointer to our Direct3D device interface
            ID3D11DeviceContext *Devcon;           // the pointer to our Direct3D device context
            ID3D11RenderTargetView *Backbuffer;    // the pointer to our back buffer
            
            ID3D11DepthStencilView *ZBuffer;       // the pointer to our depth buffer
            
            InitializeD3D(&Swapchain, &Dev, &Devcon, &Backbuffer, &ZBuffer, 
                          Width, Height, Window);
            
            ID3D11VertexShader *VS;
            ID3D11PixelShader *PS;
            ID3D11Buffer *CBuffer; // the pointer to the constant buffer
            
            ID3D11InputLayout *Layout;  
            InitPipeline( Dev, Devcon,
                         &VS,
                         &PS,
                         &Layout, 
                         &CBuffer);
            
            ID3D11Buffer *VBuffer;
            ID3D11Buffer *IBuffer;
            ID3D11ShaderResourceView *Texture;    // the pointer to the texture
            
            InitGraphics(Dev, Devcon, &VBuffer, &IBuffer, &Texture);
            
            
            
            
            ID3D11RasterizerState *RSDefault;   
            ID3D11RasterizerState *RSWireframe; 
            ID3D11BlendState *BS;
            ID3D11SamplerState *SamplerState;
            InitStates(Dev, &RSDefault, &RSWireframe, &BS, &SamplerState);
            
            
            m4 MatRotate, MatView, MatProjection, MatFinal, MatModel;
            gb_mat4_identity(&MatFinal);
            gb_mat4_identity(&MatModel);
            gb_mat4_identity(&MatView);
            gb_mat4_identity(&MatProjection);
            gb_mat4_identity(&MatRotate);
            
            gb_mat4_perspective(&MatProjection, gb_to_radians(90.0f), (float)Width/(float)Height, 0.1f, 100.0f);
            
            gb_mat4_look_at(&MatView,
                            {0.0f, 0.0f, 5.0f},    // the camera position
                            {0.0f, 0.0f, 0.0f},    // the look-at position
                            {0.0f, 1.0f, 0.0f});
            
            
            time_info TimeInfo = {};
            while(RunLoop(&TimeInfo, Running, 60))
            {
                MSG Message; 
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                
                
                //Devcon->RSSetState(RSWireframe);
                Devcon->RSSetState(RSDefault);
                Devcon->OMSetBlendState(BS, 0, 0xffffffff);
                Devcon->PSSetSamplers(0, 1, &SamplerState);
                
                ClearScreen(Devcon, Backbuffer, ZBuffer); 
                
                cbuffer ConstantB = {};
                
                ConstantB.DirLight.Direction = {1.0f, 1.0f, 1.0f, 0.0f};
                ConstantB.DirLight.Diffuse = {0.5f, 0.5f, 0.5f, 1.0f};
                ConstantB.DirLight.Ambient = {0.2f, 0.2f, 0.2f, 1.0f};
                
                //MatModel = {};
                float Scalar = 4.5f; 
                gb_mat4_scale(&MatModel, {Scalar, Scalar, Scalar});
                
                ConstantB.Model= MatModel; 
                
                MatFinal = MatProjection * MatView * MatModel;
                
                ConstantB.Final = MatFinal;
                
                Devcon->UpdateSubresource(CBuffer, 0, 0, &ConstantB, 0, 0);
                
                UINT Stride = sizeof(vertex);
                UINT Offset = 0;
                Devcon->IASetVertexBuffers(0, 1, &VBuffer, &Stride, &Offset);
                Devcon->IASetIndexBuffer(IBuffer, DXGI_FORMAT_R32_UINT, 0);
                
                Devcon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                
                Devcon->PSSetShaderResources(0, 1, &Texture);
                
                Devcon->DrawIndexed(6, 0, 0);
                
                Swapchain->Present(0, 0);
            }
            
            Swapchain->SetFullscreenState(FALSE, NULL);
        }
    }
    
    return 0; 
}