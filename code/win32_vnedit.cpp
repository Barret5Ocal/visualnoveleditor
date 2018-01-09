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

#include <d3d11.h>

// global declarations
IDXGISwapChain *Swapchain;             // the pointer to the swap chain interface
ID3D11Device *Dev;                     // the pointer to our Direct3D device interface
ID3D11DeviceContext *Devcon;           // the pointer to our Direct3D device context
ID3D11RenderTargetView *Backbuffer;    // the pointer to our back buffer

ID3D11InputLayout *Layout;            // the pointer to the input layout
ID3D11VertexShader *VS;               // the pointer to the vertex shader
ID3D11PixelShader *PS;                // the pointer to the pixel shader
ID3D11Buffer *VBuffer;                // the pointer to the vertex buffer
ID3D11Buffer *CBuffer;                // the pointer to the constant buffer
ID3D11DepthStencilView *ZBuffer;       // the pointer to our depth buffer
ID3D11Buffer *IBuffer;                // the pointer to the index buffer

#include "vshader.h"
#include "pshader.h"

#include "direct3d_math.h"

// a struct to define a single vertex
struct vertex
{
    v3 Position;
    //FLOAT X, Y, Z; 
    v3 Normal;
};

struct cbuffer
{
    m4 Final;
    m4 Rotation;
    v4 LightVector;
    v4 LightColor;
    v4 AmbientColor; 
};

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

