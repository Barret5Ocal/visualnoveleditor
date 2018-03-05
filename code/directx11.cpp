#include <d3d11.h>

//#include "vshader.h"
//#include "pshader.h"

#include <d3dcompiler.h>

void InitializeD3D(IDXGISwapChain **Swapchain,             // the pointer to the swap chain interface
                   ID3D11Device **Dev,                     // the pointer to our Direct3D device interface
                   ID3D11DeviceContext **Devcon,           // the pointer to our Direct3D device context
                   ID3D11RenderTargetView **Backbuffer,    // the pointer to our back buffer
                   ID3D11DepthStencilView **ZBuffer, 
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
    
    D3D11_TEXTURE2D_DESC Texd = {};
    Texd.Width = Width; 
    Texd.Height = Height; 
    Texd.ArraySize = 1; 
    Texd.MipLevels = 1; 
    Texd.SampleDesc.Count = 4; 
    Texd.Format = DXGI_FORMAT_D32_FLOAT;
    Texd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    
    ID3D11Texture2D *DepthBuffer;
    Dev[0]->CreateTexture2D(&Texd, 0, &DepthBuffer);
    
    D3D11_DEPTH_STENCIL_VIEW_DESC Dsvd = {};
    
    Dsvd.Format = DXGI_FORMAT_D32_FLOAT;
    Dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    
    Dev[0]->CreateDepthStencilView(DepthBuffer, &Dsvd, ZBuffer);
    DepthBuffer->Release(); 
    
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
    Viewport.MinDepth = 0;    // the closest an object can be on the depth buffer is 0.0
    Viewport.MaxDepth = 1;    // the farthest an object can be on the depth buffer is 1.0
    
    Devcon[0]->RSSetViewports(1, &Viewport);
    
}

struct vertex
{
    v3 Position;
    v3 Color; 
    v2 UV; 
};

struct dir_light
{
    v4 Direction;
    v4 Diffuse;
    v4 Ambient;
};

struct point_light 
{    
    v3 Position;
    
    float Constant;
    float Linear;
    float Quadratic;  
    
    v4 Ambient;
    v4 Diffuse;
};

struct cbuffer
{
    m4 Model;
    m4 Final;
    dir_light DirLight;
};

void InitPipeline(ID3D11Device *Dev, ID3D11DeviceContext *Devcon,
                  ID3D11VertexShader **VS,
                  ID3D11PixelShader **PS,
                  ID3D11InputLayout **Layout,
                  ID3D11Buffer **CBuffer)
{
    
    char *Code = R"SHA(
    
struct dir_light
{
    float4 Direction;
    float4 Diffuse;
    float4 Ambient;
};

    cbuffer ConstantBuffer
    {
        float4x4 model;
        float4x4 final;
        
        dir_light DirLight[1];
}

    struct VOut
    {
        float4 color : COLOR;
        float2 texcoord : TEXCOORD;    // texture coordinates
        float4 position : SV_POSITION;
    };
    
    Texture2D Texture;
    SamplerState ss; 
    
    float4 DirLightCalc(float4x4 rotationmat, float4 normalvec, float4 lightvector, float4 lightcolor)
    {
    
        float4 norm = normalize(mul(rotationmat, normalvec));
        float diffusebrightness = saturate(dot(norm, lightvector));
          return lightcolor * diffusebrightness;
          
}

    VOut VShader(float4 position : POSITION, float4 normal : NORMAL, float2 texcoord : TEXCOORD)
    {
        VOut output;
        
        
        output.position = mul(final, position);
        output.color = DirLight[0].Ambient;
        
        output.color += DirLightCalc(model, normal, DirLight[0].Direction, DirLight[0].Diffuse);
        
        
        
        output.texcoord = texcoord;    // set the texture coordinates, unmodified
        
        return output;
    }
    
    float4 PShader(float4 color : COLOR, float2 texcoord : TEXCOORD) : SV_TARGET
    {
        float4 newcolor = color * Texture.Sample(ss, texcoord);
        newcolor.a = 1.0f;
        return newcolor;
    }
    )SHA";
    
    ID3DBlob *Errors; 
    
    ID3DBlob *VSCompiled; 
    ID3DBlob *PSCompiled; 
    D3DCompile(Code, strlen(Code), 0, 0, 0, "VShader", "vs_4_0", 0, 0, &VSCompiled, &Errors);
    D3DCompile(Code, strlen(Code), 0, 0, 0, "PShader", "ps_4_0", 0, 0, &PSCompiled, &Errors);
    
    if(S_OK != Dev->CreateVertexShader(VSCompiled->GetBufferPointer(), VSCompiled->GetBufferSize(), 0, VS))
        InvalidCodePath; 
    
    if(S_OK != Dev->CreatePixelShader(PSCompiled->GetBufferPointer(), PSCompiled->GetBufferSize(), 0, PS))
        InvalidCodePath;
    
    Devcon->VSSetShader(VS[0], 0, 0);
    Devcon->PSSetShader(PS[0], 0, 0);
    
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    
    
    // NOTE(Barret5Ocal): make sure to update this function when you change the ied
    Dev->CreateInputLayout(ied, ArrayCount(ied), VSCompiled->GetBufferPointer(), VSCompiled->GetBufferSize(), Layout);
    Devcon->IASetInputLayout(Layout[0]);
    
    D3D11_BUFFER_DESC BD = {};
    
    BD.Usage = D3D11_USAGE_DEFAULT; 
    BD.ByteWidth = sizeof(cbuffer);
    BD.BindFlags = D3D11_BIND_CONSTANT_BUFFER; 
    
    Dev->CreateBuffer(&BD, 0, CBuffer);
    Devcon->VSSetConstantBuffers(0, 1, CBuffer);
}

