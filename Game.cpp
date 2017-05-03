#include "Game.h"
#include "Vertex.h"
#include "Particle.h"

#if defined(_DEBUG)
#include <dxgitype.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#pragma comment(lib, "dxgi.lib")
#endif

#include <WICTextureLoader.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	// Initialize fields
	//vertexBuffer = 0;
	//indexBuffer = 0;
	vertexShader = 0;
	pixelShader = 0;

	material1 = 0;
	material2 = 0;

	cameraRotating = false;
	prevMousePos = { 0, 0 };

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.");
#endif
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	//// Release any (and all!) DirectX objects
	//// we've made in the Game class
	//if (vertexBuffer) { vertexBuffer->Release(); }
	//if (indexBuffer) { indexBuffer->Release(); }

	// meshes will be release by its destructor

	if (frameCaptureInited)
		frameCapture->Release();

	delete particleEmitterCS;
	delete particleCS;
	delete particlePS;
	delete particleVS;

	bufDrawListSRV->Release();
	bufDrawListUAV->Release();
	bufDeadListUAV->Release();
	bufParticlesSRV->Release();
	bufParticlesUAV->Release();
	bufDrawList->Release();
	bufDeadList->Release();
	bufParticles->Release();
	bufEmitter->Release();
	bufParticleConstants->Release();
	bufIndirectDrawArgs->Release();
	bufQuadIndices->Release();

	// destroy materials
	delete material1;
	delete material2;

	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	delete vertexShader;
	delete pixelShader;
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateMatrices();
	CreateMaterials();
	CreateEntities();
	InitLights();
	InitParticles();

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files using
// my SimpleShader wrapper for DirectX shader manipulation.
// - SimpleShader provides helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = new SimpleVertexShader(device, context);
	//if (!vertexShader->LoadShaderFile(L"Debug/VertexShader.cso"))
	//	vertexShader->LoadShaderFile(L"VertexShader.cso");
	vertexShader->LoadShaderFile(L"Assets/Shaders/VertexShader.cso");

	pixelShader = new SimplePixelShader(device, context);
	//if (!pixelShader->LoadShaderFile(L"Debug/PixelShader.cso"))
	//	pixelShader->LoadShaderFile(L"PixelShader.cso");
	pixelShader->LoadShaderFile(L"Assets/Shaders/PixelShader.cso");

	// You'll notice that the code above attempts to load each
	// compiled shader file (.cso) from two different relative paths.

	// This is because the "working directory" (where relative paths begin)
	// will be different during the following two scenarios:
	//  - Debugging in VS: The "Project Directory" (where your .cpp files are) 
	//  - Run .exe directly: The "Output Directory" (where the .exe & .cso files are)

	// Checking both paths is the easiest way to ensure both 
	// scenarios work correctly, although others exist
}



// --------------------------------------------------------
// Initializes the matrices necessary to represent our geometry's 
// transformations and our 3D camera
// --------------------------------------------------------
void Game::CreateMatrices()
{
	// Set up world matrix
	// - In an actual game, each object will need one of these and they should
	//   update when/if the object moves (every frame)
	// - You'll notice a "transpose" happening below, which is redundant for
	//   an identity matrix.  This is just to show that HLSL expects a different
	//   matrix (column major vs row major) than the DirectX Math library
	// XMMATRIX W = XMMatrixIdentity();
	// XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(W)); // Transpose for HLSL!

	// Create the View matrix
	// - In an actual game, recreate this matrix every time the camera 
	//    moves (potentially every frame)
	// - We're using the LOOK TO function, which takes the position of the
	//    camera and the direction vector along which to look (as well as "up")
	// - Another option is the LOOK AT function, to look towards a specific
	//    point in 3D space
	//XMVECTOR pos = XMVectorSet(0, 0, -5, 0);
	//XMVECTOR dir = XMVectorSet(0, 0, 1, 0);
	//XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	//XMMATRIX V = XMMatrixLookToLH(
	//	pos,     // The position of the "camera"
	//	dir,     // Direction the camera is looking
	//	up);     // "Up" direction in 3D space (prevents roll)
	//XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V)); // Transpose for HLSL!

	//// Create the Projection matrix
	//// - This should match the window's aspect ratio, and also update anytime
	////   the window resizes (which is already happening in OnResize() below)
	//XMMATRIX P = XMMatrixPerspectiveFovLH(
	//	0.25f * 3.1415926535f,		// Field of View Angle
	//	(float)width / height,		// Aspect ratio
	//	0.1f,						// Near clip plane distance
	//	100.0f);					// Far clip plane distance
	//XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!

	camera.SetPosition(0, 0, -5);
	camera.SetPerspective((float)width / height, 0.25f * 3.1415926535f, 0.1f, 100.0f);
}

