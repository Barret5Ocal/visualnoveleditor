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

#include "vshader.h"
#include "pshader.h"

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


void InitializeD3D(IDXGISwapChain **Swapchain,             // the pointer to the swap chain interface
                   ID3D11Device **Dev,                     // the pointer to our Direct3D device interface
                   ID3D11DeviceContext **Devcon,           // the pointer to our Direct3D device context
                   ID3D11RenderTargetView **Backbuffer,    // the pointer to our back buffer
                   int Width, int Height, HWND Window)
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
    
    D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, 0, 0, D3D11_SDK_VERSION, &SwapChainDesc, Swapchain, Dev, 0, Devcon);
    
    
    ID3D11Texture2D *BackBufferTexture;
    Swapchain[0]->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&BackBufferTexture);
    
    Dev[0]->CreateRenderTargetView(BackBufferTexture, 0, Backbuffer);
    BackBufferTexture->Release(); 
    
    // set the render target as the back buffer
    Devcon[0]->OMSetRenderTargets(1, Backbuffer, NULL);
    
    D3D11_VIEWPORT Viewport = {};
    
    Viewport.TopLeftX = 0;
    Viewport.TopLeftY = 0;
    Viewport.Width = Width; 
    Viewport.Height = Height; 
    //Viewport.MinDepth = 0;    // the closest an object can be on the depth buffer is 0.0
    //Viewport.MaxDepth = 1;    // the farthest an object can be on the depth buffer is 1.0
    
    Devcon[0]->RSSetViewports(1, &Viewport);
    
}

struct vertex
{
    v3 Position;
    v4 Color; 
    v2 UV; 
};

void InitPipeline(ID3D11Device *Dev, ID3D11DeviceContext *Devcon,
                  ID3D11VertexShader **VS,
                  ID3D11PixelShader **PS,
                  ID3D11InputLayout **Layout,
                  ID3D11Buffer **CBuffer)
{
    // NOTE(Barret5Ocal): Init Pipeline
    if(S_OK != Dev->CreateVertexShader(g_VShader, sizeof(g_VShader), 0, VS))
        InvalidCodePath; 
    
    if(S_OK != Dev->CreatePixelShader(g_PShader, sizeof(g_PShader), 0, PS))
        InvalidCodePath;
    
    Devcon->VSSetShader(VS[0], 0, 0);
    Devcon->PSSetShader(PS[0], 0, 0);
    
    // create the input element object
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    
    // NOTE(Barret5Ocal): make sure to update this function when you change the ied
    Dev->CreateInputLayout(ied, 3, g_VShader, sizeof(g_VShader), Layout);
    Devcon->IASetInputLayout(Layout[0]);
    
    D3D11_BUFFER_DESC BD = {};
    
    BD.Usage = D3D11_USAGE_DEFAULT; 
    BD.ByteWidth = 64;
    BD.BindFlags = D3D11_BIND_CONSTANT_BUFFER; 
    
    Dev->CreateBuffer(&BD, 0, CBuffer);
    Devcon->VSSetConstantBuffers(0, 1, CBuffer);
}

void InitGraphics(ID3D11Device *Dev, ID3D11DeviceContext *Devcon, ID3D11Buffer **VBuffer, ID3D11Buffer **IBuffer, ID3D11ShaderResourceView **Texture)
{
#if 1
    vertex Vertices[]
    {
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // Top Left
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // Bottom Right 
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // Bottom Left 
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // Top Right 
    };
    
#else 
    vertex Vertices[]
    {
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},// Top Left
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}}, // Bottom Right 
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},  // Bottom Left 
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}}, // Top Right 
    };
