#include "Application.h"
#include "time.h"
#include "stdlib.h"
#include "iostream"
#include "DDSTextureLoader.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pVertexBuffer = nullptr;
	_pIndexBuffer = nullptr;
	_pConstantBuffer = nullptr;
    _pPyramidVertexBuffer = nullptr;
    _pPyramidIndexBuffer = nullptr;
    _pSkyboxIndexBuffer = nullptr;
    _pSkyboxVertexBuffer = nullptr;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}

    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

    //
	// Initialize the world matrix for all the object in the scene
    //
    XMStoreFloat4x4(_cubeObject.GetWorld(), XMMatrixIdentity());
	XMStoreFloat4x4(&_world, XMMatrixIdentity());
    XMStoreFloat4x4(_pyramidObject.GetWorld(), XMMatrixIdentity());
    XMStoreFloat4x4(_skybox.GetWorld(), XMMatrixIdentity());
    for (size_t i = 0; i < 5; i++)
    {
        XMStoreFloat4x4(&_world4[i], XMMatrixIdentity());
    }

#pragma region Setting Up Camera Values
    //
    // Initialize the view matrix (default value for the cameras)
    //
    XMFLOAT3 Eye = XMFLOAT3(0.0f, 0.0f, -3.0f);     //Default Eye value
    XMFLOAT3 At = XMFLOAT3(0.0f, 0.0f, 1.0f);       //Default At value
    XMFLOAT3 Up = XMFLOAT3(0.0f, 1.0f, 0.0f);       //Default Up value

    _camera1 = Camera(Eye, At, Up, _WindowWidth, _WindowHeight, 0.01f, 100.0f);     //Initialise the first camera with default values

    XMFLOAT3 Eye2 = XMFLOAT3(7.0f, 0.0f, -4.0f);                                    //Set different Eye value for the Second camera
    _camera2 = Camera(Eye2, At, Up, _WindowWidth, _WindowHeight, 0.01f, 100.0f);    //Initialise the Second camera

    XMFLOAT3 Eye3 = XMFLOAT3(13.0f, 0.0f, -3.0f);                                   //Set different Eye value for the Third camera
    _camera3 = Camera(Eye3, At, Up, _WindowWidth, _WindowHeight, 0.01f, 100.0f);    //Initialise the Third camera

    _currentCamera = _camera1;                      //Setting default current camera to first camera
    _view = _currentCamera.GetView();               //Passing View from camera
    _projection = _currentCamera.GetProjection();   //Passing projection from camera
#pragma endregion

#pragma region Lighting
    //
    //Initializing the Lights
    // 
    
    //Diffuse Setup
    DiffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);        //Set Diffuse Lighting (color) Values
    DiffuseMatieral = XMFLOAT4(0.5f, 1.0f, 1.0f, 1.0f);     //Set Diffuse Material Values

    //Ambient Setup
    AmbientLight = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) ;       //Set Ambient Lighting (color) Values
    AmbientMaterial = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);     //Set Ambient Material Values

    //SpecularSetup
    SpecularLight = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);       //Set Specular Lighting (color) Values
    SpecularMat = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);         //Set Specular Material Values
    SpecularPower = 10.0f;                                  //Set Specular Intensity Value

    directionToLight = XMFLOAT3(0.0f, 0.5f, -0.5f);         //Setup light direction
    EyeWorldPos = XMFLOAT3(0.0f, 0.0f, -3.0f);              //Setup View Direction
#pragma endregion

    //
    // Initializing other Varaibles
    // 
    
    //FogSetup
    fogStart = 0.0f;
    fogEnd = 3.0f;

    //Default boolean value for UINT
    enableSkyBox = 0;
    fogEnable = 0;

    //Intializing TEXTURE 
    CreateDDSTextureFromFile(_pd3dDevice, L"Crate_COLOR.dds", nullptr, &_pTextureRV);   //Create crate Texture from File
    CreateDDSTextureFromFile(_pd3dDevice, L"SkyBox1.dds", nullptr, &_pSkyboxTexture);   //Create Skybox Texture from File

    // Creating the sample state
    D3D11_SAMPLER_DESC sampDesc;

    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    _pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);   //Create linear sampler
    _pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);      //Set Linear Sampler
	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.hlsl", "VS", "vs_4_0", &pVSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The HLSL file cannot be compiled. Check VS Outpot for Error Log.", L"Error", MB_OK);
        return hr;
    }

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

	if (FAILED(hr))
	{	
		pVSBlob->Release();
        return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.hlsl", "PS", "ps_4_0", &pPSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The HLSL file cannot be compiled. Check VS Outpot for Error Log.", L"Error", MB_OK);
        return hr;
    }

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

    if (FAILED(hr))
        return hr;

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
                                        pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();

	if (FAILED(hr))
        return hr;

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);


	return hr;
}