void Game::CreateMaterials()
{
	// Create material
	ID3D11ShaderResourceView* srv1 = nullptr;
	CreateWICTextureFromFile(device, context, L"Assets/Textures/wood.jpg", nullptr, &srv1);

	ID3D11ShaderResourceView* srv2 = nullptr;
	CreateWICTextureFromFile(device, context, L"Assets/Textures/rock.jpg", nullptr, &srv2);

	ID3D11SamplerState* sampler = nullptr;
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&samplerDesc, &sampler);

	material1 = new Material(vertexShader, pixelShader, srv1, sampler);
	material2 = new Material(vertexShader, pixelShader, srv2, sampler);

	srv1->Release();
	srv2->Release();
	sampler->Release();
}


// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateEntities()
{
	//// Create some temporary variables to represent colors
	//// - Not necessary, just makes things more readable
	//XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	//XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	//XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	//XMFLOAT4 yellow = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	//XMFLOAT4 cyan = XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f);

	//// Set up the vertices of the triangle we would like to draw
	//// - We're going to copy this array, exactly as it exists in memory
	////    over to a DirectX-controlled data structure (the vertex buffer)
	//Vertex vertices[] =
	//{
	//	{ XMFLOAT3(+0.0f, +1.0f, +0.0f), red },
	//	{ XMFLOAT3(+1.5f, -1.0f, +0.0f), blue },
	//	{ XMFLOAT3(-1.5f, -1.0f, +0.0f), green },
	//};

	//// Set up the indices, which tell us which vertices to use and in which order
	//// - This is somewhat redundant for just 3 vertices (it's a simple example)
	//// - Indices are technically not required if the vertices are in the buffer 
	////    in the correct order and each one will be used exactly once
	//// - But just to see how it's done...
	//int indices[] = { 0, 1, 2 };


	////// Create the VERTEX BUFFER description -----------------------------------
	////// - The description is created on the stack because we only need
	//////    it to create the buffer.  The description is then useless.
	////D3D11_BUFFER_DESC vbd;
	////vbd.Usage				= D3D11_USAGE_IMMUTABLE;
	////vbd.ByteWidth			= sizeof(Vertex) * 3;       // 3 = number of vertices in the buffer
	////vbd.BindFlags			= D3D11_BIND_VERTEX_BUFFER; // Tells DirectX this is a vertex buffer
	////vbd.CPUAccessFlags		= 0;
	////vbd.MiscFlags			= 0;
	////vbd.StructureByteStride	= 0;

	////// Create the proper struct to hold the initial vertex data
	////// - This is how we put the initial data into the buffer
	////D3D11_SUBRESOURCE_DATA initialVertexData;
	////initialVertexData.pSysMem = vertices;

	////// Actually create the buffer with the initial data
	////// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
	////device->CreateBuffer(&vbd, &initialVertexData, &vertexBuffer);



	////// Create the INDEX BUFFER description ------------------------------------
	////// - The description is created on the stack because we only need
	//////    it to create the buffer.  The description is then useless.
	////D3D11_BUFFER_DESC ibd;
	////ibd.Usage               = D3D11_USAGE_IMMUTABLE;
	////ibd.ByteWidth           = sizeof(int) * 3;         // 3 = number of indices in the buffer
	////ibd.BindFlags           = D3D11_BIND_INDEX_BUFFER; // Tells DirectX this is an index buffer
	////ibd.CPUAccessFlags      = 0;
	////ibd.MiscFlags           = 0;
	////ibd.StructureByteStride = 0;

	////// Create the proper struct to hold the initial index data
	////// - This is how we put the initial data into the buffer
	////D3D11_SUBRESOURCE_DATA initialIndexData;
	////initialIndexData.pSysMem = indices;

	////// Actually create the buffer with the initial data
	////// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
	////device->CreateBuffer(&ibd, &initialIndexData, &indexBuffer);

	//// Mesh 1 : a mesh using original vertices data
	//meshes.push_back(Mesh(device, vertices, sizeof(Vertex), 3, indices, 3));

	//// Mesh 2 : a quad, centered at origin point
	//Vertex vertices2[] =
	//{
	//	{ XMFLOAT3(-0.5f,  0.5f, 0.0f), red },
	//	{ XMFLOAT3(0.5f,  0.5f, 0.0f), green },
	//	{ XMFLOAT3(0.5f, -0.5f, 0.0f), blue },
	//	{ XMFLOAT3(-0.5f, -0.5f, 0.0f), yellow }
	//};

	//int indices2[] = { 0, 1, 2, 0, 2, 3 };

	//meshes.push_back(Mesh(device, vertices2, sizeof(Vertex), 4, indices2, 6));

	//// Mesh 3 : a pentagon, centered at origin point
	//Vertex vertices3[] =
	//{
	//	{ XMFLOAT3(), red },
	//	{ XMFLOAT3(), green },
	//	{ XMFLOAT3(), blue },
	//	{ XMFLOAT3(), yellow },
	//	{ XMFLOAT3(), cyan },
	//};

	//for (int i = 0; i < 5; i++)
	//{
	//	float angle = (XM_2PI / 5) * i;
	//	vertices3[i].Position = XMFLOAT3(sin(angle), cos(angle), 0.0f);
	//}

	//int indices3[] = { 0, 1, 2, 0, 2, 3, 0, 3, 4 };

	//meshes.push_back(Mesh(device, vertices3, sizeof(Vertex), 5, indices3, 9));

	// Create meshes
	meshes.push_back(Mesh(device, "Assets/Models/cone.obj"));
	meshes.push_back(Mesh(device, "Assets/Models/cube.obj"));
	meshes.push_back(Mesh(device, "Assets/Models/cylinder.obj"));
	meshes.push_back(Mesh(device, "Assets/Models/helix.obj"));
	meshes.push_back(Mesh(device, "Assets/Models/sphere.obj"));
	meshes.push_back(Mesh(device, "Assets/Models/torus.obj"));

	// Create entities :
	entities.push_back(Entity(meshes[0], material1));
	entities.push_back(Entity(meshes[1], material1));
	entities.push_back(Entity(meshes[2], material1));
	entities.push_back(Entity(meshes[3], material2));
	entities.push_back(Entity(meshes[4], material2));
	entities.push_back(Entity(meshes[5], material2));

	// Set initial transforms
	entities[0].SetScale(0.5f);
	entities[0].SetTranslation(0.0f, 1.0f, 0.0f);
	entities[1].SetScale(0.3f);
	entities[1].SetTranslation(2.0f, 1.0f, 0.0f);
	entities[2].SetTranslation(2.0f, -1.0f, 0.0f);
	entities[3].SetScale(0.5f);
	entities[3].SetTranslation(-2.0f, -1.0f, 0.0f);
	entities[4].SetTranslation(-2.0f, 1.0f, 0.0f);
	entities[4].SetScale(0.6f);
	entities[5].SetTranslation(0.0f, -1.0f, 0.0f);
}