// state objects
ID3D11RasterizerState *pRSDefault;     // the default rasterizer state
ID3D11RasterizerState *pRSWireframe;   // a rasterizer using wireframe
ID3D11RasterizerState *pRS;
// initializes the states
void InitStates()
{
    D3D11_RASTERIZER_DESC rd;
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_BACK;
    rd.FrontCounterClockwise = FALSE;
    rd.DepthClipEnable = TRUE;
    rd.ScissorEnable = FALSE;
    rd.AntialiasedLineEnable = FALSE;
    rd.MultisampleEnable = FALSE;
    rd.DepthBias = 0;
    rd.DepthBiasClamp = 0.0f;
    rd.SlopeScaledDepthBias = 0.0f;
    
    Dev->CreateRasterizerState(&rd, &pRSDefault);
    
    // set the changed values for wireframe mode
    rd.FillMode = D3D11_FILL_WIREFRAME;
    rd.AntialiasedLineEnable = TRUE;
    
    Dev->CreateRasterizerState(&rd, &pRSWireframe);
    
    
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_BACK;
    rd.FrontCounterClockwise = FALSE;
    rd.DepthClipEnable = FALSE;    // turn off depth clipping
    
    Dev->CreateRasterizerState(&rd, &pRS);
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
            DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
            
            SwapChainDesc.BufferCount = 1;
            SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            SwapChainDesc.BufferDesc.Width = Width;
            SwapChainDesc.BufferDesc.Height= Height; 
            SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            SwapChainDesc.OutputWindow = Window;
            SwapChainDesc.SampleDesc.Count = 4; 
            SwapChainDesc.Windowed = true; 
            SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
            
            D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, 0, 0, D3D11_SDK_VERSION, &SwapChainDesc, &Swapchain, &Dev, 0, &Devcon);
            
            // create the depth buffer texture
            D3D11_TEXTURE2D_DESC texd = {};
            
            texd.Width = Width;
            texd.Height = Height;
            texd.ArraySize = 1;
            texd.MipLevels = 1;
            texd.SampleDesc.Count = 4;
            texd.Format = DXGI_FORMAT_D32_FLOAT;
            texd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            
            ID3D11Texture2D *DepthBuffer;
            Dev->CreateTexture2D(&texd, NULL, &DepthBuffer);
            
            // create the depth buffer
            D3D11_DEPTH_STENCIL_VIEW_DESC dsvd = {};
            
            dsvd.Format = DXGI_FORMAT_D32_FLOAT;
            dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
            
            Dev->CreateDepthStencilView(DepthBuffer, &dsvd, &ZBuffer);
            DepthBuffer->Release();
            
            
            ID3D11Texture2D *BackBufferTexture;
            Swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&BackBufferTexture);
            
            Dev->CreateRenderTargetView(BackBufferTexture, 0, &Backbuffer);
            BackBufferTexture->Release(); 
            
            Devcon->OMSetRenderTargets(1, &Backbuffer, ZBuffer);
            
            D3D11_VIEWPORT Viewport = {};
            
            Viewport.TopLeftX = 0;
            Viewport.TopLeftY = 0;
            Viewport.Width = Width; 
            Viewport.Height = Height; 
            Viewport.MinDepth = 0;    // the closest an object can be on the depth buffer is 0.0
            Viewport.MaxDepth = 1;    // the farthest an object can be on the depth buffer is 1.0
            
            Devcon->RSSetViewports(1, &Viewport);
            
            // NOTE(Barret5Ocal): Init Pipeline
            if(S_OK != Dev->CreateVertexShader(g_VShader, sizeof(g_VShader), 0, &VS))
                InvalidCodePath; 
            
            if(S_OK != Dev->CreatePixelShader(g_PShader, sizeof(g_PShader), 0, &PS))
                InvalidCodePath;
            
            Devcon->VSSetShader(VS, 0, 0);
            Devcon->PSSetShader(PS, 0, 0);
            
            // create the input element object
            D3D11_INPUT_ELEMENT_DESC ied[] =
            {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                //{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
                {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            };
            
            Dev->CreateInputLayout(ied, 2, g_VShader, sizeof(g_VShader), &Layout);
            Devcon->IASetInputLayout(Layout);
            
            D3D11_BUFFER_DESC BD = {};
            
            BD.Usage = D3D11_USAGE_DEFAULT;
            BD.ByteWidth = 176; 
            BD.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            
            Dev->CreateBuffer(&BD, 0, &CBuffer);
            Devcon->VSSetConstantBuffers(0, 1, &CBuffer);
            
            // NOTE(Barret5Ocal): Init Graphics
            // this creates the shape to render
            vertex OurVertices[] =
            {
                {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},    // side 1
                {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
                {{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
                {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
                
                {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}},    // side 2
                {{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}},
                {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}},
                {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}},
                
                {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},    // side 3
                {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
                {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
                {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
                
                {{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}},    // side 4
                {{1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}},
                {{-1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
                {{1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
                
                {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},    // side 5
                {{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
                {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
                {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
                
                {{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}},    // side 6
                {{-1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
                {{-1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}},
                {{-1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
                
            };
            
            D3D11_BUFFER_DESC VBD = {};
            
            VBD.Usage = D3D11_USAGE_DYNAMIC;
            VBD.ByteWidth = sizeof(vertex) * ArrayCount(OurVertices); 
            VBD.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            VBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; 
            
            Dev->CreateBuffer(&VBD, 0, &VBuffer);
            
            D3D11_MAPPED_SUBRESOURCE MS;
            Devcon->Map(VBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MS);
            memcpy(MS.pData, OurVertices, sizeof(OurVertices));
            Devcon->Unmap(VBuffer, 0);
            
            // create the index buffer out of DWORDs
            DWORD OurIndices[] = 
            {
                0, 1, 2,    // side 1
                2, 1, 3,
                4, 5, 6,    // side 2
                6, 5, 7,
                8, 9, 10,    // side 3
                10, 9, 11,
                12, 13, 14,    // side 4
                14, 13, 15,
                16, 17, 18,    // side 5
                18, 17, 19,
                20, 21, 22,    // side 6
                22, 21, 23,
            };
            
            D3D11_BUFFER_DESC IBD = {};
            
            // create the index buffer
            IBD.Usage = D3D11_USAGE_DYNAMIC;
            IBD.ByteWidth = sizeof(DWORD) * ArrayCount(OurIndices);
            IBD.BindFlags = D3D11_BIND_INDEX_BUFFER;
            IBD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            IBD.MiscFlags = 0;
            
            Dev->CreateBuffer(&IBD, NULL, &IBuffer);
            
            Devcon->Map(IBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &MS);    // map the buffer
            memcpy(MS.pData, OurIndices, sizeof(OurIndices));                   // copy the data
            Devcon->Unmap(IBuffer, NULL);
            
            InitStates();
            
            time_info TimeInfo = {};
            while(RunLoop(&TimeInfo, Running, 60))
            {
                MSG Message; 
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                
                cbuffer Cbuffer = {};
                
                Cbuffer.LightVector = {1.0f, 1.0f, 1.0f, 0.0f};
                Cbuffer.LightColor = {0.5f, 0.5f, 0.5f, 1.0f};
                Cbuffer.AmbientColor = {0.2f, 0.2f, 0.2f, 1.0f};
                
                m4 MatRotate, MatView, MatProjection, MatFinal;
                gb_mat4_identity(&MatFinal);
                gb_mat4_identity(&MatView);
                gb_mat4_identity(&MatProjection);
                gb_mat4_identity(&MatRotate);
                
                static float Time = 52.8494797; //Time += 0.05f; 
                
                gb_mat4_rotate(&MatRotate, {0.0f, 1.0f, 0.0f}, Time);
                
                gb_mat4_perspective(&MatProjection, gb_to_radians(90.0f), (float)Width/(float)Height, 0.1f, 100.0f);
                
                gb_mat4_look_at(&MatView, {0.0f, 3.0f, -5.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
                
                // NOTE(Barret5Ocal): REMEMBER THAT THE MATRIX MULTIPLY ORDER MATTERS
                Cbuffer.Final  = MatProjection * MatView * MatRotate; 
                Cbuffer.Rotation = MatRotate;
                
                //Devcon->RSSetState(pRSWireframe);
                Devcon->RSSetState(pRS);
                
                float Color[] = {0.0f, 0.2f, 0.4f, 1.0f};
                Devcon->ClearRenderTargetView(Backbuffer, Color);
                
                // clear the depth buffer
                Devcon->ClearDepthStencilView(ZBuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);
                
                UINT Stride = sizeof(vertex);
                UINT Offset = 0;
                Devcon->IASetVertexBuffers(0, 1, &VBuffer, &Stride, &Offset);
                Devcon->IASetIndexBuffer(IBuffer, DXGI_FORMAT_R32_UINT, 0);
                
                Devcon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                
                // draw the Hypercraft
                Devcon->UpdateSubresource(CBuffer, 0, 0, &Cbuffer, 0, 0);
                Devcon->DrawIndexed(36, 0, 0);
                
                Swapchain->Present(0, 0);
            }
            
        }
    }
    Swapchain->SetFullscreenState(FALSE, NULL);
    return 0; 
}