HRESULT Application::InitVertexBuffer()
{
	HRESULT hr;

#pragma region Cube Vertices
    // Create vertex buffer
    SimpleVertex vertices[] =
    {
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f),   XMFLOAT2 (0,0)},
        { XMFLOAT3(1.0f, 1.0f, -1.0f),  XMFLOAT3(1.0f, 1.0f, -1.0f),    XMFLOAT2(1,0) },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f),XMFLOAT3(-1.0f, -1.0f, -1.0f),  XMFLOAT2(0,1) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f),   XMFLOAT2(1,1) },

        { XMFLOAT3(-1.0f, 1.0f, 1.0f),  XMFLOAT3(-1.0f, 1.0f, 1.0f),    XMFLOAT2(1,0) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f),   XMFLOAT3(1.0f, 1.0f, 1.0f),     XMFLOAT2(0,0) },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f),   XMFLOAT2(1,1) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f),  XMFLOAT3(1.0f, -1.0f, 1.0f),    XMFLOAT2(0,1) },
    };

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 8;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;

    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer);

    //Storing Vertex buffer in the gameobject class
    pCubeData = _cubeObject.GetMeshData();
    pCubeData->VertexBuffer = _pVertexBuffer;

#pragma endregion

#pragma region Pyramid Vertices
    SimpleVertex PyramidVertices[] =
    {
        { XMFLOAT3(-1.0f, -1.0f, -1.0f),XMFLOAT3(-1.0f, -1.0f, -1.0f)},
        { XMFLOAT3(0.0f, 2.0f, 0.0f),   XMFLOAT3(0.0f, 2.0f, 0.0f)},
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f)},

        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f)},
        { XMFLOAT3(1.0f, -1.0f, 1.0f),  XMFLOAT3(1.0f, -1.0f, 1.0f)},
    };

    D3D11_BUFFER_DESC pyramidbd;
    ZeroMemory(&pyramidbd, sizeof(pyramidbd));
    pyramidbd.Usage = D3D11_USAGE_DEFAULT;
    pyramidbd.ByteWidth = sizeof(SimpleVertex) * 6;
    pyramidbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    pyramidbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA PyramidInitData;
    ZeroMemory(&PyramidInitData, sizeof(PyramidInitData));
    PyramidInitData.pSysMem = PyramidVertices;

    hr = _pd3dDevice->CreateBuffer(&pyramidbd, &PyramidInitData, &_pPyramidVertexBuffer);

    //Storing Vertex buffer in the gameobject class
    pPyramidData = _pyramidObject.GetMeshData();
    pPyramidData->VertexBuffer = _pPyramidVertexBuffer;
#pragma endregion

#pragma region SkyBox Vertices
    SimpleVertex SkyboxVertices[] =
    {
        { XMFLOAT3(-10.0f, 10.0f, -10.0f), XMFLOAT3(-3.0f, 3.0f, -3.0f),   XMFLOAT2(0,0)},
        { XMFLOAT3(10.0f, 10.0f, -10.0f),  XMFLOAT3(3.0f, 3.0f, -3.0f),    XMFLOAT2(1,0) },
        { XMFLOAT3(-10.0f, -10.0f, -10.0f),XMFLOAT3(-3.0f, -3.0f, -3.0f),  XMFLOAT2(0,1) },
        { XMFLOAT3(10.0f, -10.0f, -10.0f), XMFLOAT3(3.0f, -3.0f, -3.0f),   XMFLOAT2(1,1) },

        { XMFLOAT3(-15.0f, 15.0f, 15.0f),  XMFLOAT3(-3.0f, 3.0f, 3.0f),    XMFLOAT2(1,0) },
        { XMFLOAT3(15.0f, 15.0f, 15.0f),   XMFLOAT3(3.0f, 3.0f, 3.0f),     XMFLOAT2(0,0) },
        { XMFLOAT3(-15.0f, -15.0f, 15.0f), XMFLOAT3(-3.0f, -3.0f, 3.0f),   XMFLOAT2(1,1) },
        { XMFLOAT3(15.0f, -15.0f, 15.0f),  XMFLOAT3(3.0f, -3.0f, 3.0f),    XMFLOAT2(0,1) },
    };

    D3D11_BUFFER_DESC skyboxbd;
    ZeroMemory(&skyboxbd, sizeof(skyboxbd));
    skyboxbd.Usage = D3D11_USAGE_DEFAULT;
    skyboxbd.ByteWidth = sizeof(SimpleVertex) * 8;
    skyboxbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    skyboxbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA SkyboxInitData;
    ZeroMemory(&SkyboxInitData, sizeof(SkyboxInitData));
    SkyboxInitData.pSysMem = SkyboxVertices;

    hr = _pd3dDevice->CreateBuffer(&skyboxbd, &SkyboxInitData, &_pSkyboxVertexBuffer);

    //Storing Vertex buffer in the gameobject class
    pSkyboxData = _skybox.GetMeshData();
    pSkyboxData->VertexBuffer = _pSkyboxVertexBuffer;