void Game::InitLights()
{
	// setup light parameters
	{  // directional lights
		DirectionalLight light;
		light.AmbientColor = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		light.DiffuseColor = XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f);
		XMStoreFloat3(
			&(light.Direction),
			XMVector3Normalize(XMVectorSet(1.0f, 0.0f, -1.0f, 0.0f))
		);

		directionalLights.push_back(light);

		light.AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		light.DiffuseColor = XMFLOAT4(0.0f, 0.0f, 0.5f, 1.0f);
		XMStoreFloat3(
			&(light.Direction),
			XMVector3Normalize(XMVectorSet(-1.0f, 0.0f, -1.0f, 0.0f))
		);

		directionalLights.push_back(light);
	}
	{  // point lights
		PointLight light;
		light.AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		light.DiffuseColor = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
		light.PositionAndRadius = XMFLOAT4(0.0f, 0.0f, 0.0f, 8.0f);

		pointLights.push_back(light);
	}

	// upload lights to constant buffer;

	SimplePixelShader* ps = material1->GetPixelShader(); // two materials are sharing the same pixel shader

	if (directionalLights.size() > 0)
	{
		size_t buffer_size = sizeof(DirectionalLight) * 32;
		char* temp_buffer = new char[buffer_size];

		memset(temp_buffer, 0, buffer_size);
		memcpy(temp_buffer, reinterpret_cast<void*>(directionalLights.data()), sizeof(DirectionalLight) * directionalLights.size());

		ps->SetData(
			"directionalLights",
			temp_buffer,
			buffer_size
		);

		delete[] temp_buffer;
	}

	if (pointLights.size() > 0)
	{
		size_t buffer_size = sizeof(PointLight) * 32;
		char* temp_buffer = new char[buffer_size];

		memset(temp_buffer, 0, buffer_size);
		memcpy(temp_buffer, reinterpret_cast<void*>(pointLights.data()), sizeof(PointLight) * pointLights.size());

		ps->SetData(
			"pointLights",
			temp_buffer,
			buffer_size
		);

		delete[] temp_buffer;
	}

	ps->SetInt("directionalLightCount", directionalLights.size());
	ps->SetInt("pointLightCount", pointLights.size());
}

