#include "pch.h"
#include "HardwareRasterizer.h"

#include "Effect.h"


dae::HardwareRasterizer::HardwareRasterizer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Initialize DirectX pipeline
	const HRESULT result = InitializeDirectX();

	if (result == S_OK)
	{
		m_IsInitialized = true;
		std::cout << "DirectX is initialized and ready!\n";
	}
	else
	{
		std::cout << "DirectX initialization failed!\n";
	}
}

dae::HardwareRasterizer::~HardwareRasterizer()
{
	if (m_pSamplerState) m_pSamplerState->Release();

	if (m_pRenderTargetView) m_pRenderTargetView->Release();
	if (m_pRenderTargetBuffer) m_pRenderTargetBuffer->Release();

	if (m_pDepthStencilView) m_pDepthStencilView->Release();
	if (m_pDepthStencilBuffer) m_pDepthStencilBuffer->Release();

	if (m_pSwapChain) m_pSwapChain->Release();

	if (m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();
	}
	if (m_pDevice) m_pDevice->Release();
}


void dae::HardwareRasterizer::Render(Mesh* pMesh) const
{

	//1. Set Primitive Topology
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//2. Set Input Layout
	m_pDeviceContext->IASetInputLayout(pMesh->GetInputLayout());

	//3. Set Vertex Buffer
	constexpr UINT stride = sizeof(Vertex);
	constexpr UINT offset = 0;


	auto* pvertexBuffer = pMesh->GetVertexBuffer();


	m_pDeviceContext->IASetVertexBuffers(0, 1, &pvertexBuffer, &stride, &offset);

	//4. Set IndexBuffer
	m_pDeviceContext->IASetIndexBuffer(pMesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);


	//5. Draw
	D3DX11_TECHNIQUE_DESC techDesc{};
	pMesh->GetEffect()->GetTechnique()->GetDesc(&techDesc);

	for (UINT p{ 0 }; p < techDesc.Passes; ++p)
	{
		pMesh->GetEffect()->GetTechnique()->GetPassByIndex(p)->Apply(0, m_pDeviceContext);
		m_pDeviceContext->DrawIndexed(pMesh->GetNumIndices(), 0, 0);
	}
}


void dae::HardwareRasterizer::HardwareRender(std::vector<Mesh*> pMeshes, bool isBackgroundUniform) const
{
	if (!m_IsInitialized) return;

	//1. Clear RTV & DSV

	ColorRGB clearColor{};

	if (isBackgroundUniform)
		clearColor = ColorRGB{ 0.1f, 0.1f, 0.1f };
	else
		clearColor = ColorRGB{ 0.39f, 0.59f, 0.93f };



	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	//2. Set Pipeline + Invoke Drawcalls (= render)

	for (Mesh* pMesh : pMeshes)
	{
		if(pMesh->IsActive())
			Render(pMesh);
	}


	//3. Present Backbuffer (swap)
	m_pSwapChain->Present(0, 0);
}



void dae::HardwareRasterizer::LoadSampleState(const D3D11_FILTER& filter, ID3D11Device* device, std::vector<Mesh*> pMeshes)
{
	// Create the SampleState description
	D3D11_SAMPLER_DESC sampleDesc{};
	sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampleDesc.MipLODBias = 0;
	sampleDesc.MinLOD = -D3D11_FLOAT32_MAX;;
	sampleDesc.MaxLOD = D3D11_FLOAT32_MAX;
	sampleDesc.MaxAnisotropy = 1;
	sampleDesc.Filter = filter;


	if (m_pSamplerState) m_pSamplerState->Release();


	HRESULT result{ device->CreateSamplerState(&sampleDesc, &m_pSamplerState) };
	if (FAILED(result))
		std::cout << "m_pSamplerState failed to load\n";

	for (Mesh* pMesh : pMeshes)
	{
		pMesh->UpdateSampleState(m_pSamplerState);
	}
}