#pragma endregion


    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitIndexBuffer()
{
	HRESULT hr;

#pragma region Cube Indices
    // Create index buffer
    WORD indices[] =
    {
        //Front
       0,1,2,
       2,1,3,

       //Right
       1,7,3,
       7,1,5,

       //Back
       5,6,7,
       6,5,4,

       //left
       4,2,6,
       2,4,0,

       //Top
       4,1,0,
       1,4,5,

       //Bottom
       3,6,2,
       6,3,7
    };

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indices;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer);

    //Storing Index buffer in the gameobject class
    pCubeData->IndexBuffer = _pIndexBuffer;
    pCubeData->IndexCount = 36;
#pragma endregion

#pragma region Pyramid Indices
    // Create index buffer
    WORD pyramidIndices[] =
    {
       0,1,2,

       2,1,4,

       4,1,3,

       3,1,0,

       0,2,4,

       0,3,4,
    };

    D3D11_BUFFER_DESC pyramidbd;
    ZeroMemory(&pyramidbd, sizeof(pyramidbd));

    pyramidbd.Usage = D3D11_USAGE_DEFAULT;
    pyramidbd.ByteWidth = sizeof(WORD) * 18;
    pyramidbd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    pyramidbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA PyramidInitData;
    ZeroMemory(&PyramidInitData, sizeof(PyramidInitData));
    PyramidInitData.pSysMem = pyramidIndices;
    hr = _pd3dDevice->CreateBuffer(&pyramidbd, &PyramidInitData, &_pPyramidIndexBuffer);

    //Storing Index buffer in the gameobject class
    pPyramidData->IndexBuffer = _pPyramidIndexBuffer;
    pPyramidData->IndexCount = 18;
#pragma endregion

#pragma region Skybox Indices
    WORD skyboxIndices[] =
    {
        //Front
       2,1,0,
       3,1,2,

       //Right
       3,7,1,
       5,1,7,

       //Back
       7,6,5,
       4,5,6,

       //left
       6,2,4,
       0,4,2,

       //Top
       0,1,4,
       5,4,1,

       //Bottom
       2,6,3,
       7,3,6
    };

    D3D11_BUFFER_DESC Skyboxbd;
    ZeroMemory(&Skyboxbd, sizeof(Skyboxbd));

    Skyboxbd.Usage = D3D11_USAGE_DEFAULT;
    Skyboxbd.ByteWidth = sizeof(WORD) * 36;
    Skyboxbd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    Skyboxbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA SkyboxInitData;
    ZeroMemory(&SkyboxInitData, sizeof(SkyboxInitData));
    SkyboxInitData.pSysMem = skyboxIndices;
    hr = _pd3dDevice->CreateBuffer(&Skyboxbd, &SkyboxInitData, &_pSkyboxIndexBuffer);

    //Storing Index buffer in the gameobject class
    pSkyboxData->IndexBuffer = _pSkyboxIndexBuffer;
    pSkyboxData->IndexCount = 36;
