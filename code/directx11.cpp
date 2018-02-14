#include <d3d11.h>

//#include "vshader.h"
//#include "pshader.h"

#include <d3dcompiler.h>

struct vertex
{
    v3 Position;
    v3 Color; 
    v2 UV; 
};

struct cbuffer
{
    m4 Final;
    m4 Rotation;
    v4 LightVector;
    v4 LightColor;
    v4 AmbientColor;
};

struct directx11_state
{
    IDXGISwapChain *Swapchain;             // the pointer to the swap chain interface
    ID3D11Device *Dev;                     // the pointer to our Direct3D device interface
    ID3D11DeviceContext *Devcon;           // the pointer to our Direct3D device context
    ID3D11RenderTargetView *Backbuffer;    // the pointer to our back buffer
    
    ID3D11DepthStencilView *ZBuffer;       // the pointer to our depth buffer
    
    ID3D11VertexShader *VS;
    ID3D11PixelShader *PS;
    ID3D11Buffer *CBuffer; // the pointer to the constant buffer
    
    ID3D11InputLayout *Layout;
    
    ID3D11Buffer *VBuffer;
    ID3D11Buffer *IBuffer;
    ID3D11ShaderResourceView *Texture;    // the pointer to the texture
    
    ID3D11RasterizerState *RSDefault;   
    ID3D11RasterizerState *RSWireframe; 
    ID3D11BlendState *BS;
    
};

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

void InitPipeline(ID3D11Device *Dev, ID3D11DeviceContext *Devcon,
                  ID3D11VertexShader **VS,
                  ID3D11PixelShader **PS,
                  ID3D11InputLayout **Layout,
                  ID3D11Buffer **CBuffer)
{
    char *Code = R"SHA(
    cbuffer ConstantBuffer
    {
        float4x4 final;
        float4x4 rotation;    // the rotation matrix
        float4 lightvec;      // the light's vector
        float4 lightcol;      // the light's color
        float4 ambientcol;    // the ambient light's color
    }
    
    struct VOut
    {
        float4 color : COLOR;
        float2 texcoord : TEXCOORD;    // texture coordinates
        float4 position : SV_POSITION;
    };
    
    Texture2D Texture;
    SamplerState ss; 
    
    VOut VShader(float4 position : POSITION, float4 normal : NORMAL, float2 texcoord : TEXCOORD)
    {
        VOut output;
        
        output.position = mul(final, position);
        output.color = ambientcol;
        
        float4 norm = normalize(mul(rotation, normal));
        float diffusebrightness = saturate(dot(norm, lightvec));
        output.color += lightcol * diffusebrightness;
        
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
    
    // NOTE(Barret5Ocal): Init Pipeline
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
    Dev->CreateInputLayout(ied, 3, VSCompiled->GetBufferPointer(), VSCompiled->GetBufferSize(), Layout);
    Devcon->IASetInputLayout(Layout[0]);
    
    D3D11_BUFFER_DESC BD = {};
    
    BD.Usage = D3D11_USAGE_DEFAULT; 
    BD.ByteWidth = 64;
    BD.BindFlags = D3D11_BIND_CONSTANT_BUFFER; 
    
    Dev->CreateBuffer(&BD, 0, CBuffer);
    Devcon->VSSetConstantBuffers(0, 1, CBuffer);
}

void D11LoadVertices(directx11_state *State, vertex *Vertices, int Count)
{
    D3D11_BUFFER_DESC BD = {};
    BD.Usage = D3D11_USAGE_DYNAMIC;
    BD.ByteWidth = sizeof(vertex) * Count;
    BD.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    BD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    State->Dev->CreateBuffer(&BD, 0, &State->VBuffer);
    
    D3D11_MAPPED_SUBRESOURCE MS; 
    State->Devcon->Map(State->VBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MS);
    memcpy(MS.pData, Vertices, sizeof(vertex) * Count);
    State->Devcon->Unmap(State->VBuffer, 0);
}

void D11LoadIndices(directx11_state *State, DWORD *Indices, int Count)
{
    
    D3D11_BUFFER_DESC IBD = {};
    IBD.Usage = D3D11_USAGE_DYNAMIC;
    IBD.ByteWidth = sizeof(DWORD) * Count;
    IBD.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IBD. CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    IBD.MiscFlags = 0;
    
    State->Dev->CreateBuffer(&IBD, 0, &State->IBuffer);
    
    D3D11_MAPPED_SUBRESOURCE MS; 
    State->Devcon->Map(State->IBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MS);
    memcpy(MS.pData, Indices, sizeof(DWORD) * Count);
    State->Devcon->Unmap(State->IBuffer, 0);
}

void D11LoadTexture(directx11_state *State, void *Memory, int Width, int Height)
{
    // TODO(Barret5Ocal): Look into more Texture stuff 
    
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = Width;
    desc.Height = Height;
    desc.MipLevels = desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT; //D3D11_USAGE_DYNAMIC; 
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0; //D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    
    D3D11_SUBRESOURCE_DATA  SubData = {}; 
    SubData.pSysMem = Memory; 
    SubData.SysMemPitch = Width * 4;
    SubData.SysMemSlicePitch = 4 * Width * Height; 
    
    ID3D11Texture2D *pTexture = NULL;
    HRESULT Result = State->Dev->CreateTexture2D( &desc, &SubData, &pTexture );
    if(Result != S_OK)
        InvalidCodePath;
    Result = State->Dev->CreateShaderResourceView(pTexture, 0, &State->Texture);
    if(Result != S_OK)
        InvalidCodePath;
    
}

// initializes the states
void InitStates(ID3D11Device *Dev, ID3D11RasterizerState **RSDefault,   
                ID3D11RasterizerState **RSWireframe, ID3D11BlendState **BS)
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
}