void Game::InitParticles()
{
	HRESULT hr = S_OK;

#if defined(_DEBUG)
	hr = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&frameCapture));
	frameCaptureInited = (hr == S_OK);

	if (frameCaptureInited)
		frameCaptureCount = 10;
	else
		frameCaptureCount = 0;
#endif



	emitterCount = 2;

	emitters[0].position = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
	emitters[0].velocity = XMFLOAT4(0.0f, 0.4f, 0.0f, 2.0f);
	emitters[0].emitCount = 0;
	emitters[0].deadParticles = 0;
	emitters[0].emitRate = 10;
	emitters[0].counter = 0;
	emitters[0].totalTime = 0;

	emitters[1].position = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
	emitters[1].velocity = XMFLOAT4(0.0f, 1.0f, 0.0f, 2.0f);
	emitters[1].emitCount = 0;
	emitters[1].deadParticles = 0;
	emitters[1].emitRate = 10;
	emitters[1].counter = 0;
	emitters[1].totalTime = 0;

	{
		//D3D11_MAPPED_SUBRESOURCE data = {};

		//context->UpdateSubresource(bufEmitter, 0, nullptr, emitters, 2 * sizeof(Emitter), 2 * sizeof(Emitter));
		/*assert(S_OK == context->Map(bufEmitters, 0, D3D11_MAP_WRITE_DISCARD, 0, &data));
		memcpy(data.pData, emitters, 2 * sizeof(Emitter));
		context->Unmap(bufEmitters, 0);*/
	}



	frameCount = 0;
}