#endif 
    
    D3D11_BUFFER_DESC BD = {};
    BD.Usage = D3D11_USAGE_DYNAMIC;
    BD.ByteWidth = sizeof(vertex) * ArrayCount(Vertices);
    BD.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    BD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    Dev->CreateBuffer(&BD, 0, VBuffer);
    
    D3D11_MAPPED_SUBRESOURCE MS; 
    Devcon->Map(VBuffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &MS);
    memcpy(MS.pData, Vertices, sizeof(Vertices));
    Devcon->Unmap(VBuffer[0], 0);
    
    DWORD Indices[] = 
    {
        0, 1, 2, 
        0, 3, 1
    };
    
    D3D11_BUFFER_DESC IBD = {};
    IBD.Usage = D3D11_USAGE_DYNAMIC;
    IBD.ByteWidth = sizeof(DWORD) * ArrayCount(Indices);
    IBD.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IBD. CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    IBD.MiscFlags = 0;
    
    Dev->CreateBuffer(&IBD, 0, IBuffer);
    
    Devcon->Map(IBuffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &MS);
    memcpy(MS.pData, Indices, sizeof(Indices));
    Devcon->Unmap(IBuffer[0], 0);
    
    
    // NOTE(Barret5Ocal): Texture Stuff
    // TODO(Barret5Ocal): Look into more Texture stuff 
    int x,y,n;
    unsigned char *data = stbi_load("Wood.png", &x, &y, &n, 4);
    
    
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = x;
    desc.Height = y;
    desc.MipLevels = desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT; //D3D11_USAGE_DYNAMIC; 
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0; //D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    
    D3D11_SUBRESOURCE_DATA  SubData = {}; 
    SubData.pSysMem = data; 
    SubData.SysMemPitch = x * 4;
    SubData.SysMemSlicePitch = 4 * x * y; 
    
    ID3D11Texture2D *pTexture = NULL;
    HRESULT Result = Dev->CreateTexture2D( &desc, &SubData, &pTexture );
    if(Result != S_OK)
        InvalidCodePath;
    Result = Dev->CreateShaderResourceView(pTexture, 0, Texture);
    if(Result != S_OK)
        InvalidCodePath;
#if 0
    Devcon->Map(pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &MS);
    memcpy(MS.pData, data, n * x * y);
    Devcon->Unmap(pTexture, 0);
#endif 
    // NOTE(Barret5Ocal): End Texture Stuff
}



struct cbuffer
{
    m4 Final;
};

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
            
            InitializeD3D(&Swapchain, &Dev, &Devcon, &Backbuffer,
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
            
            time_info TimeInfo = {};
            while(RunLoop(&TimeInfo, Running, 60))
            {
                MSG Message; 
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                
                cbuffer ConstantB = {};
                
                m4 MatRotate, MatView, MatProjection, MatFinal;
                gb_mat4_identity(&MatFinal);
                gb_mat4_identity(&MatView);
                gb_mat4_identity(&MatProjection);
                gb_mat4_identity(&MatRotate);
                
                static float Time = 0.0f; Time += 0.05f; 
                
                gb_mat4_rotate(&MatRotate, {0.0f, 1.0f, 0.0f}, Time);
                
                gb_mat4_perspective(&MatProjection, gb_to_radians(90.0f), (float)Width/(float)Height, 0.1f, 100.0f);
                
                gb_mat4_look_at(&MatView, {0.0f, 1.0f, -2.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
                ConstantB.Final  = MatProjection * MatView * MatRotate; 
                
                
                float Color[] = {0.0f, 0.2f, 0.4f, 1.0f};
                Devcon->ClearRenderTargetView(Backbuffer, Color);
                
                UINT Stride = sizeof(vertex);
                UINT Offset = 0;
                Devcon->IASetVertexBuffers(0, 1, &VBuffer, &Stride, &Offset);
                Devcon->IASetIndexBuffer(IBuffer, DXGI_FORMAT_R32_UINT, 0);
                
                Devcon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                
                Devcon->PSSetShaderResources(0, 1, &Texture);
                Devcon->UpdateSubresource(CBuffer, 0, 0, &ConstantB, 0, 0);
                
                Devcon->DrawIndexed(6, 0, 0);
                
                Swapchain->Present(0, 0);
            }
            
            Swapchain->SetFullscreenState(FALSE, NULL);
        }
    }
    
    
    return 0; 
}