void InitGraphics(ID3D11Device *Dev, ID3D11DeviceContext *Devcon, ID3D11Buffer **VBuffer, ID3D11Buffer **IBuffer, ID3D11ShaderResourceView **Texture)
{
#if 1
    vertex Vertices[] =
    {
        {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    };
#else 
    vertex Vertices[] =
    {
        {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
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
#if 1
        0, 1, 2, 
        0, 3, 1,
#else 
        0, 1, 3,
        0, 2, 1, 
#endif 
    };
    
    D3D11_BUFFER_DESC IBD = {};
    IBD.Usage = D3D11_USAGE_DYNAMIC;
    IBD.ByteWidth = sizeof(DWORD) * ArrayCount(Indices);
    IBD.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IBD. CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    IBD.MiscFlags = 0;
    
    Dev->CreateBuffer(&IBD, 0, IBuffer);
    
    int is =sizeof(Indices);  
    Devcon->Map(IBuffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &MS);
    memcpy(MS.pData, Indices, sizeof(Indices));
    Devcon->Unmap(IBuffer[0], 0);
    
    //int sx,sy,sn;
    //unsigned char *SD = stbi_load("StructuredArt.png", &sx, &sy, &sn, 4);
    
    // NOTE(Barret5Ocal): Texture Stuff
    // TODO(Barret5Ocal): Look into more Texture stuff 
    int x,y,n;
    unsigned char *data = stbi_load("bedroom_test.png", &x, &y, &n, 4);
    
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
    
#if 1
    D3D11_SUBRESOURCE_DATA  SubData = {}; 
    SubData.pSysMem = data; 
    SubData.SysMemPitch = x * 4;
    SubData.SysMemSlicePitch = 4 * x * y; 
#endif
    
    ID3D11Texture2D *pTexture = NULL;
    HRESULT Result = Dev->CreateTexture2D( &desc,
                                          &SubData,
                                          &pTexture );
    if(Result != S_OK)
        InvalidCodePath;
    Result = Dev->CreateShaderResourceView(pTexture, 0, Texture);
    if(Result != S_OK)
        InvalidCodePath;
#if 0
    Devcon->Map(pTexture, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &MS);
    memcpy(MS.pData, data, n * x * y);
    Devcon->Unmap(pTexture, 0);
#endif 
    // NOTE(Barret5Ocal): End Texture Stuff
}

// initializes the states
void InitStates(ID3D11Device *Dev, ID3D11RasterizerState **RSDefault,   
                ID3D11RasterizerState **RSWireframe, ID3D11BlendState **BS,ID3D11SamplerState **pSS)
{
    D3D11_RASTERIZER_DESC rd;
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_BACK;
    rd.FrontCounterClockwise = FALSE;
    rd.DepthClipEnable = TRUE;
    rd.ScissorEnable = FALSE;
    rd.AntialiasedLineEnable = TRUE;
    rd.MultisampleEnable = FALSE;
    rd.DepthBias = 0;
    rd.DepthBiasClamp = 0.0f;
    rd.SlopeScaledDepthBias = 0.0f;
    
    Dev->CreateRasterizerState(&rd, RSDefault);
    
    // set the changed values for wireframe mode
    rd.FillMode = D3D11_FILL_WIREFRAME;
    rd.AntialiasedLineEnable = TRUE;
    
    Dev->CreateRasterizerState(&rd, RSWireframe);
    
    D3D11_BLEND_DESC BD = {};
    BD.RenderTarget[0].BlendEnable = TRUE;
    BD.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    BD.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    BD.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    BD.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    BD.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    BD.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    BD.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    BD.IndependentBlendEnable = FALSE;
    BD.AlphaToCoverageEnable = FALSE;
    
    Dev->CreateBlendState(&BD, BS);
    
    D3D11_SAMPLER_DESC sd;
    
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.MaxAnisotropy = 8;    // use Anisotropic x 8
    sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;      // horizontally the texture is repeated
    sd.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;    // vertically the texture is mirrored
    sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;     // if it's a 3D texture, it is clamped
    sd.BorderColor[0] = 1.0f;    // set the border color to white
    sd.BorderColor[1] = 1.0f;
    sd.BorderColor[2] = 1.0f;
    sd.BorderColor[3] = 1.0f;
    sd.MinLOD = 0.0f;
    sd.MaxLOD = FLT_MAX;
    sd.MipLODBias = 2.0f;    // decrease mip level of detail by 2
    
    Dev->CreateSamplerState(&sd, pSS);
}