#pragma endregion

    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    RECT rc = {0, 0, 640, 480};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"DX11 Framework", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!_hWnd)
		return E_FAIL;

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

        if (pErrorBlob) pErrorBlob->Release();

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _WindowWidth;
    sd.BufferDesc.Height = _WindowHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        _driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;

    hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr))
        return hr;

    //D3D11_COMPARISON_LESS_EQUAL = 1;

    //Created depth/stencil desc vie and texture 2D
    D3D11_TEXTURE2D_DESC depthStencilDesc;
    depthStencilDesc.Width = _WindowWidth;
    depthStencilDesc.Height = _WindowHeight;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    _pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStancilBuffer);
    _pd3dDevice->CreateDepthStencilView(_depthStancilBuffer, nullptr, &_depthStancilView);

    _pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStancilView);

    //Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_WindowWidth;
    vp.Height = (FLOAT)_WindowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

	hr = InitShadersAndInputLayout();
    if (FAILED(hr)) { return S_FALSE; }

	InitVertexBuffer();

    // Set vertex buffer
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    _pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBuffer, &stride, &offset);
    _pImmediateContext->IASetVertexBuffers(0, 1, &_pPyramidIndexBuffer, &stride, &offset);
    _pImmediateContext->IASetVertexBuffers(0, 1, &_pSkyboxIndexBuffer, &stride, &offset);

	InitIndexBuffer();

    // Set index buffer
    _pImmediateContext->IASetIndexBuffer(_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    _pImmediateContext->IASetIndexBuffer(_pPyramidIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    _pImmediateContext->IASetIndexBuffer(_pSkyboxIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Initializing the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);    //created constent buffer

    D3D11_RASTERIZER_DESC wfdesc;
    ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
    wfdesc.FillMode = D3D11_FILL_WIREFRAME;
    wfdesc.CullMode = D3D11_CULL_NONE;
    hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_wireFrame);      //Create Wireframe

    D3D11_RASTERIZER_DESC soliddesc;
    ZeroMemory(&soliddesc, sizeof(D3D11_RASTERIZER_DESC)); 
    soliddesc.FillMode = D3D11_FILL_SOLID;
    soliddesc.CullMode = D3D11_CULL_NONE;
    hr = _pd3dDevice->CreateRasterizerState(&soliddesc, &_solidState);  //Create Solid state

    //
    //Object loaded
    //
    myObjectMeshData = OBJLoader::Load("donut.obj", _pd3dDevice, false); //Load Doughnut

    //
    //Blending Equation Setup
    //
    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(blendDesc));
    D3D11_RENDER_TARGET_BLEND_DESC rtbd;
    ZeroMemory(&rtbd, sizeof(rtbd));

    rtbd.BlendEnable = true;
    rtbd.SrcBlend = D3D11_BLEND_SRC_COLOR;
    rtbd.DestBlend = D3D11_BLEND_BLEND_FACTOR;
    rtbd.BlendOp = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.RenderTarget[0] = rtbd;
    hr = _pd3dDevice->CreateBlendState(&blendDesc, &Transparency);

    if (FAILED(hr))
        return hr;

    return S_OK;
}

void Application::Cleanup()
{
    if (_pImmediateContext) _pImmediateContext->ClearState();

    if (_pConstantBuffer) _pConstantBuffer->Release();
    if (_pVertexBuffer) _pVertexBuffer->Release();
    if (_pIndexBuffer) _pIndexBuffer->Release();
    if (_pVertexLayout) _pVertexLayout->Release();
    if (_pVertexShader) _pVertexShader->Release();
    if (_pPixelShader) _pPixelShader->Release();
    if (_pRenderTargetView) _pRenderTargetView->Release();
    if (_pSwapChain) _pSwapChain->Release();
    if (_pImmediateContext) _pImmediateContext->Release();
    if (_pd3dDevice) _pd3dDevice->Release();
    if (_wireFrame) _wireFrame->Release();
    if (_depthStancilView) _depthStancilView->Release();
    if (_depthStancilBuffer) _depthStancilBuffer->Release();
    if (Transparency) Transparency->Release();
    if (_pPyramidIndexBuffer) _pPyramidIndexBuffer->Release();
    if (_pPyramidVertexBuffer) _pPyramidVertexBuffer->Release();
    if (_pSkyboxIndexBuffer) _pSkyboxIndexBuffer->Release();
    if (_pSkyboxVertexBuffer) _pSkyboxVertexBuffer->Release();
}