void D11DrawIndexed(directx11_state *D11State, int Count, m4 *MatFinal, m4 *MatRotate)
{
    cbuffer ConstantB = {};
    
    ConstantB.Final  = *MatFinal;
    
    ConstantB.Rotation = *MatRotate;
    
    D11State->Devcon->ClearDepthStencilView(D11State->ZBuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);
    
    UINT Stride = sizeof(vertex);
    UINT Offset = 0;
    D11State->Devcon->IASetVertexBuffers(0, 1, &D11State->VBuffer, &Stride, &Offset);
    D11State->Devcon->IASetIndexBuffer(D11State->IBuffer, DXGI_FORMAT_R32_UINT, 0);
    
    D11State->Devcon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    D11State->Devcon->PSSetShaderResources(0, 1, &D11State->Texture);
    D11State->Devcon->UpdateSubresource(D11State->CBuffer, 0, 0, &ConstantB, 0, 0);
    
    D11State->Devcon->DrawIndexed(Count, 0, 0);
    
}

void D11Clear(directx11_state *D11State, float Color[4])
{
    D11State->Devcon->ClearRenderTargetView(D11State->Backbuffer, Color);
}

void D11PresentFrame(directx11_state *D11State)
{
    D11State->Swapchain->Present(0, 0);
    
}

#if 0
void RenderFrame(int Width, int Height, directx11_state *D11State)
{
    
    cbuffer ConstantB = {};
    
    ConstantB.LightVector = {1.0f, 1.0f, 1.0f, 0.0f};
    ConstantB.LightColor = {0.5f, 0.5f, 0.5f, 1.0f};
    ConstantB.AmbientColor = {0.2f, 0.2f, 0.2f, 1.0f};
    
    ConstantB.Final  = MatProjection * MatView * MatRotate; 
    
    //Devcon->RSSetState(RSWireframe);
    D11State->Devcon->RSSetState(D11State->RSDefault);
    D11State->Devcon->OMSetBlendState(D11State->BS, 0, 0xffffffff);
    
    float Color[] = {0.0f, 0.2f, 0.4f, 1.0f};
    D11State->Devcon->ClearRenderTargetView(D11State->Backbuffer, Color);
    
    // clear the depth buffer
    D11State->Devcon->ClearDepthStencilView(D11State->ZBuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);
    
    UINT Stride = sizeof(vertex);
    UINT Offset = 0;
    D11State->Devcon->IASetVertexBuffers(0, 1, &D11State->VBuffer, &Stride, &Offset);
    D11State->Devcon->IASetIndexBuffer(D11State->IBuffer, DXGI_FORMAT_R32_UINT, 0);
    
    D11State->Devcon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    D11State->Devcon->PSSetShaderResources(0, 1, &D11State->Texture);
    D11State->Devcon->UpdateSubresource(D11State->CBuffer, 0, 0, &ConstantB, 0, 0);
    
    D11State->Devcon->DrawIndexed(6, 0, 0);
    
    
}
#endif 