void Game::UpdateParticles(float deltaTime, float totalTime)
{
	particleConstants.deltaTime = deltaTime;

	particleEmitterCS->SetShader();
	particleEmitterCS->SetFloat("totalTime", totalTime);

	for (uint32_t i = 0; i < emitterCount; i++)
	{
		emitters[i].counter += deltaTime * emitters[i].emitRate;
		emitters[i].emitCount = static_cast<uint32_t>(emitters[i].counter); // floor of uint
		emitters[i].counter -= emitters[i].emitCount;

		if (0 == emitters[i].emitCount)
			continue;

#if defined(_DEBUG)
		if (frameCaptureCount > 0)
		{
			frameCapture->BeginCapture();
		}
#endif
		particleEmitterCS->SetFloat4("position", emitters[i].position);
		particleEmitterCS->SetFloat4("velocity", emitters[i].velocity);
		particleEmitterCS->SetInt("emitCount", emitters[i].emitCount);

		if (particleFirstUpdate)
		{
			particleEmitterCS->SetUnorderedAccessView("deadList", bufDeadListUAV, particleConstants.maxParticles);
			particleEmitterCS->SetInt("deadParticles", particleConstants.maxParticles);
			particleFirstUpdate = false;
		}
		else
		{
			particleEmitterCS->SetUnorderedAccessView("deadList", bufDeadListUAV);
			context->CopyStructureCount(bufEmitter, offsetof(Emitter, deadParticles), bufDeadListUAV);
		}
		particleEmitterCS->SetUnorderedAccessView("particles", bufParticlesUAV);

		particleEmitterCS->CopyAllBufferData();
		particleEmitterCS->DispatchByThreads(emitters[i].emitCount, 1, 1);

#if defined(_DEBUG)
		if (frameCaptureCount > 0)
		{
			frameCapture->EndCapture();
			frameCaptureCount--;
		}
#endif
	}


	particleCS->SetUnorderedAccessView("particles", bufParticlesUAV);
	particleCS->SetUnorderedAccessView("deadList", bufDeadListUAV);
	particleCS->SetUnorderedAccessView("drawList", bufDrawListUAV, 0);

	particleCS->SetFloat("deltaTime", particleConstants.deltaTime);
	particleCS->SetShader();
	particleCS->CopyAllBufferData();
	particleCS->DispatchByThreads(particleConstants.maxParticles, 1, 1);

	{
		ID3D11UnorderedAccessView* nulls[] = { nullptr, nullptr, nullptr };
		uint32_t initVals[] = { -1, -1, -1 };
		context->CSSetUnorderedAccessViews(0, 3, nulls, initVals);
	}

	frameCount++;
}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	// Update our projection matrix since the window size changed
	//XMMATRIX P = XMMatrixPerspectiveFovLH(
	//	0.25f * 3.1415926535f,	// Field of View Angle
	//	(float)width / height,	// Aspect ratio
	//	0.1f,				  	// Near clip plane distance
	//	100.0f);			  	// Far clip plane distance
	//XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!

	camera.SetPerspective((float)width / height, 0.25f * 3.1415926535f, 0.1f, 100.0f);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	// Camera movement
	float speed = 10.0f;
	float velForward = 0, velRight = 0, velAxisY = 0;
	if (GetAsyncKeyState('W') & 0x8000) velForward += speed * deltaTime;
	if (GetAsyncKeyState('S') & 0x8000) velForward -= speed * deltaTime;
	if (GetAsyncKeyState('A') & 0x8000) velRight -= speed * deltaTime;
	if (GetAsyncKeyState('D') & 0x8000) velRight += speed * deltaTime;
	if (GetAsyncKeyState(' ') & 0x8000) velAxisY += speed * deltaTime;
	if (GetAsyncKeyState('X') & 0x8000) velAxisY -= speed * deltaTime;

	camera.MoveAlongDirection(velForward, velRight, 0.0f);
	camera.MoveAlongWorldAxes(0.0f, velAxisY, 0.0f);

	// Objects
	float cosTime = cos(totalTime * 2);
	float sinTime = sin(totalTime * 2);

	entities[0].SetRotation(0.0f, 0.0f, totalTime);

	entities[1].SetTranslation(
		2.0f + 0.5f * sinTime,
		1.0f + 0.5f * cosTime,
		0.0f
	);

	entities[2].SetScale(0.25f * cosTime + 0.75f, 0.25f * sinTime + 0.75f, 1.0f);

	entities[3].SetRotation(0.0f, XM_PIDIV2 * sinTime * 0.7f, 0.0f);

	entities[4].SetRotation(0.0f, 0.0f, -totalTime * 5);

	entities[5].SetRotation(totalTime, 0.0f, 0.0f);

	//if (frameCount <= 1000)
	{
		UpdateParticles(deltaTime, totalTime);
	}
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	//const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
	const float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	// iterate through and draw all entities
	//UINT offset = 0;
	//for (auto iter = entities.begin(); iter != entities.end(); ++iter)
	//{
	//	Entity& entity = *iter;

	//	// setup material
	//	entity.PrepareMaterial(camera.GetViewMatrix(), camera.GetProjectionMatrix());

	//	// draw it
	//	Mesh& mesh = entity.GetMesh();

	//	ID3D11Buffer* vb = mesh.GetVertexBuffer();
	//	ID3D11Buffer* ib = mesh.GetIndexBuffer();
	//	UINT stride = mesh.GetStride();

	//	if (nullptr == vb) continue;

	//	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	//	vb->Release();

	//	if (nullptr != ib)
	//	{
	//		context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	//		ib->Release();

	//		context->DrawIndexed(mesh.GetIndexCount(), 0, 0);
	//	}
	//	else
	//	{
	//		context->Draw(mesh.GetVertexCount(), 0);
	//	}
	//}

	// Particle System

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(nullptr);
	context->IASetIndexBuffer(bufQuadIndices, DXGI_FORMAT_R32_UINT, 0);
	particleVS->SetShaderResourceView("particles", bufParticlesSRV);
	particleVS->SetShaderResourceView("drawList", bufDrawListSRV);
	particleVS->SetMatrix4x4("view", camera.GetViewMatrix());
	particleVS->SetMatrix4x4("projection", camera.GetProjectionMatrix());
	particleVS->CopyAllBufferData();
	particleVS->SetShader();

	particlePS->SetShader();

	context->CopyStructureCount(bufIndirectDrawArgs, 4, bufDrawListUAV);
	context->CopyStructureCount(bufIndirectDrawArgs, 24, bufDeadListUAV);
	context->DrawIndexedInstancedIndirect(bufIndirectDrawArgs, 0);
	//context->DrawIndexedInstanced(6, 100, 0, 0, 0);
	{
		ID3D11ShaderResourceView* nulls[] = { nullptr, nullptr, nullptr };
		context->VSSetShaderResources(0, 3, nulls);
	}

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);
}


#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...
	if (buttonState & 0x0001) cameraRotating = true;

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...
	if (!(buttonState & 0x0001)) cameraRotating = false;

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...
	if (cameraRotating)
	{
		float angularSpeed = 0.003f;
		float yaw = camera.GetYaw();
		float pitch = camera.GetPitch();
		float deltaX = (x - prevMousePos.x) * angularSpeed;
		float deltaY = (y - prevMousePos.y) * angularSpeed;
		camera.SetRotation(
			yaw + deltaX,
			pitch + deltaY
		);
		//printf("[%.6f, %.6f] [%.6f, %.6f]\n", deltaX, deltaY, yaw, pitch);
	}

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}
#pragma endregion