void Application::Update()
{
    // Update our time
    static float t = 0.0f;

    if (_driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        t += (float) XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();

        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;

        t = (dwTimeCur - dwTimeStart) / 1000.0f;
    }

#pragma region Set  Rasterizer State
    // 
    // Switch between Stats
    // 
    
    if (GetAsyncKeyState(0x55) < 0)         // U Key Down
        _pImmediateContext->RSSetState(_solidState);        // Enable Solid State
    else if (GetAsyncKeyState(0x49) < 0)    // I Key Down
        _pImmediateContext->RSSetState(_wireFrame);         // Enable WireFrame
#pragma endregion

#pragma region Camera Update
    // 
    // Switch betwen Cameras
    // 

    if (GetAsyncKeyState(0x31) < 0)         // Number 1 Key
        _currentCamera = _camera1;
    else if (GetAsyncKeyState(0x32) < 0)    // Number 2 Key
        _currentCamera = _camera2;
    else if (GetAsyncKeyState(0x33) < 0)    // Number 3 Key
        _currentCamera = _camera3;

    _currentCamera.Update();
    _projection = _currentCamera.GetProjection();
    _view = _currentCamera.GetView();
#pragma endregion

#pragma region Enable Effects (fog & skybox)
    //
    // Activate/Deactive Fog & skybox texture
    // 

    if (GetAsyncKeyState(0x46) < 0)         // F key down
    {
        fogEnable = 1;                      // EnableFog
        enableSkyBox = 0;
    }
    else if (GetAsyncKeyState(0x44) < 0)    // D key down
    {
        enableSkyBox = 0;                   //Default mode
        fogEnable = 0;                      //Disbale All Effect
    }
    else if (GetAsyncKeyState(0x53) < 0)    //S key down
    {
        enableSkyBox = 1;                   // Enable Skybox 
        fogEnable = 0;
    }
#pragma endregion
    
#pragma region Set Objects Rotation
    // 
    // Set Cube Transform values
    // 
    _cubeObject.SetScale(XMFLOAT3(1.0f, 1.0f, 1.0f));
    _cubeObject.SetRotation(XMFLOAT3(t, t, 0.0f));
    _cubeObject.SetTranslation(XMFLOAT3(7.0f, 0.0f, 0.0f));
    _cubeObject.Update();

    // 
    // Set Cube2 Transform values
    // 
    _cubeObject2.SetScale(XMFLOAT3(1.0f, 1.0f, 1.0f));
    _cubeObject2.SetRotation(XMFLOAT3(0.0f, t, 0.0f));
    _cubeObject2.SetTranslation(XMFLOAT3(12.0f, 0.0f, 0.0f));
    _cubeObject2.Update();

    // 
    // Set Pyramid Transform value
    // 
    _pyramidObject.SetScale(XMFLOAT3(1.0f, 1.0f, 1.0f));
    _pyramidObject.SetRotation(XMFLOAT3(t, t, 0.0f));
    _pyramidObject.SetTranslation(XMFLOAT3(13.0f, 0.0f, 0.0f));
    _pyramidObject.Update();

    // 
    // Set Skybox Transform value
    // 
    _skybox.SetScale(XMFLOAT3(2.0f, 2.0f, 2.0f));
    _skybox.SetRotation(XMFLOAT3(0.0, 0.0, 0.0f));
    _skybox.SetTranslation(XMFLOAT3(0.0f, 0.0f, 0.0f));
    _skybox.Update();

    //
    // Set Multiple Triangle Transform
    // 
    int increment = 0;
    for (size_t i = 0; i < 5; i++)
    {
        //secound cube 
        XMStoreFloat4x4(&_world4[i], XMMatrixScaling(-0.5, 0.5, -0.5) * XMMatrixRotationY(t * 2) * XMMatrixTranslation(3.0f, 0.0f, 0.0f) *
            XMMatrixRotationY(t * 2 + increment) * XMMatrixTranslation(7.0f, 0.0f, 0.0f));

        increment++;
    }

    //
    //  Set Object loaded from File Transform
    //
    XMStoreFloat4x4(&_world, XMMatrixRotationX(t));
#pragma endregion
}