void dae::HardwareRasterizer::LoadCullMode(D3D11_CULL_MODE cullingMode, const std::vector<Mesh*>& pMeshes)
{
	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = false;
	rasterizerDesc.CullMode = cullingMode;






	if (m_pCullingMode) m_pCullingMode->Release();

	// Create a new rasterizer state
	const HRESULT hr{ m_pDevice->CreateRasterizerState(&rasterizerDesc, &m_pCullingMode) };
	if (FAILED(hr)) std::wcout << L"m_pRasterizerState failed to load\n";

	for (Mesh* pMesh : pMeshes)
	{
		pMesh->UpdateCullMode(m_pCullingMode);
	}
}

void dae::HardwareRasterizer::NextSampleStateFilter(std::vector<Mesh*> pMeshes)
{
	D3D11_FILTER newFilter{};

	switch (m_SampleFilters)
	{
	case SampleStateFilters::POINT:

		m_SampleFilters = SampleStateFilters::LINEAR;
		newFilter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		std::cout << "FILTERING METHOD: LINEAR" << "\n";
		break;

	case SampleStateFilters::LINEAR:

		m_SampleFilters = SampleStateFilters::ANISOTROPIC;
		newFilter = D3D11_FILTER_ANISOTROPIC;
		std::cout << "FILTERING METHOD: ANISOTROPIC" << "\n";
		break;

	case SampleStateFilters::ANISOTROPIC:

		m_SampleFilters = SampleStateFilters::POINT;
		newFilter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		std::cout << "FILTERING METHOD: POINT" << "\n";
		break;
	}

	LoadSampleState(newFilter, m_pDevice, pMeshes);
}

void dae::HardwareRasterizer::NextCullingMode(std::vector<Mesh*> pMeshes, Mesh::CullMode cullMode)
{

	D3D11_CULL_MODE cullingMode{};

	switch (cullMode)
	{
		case Mesh::CullMode::Back:
		{
			cullingMode = D3D11_CULL_BACK;
			break;
		}
		case Mesh::CullMode::Front:
		{
			cullingMode = D3D11_CULL_FRONT;
			break;
		}
		case Mesh::CullMode::None:
		{
			cullingMode = D3D11_CULL_NONE;
			break;
		}
	}


	LoadCullMode(cullingMode, pMeshes);


}



ID3D11Device* dae::HardwareRasterizer::GetDevice()
{
	return m_pDevice;
}

HRESULT dae::HardwareRasterizer::InitializeDirectX()
{

	//1. Create Device & DeviceContext
	//=====
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
	uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel,
		1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);

	if (FAILED(result)) return result;


	//Create DXGI Factory
	IDXGIFactory1* pDxgiFactory{};
	result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));


	if (FAILED(result)) return result;


	//2. Create Swapchain
	//=====
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;



	//Get Handle (HWND) from the SDL Backbuffer
	SDL_SysWMinfo sysWMInfo{};
	SDL_VERSION(&sysWMInfo.version)
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
	swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

	//Create SwapChain
	result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
	if (pDxgiFactory) pDxgiFactory->Release();




	if (FAILED(result)) return result;



	//3. Create DepthStencil (DS) & DepthStencilView (DSV)
	//Resource
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;


	//View
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
	if (FAILED(result)) return result;

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(result)) return result;



	//4. Create RenderTarget (RT) & RenderTargetView (RTV)
	//=====

	//Resource
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
	if (FAILED(result)) return result;

	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
	if (FAILED(result)) return result;


	//5. Bind RTV & DSV to Output Merger Stage
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);



	//6. Set Viewport
	//====
	D3D11_VIEWPORT viewPort{};
	viewPort.Width = static_cast<float>(m_Width);
	viewPort.Height = static_cast<float>(m_Height);
	viewPort.TopLeftX = 0.f;
	viewPort.TopLeftY = 0.f;
	viewPort.MinDepth = 0.f;
	viewPort.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &viewPort);



	return S_OK;
}