void Application::Draw()
{
    //
    // Clear the back buffer
    //

    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
    _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
    //clear stencil/depth view
    _pImmediateContext->ClearDepthStencilView(_depthStancilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    XMMATRIX world = XMLoadFloat4x4(&_world);
    XMMATRIX view = XMLoadFloat4x4(&_view);
    XMMATRIX projection = XMLoadFloat4x4(&_projection);

    //
    // Pass Values to constantBuffer
    //

    ConstantBuffer cb;
    cb.mWorld = XMMatrixTranspose(world);
    cb.mView = XMMatrixTranspose(view);
    cb.mProjection = XMMatrixTranspose(projection);
    //Set Light Direction
    cb.DireToLight = directionToLight;
    //Set Diffuse
    cb.DiffLight = DiffuseLight;
    cb.DiffMat = DiffuseMatieral;
    //Set Ambient
    cb.padding = 0;
    cb.AmbLight = AmbientLight;
    cb.AmbMat = AmbientMaterial;
    //Set Specular
    cb.SpecLight = SpecularLight;
    cb.SpecMat = SpecularMat;
    cb.SpecPower = SpecularPower;
    cb.EyeWorldPos = _currentCamera.GetEye();
    //Set Fog
    cb.startFog = fogStart;
    cb.endFog = fogEnd;
    //Set Boolean 
    cb.skyBoxEnable = enableSkyBox;
    cb.enableFog = fogEnable;

    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;

    //
    // Set Vertex and Index Buffers
    //

    //Pass values to shaders and constant buffer
    _pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
    _pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
    _pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
    _pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);
        
    //
    // Blend Setup
    // 

    float blendFactor[] = { 0.75f, 0.75f, 0.75f, 1.0f };                 //"fine-tune" the blending equation
    _pImmediateContext->OMSetBlendState(NULL, NULL, 0xffffffff);        //Set the default blend state (no blending) for opaque objects

    //
    // Set Texture
    //

    _pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV);       //Set crate Texture

    //
    // Draw Object from file using OBJLoader
    //

    _pImmediateContext->IASetVertexBuffers(0, 1, &myObjectMeshData.VertexBuffer, &myObjectMeshData.VBStride, &myObjectMeshData.VBOffset);
    _pImmediateContext->IASetIndexBuffer(myObjectMeshData.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    _pImmediateContext->DrawIndexed(myObjectMeshData.IndexCount, 0, 0);

    
    //
    // Render multiple Pyramid around the cube
    //

    _pImmediateContext->IASetVertexBuffers(0, 1, &_pyramidObject.GetMeshData()->VertexBuffer, &stride, &offset);      // Set vertex buffer for Pyramid
    _pImmediateContext->IASetIndexBuffer(_pyramidObject.GetMeshData()->IndexBuffer, DXGI_FORMAT_R16_UINT, 0);        // Set index buffer for Pyramid

    for (size_t i = 0; i < 5; i++)
    {
        world = XMLoadFloat4x4(&_world4[i]);                                                                         // Pass each Pyramid world
        cb.mWorld = XMMatrixTranspose(world);
        _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
        _pImmediateContext->DrawIndexed(_pyramidObject.GetMeshData()->IndexCount, 0, 0);                             // Pass each Pyramid index count
    }
 
    //
    //Rendering (2) single Cube
    //

    _pImmediateContext->IASetVertexBuffers(0, 1, &_cubeObject.GetMeshData()->VertexBuffer, &stride, &offset);        // Set vertex buffer for cube
    _pImmediateContext->IASetIndexBuffer(_cubeObject.GetMeshData()->IndexBuffer, DXGI_FORMAT_R16_UINT, 0);          //  Set index buffer
 
    world = XMLoadFloat4x4(_cubeObject2.GetWorld());                                        // Pass first Cube Object World
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(_cubeObject.GetMeshData()->IndexCount, 0, 0);           // Pass first Cube Object Index count
    
    //BLEND THE NEXT OBJECT
    _pImmediateContext->OMSetBlendState(Transparency, blendFactor, 0xffffffff);             // Set Blend state for next object

    world = XMLoadFloat4x4(_cubeObject.GetWorld());                                         // Pass second Cube Object world
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(_cubeObject.GetMeshData()->IndexCount, 0, 0);           // Pass second Cube Object Index count


    //
    // Render SkyBox
    //
    
    if (enableSkyBox == 1)
    {
        //Set the default blend state (no blending) for opaque objects
        _pImmediateContext->OMSetBlendState(NULL, NULL, 0xffffffff);

        _pImmediateContext->PSSetShaderResources(1, 1, &_pSkyboxTexture);   //Set Skybox Texture


        world = XMLoadFloat4x4(_skybox.GetWorld());                         // Update world in constant buffer                                
        cb.mWorld = XMMatrixTranspose(world);
        _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

        _pImmediateContext->IASetVertexBuffers(0, 1, &_skybox.GetMeshData()->VertexBuffer, &stride, &offset);       //Pass skybox vertex Buffer
        _pImmediateContext->IASetIndexBuffer(_skybox.GetMeshData()->IndexBuffer, DXGI_FORMAT_R16_UINT, 0);          //Pass skybox index Buffer
        _pImmediateContext->DrawIndexed(_skybox.GetMeshData()->IndexCount, 0, 0);                                   //Pass skybox Index Count
    }

    //
    // Present our back buffer to our front buffer
    //

    _pSwapChain->Present(0, 0);
}