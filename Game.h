#pragma once

#include "DXCore.h"
#include "SimpleShader.h"
#include "Mesh.h"
#include "Entity.h"
#include "Camera.h"
#include "Material.h"
#include "Lights.h"
#include "Emitter.h"
#include "Particle.h"
#include <DirectXMath.h>
#include <vector>

#if defined(_DEBUG)
#include <DXProgrammableCapture.h>
#endif

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

	// Overridden mouse input helper methods
	void OnMouseDown (WPARAM buttonState, int x, int y);
	void OnMouseUp	 (WPARAM buttonState, int x, int y);
	void OnMouseMove (WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta,   int x, int y);
private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	void CreateMatrices();
	void CreateMaterials();
	void CreateEntities();
	void InitLights(); 
	void InitParticles();

	void UpdateParticles(float deltaTime, float totalTime);

	//// Buffers to hold actual geometry data
	//ID3D11Buffer* vertexBuffer;
	//ID3D11Buffer* indexBuffer;

	// Meshes holding vertex and index buffers
	std::vector<Mesh> meshes;

	// 
	std::vector<Entity> entities;

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	// Materials
	Material* material1;
	Material* material2;

#if defined(_DEBUG)
	// Debug
	IDXGraphicsAnalysis*			frameCapture;
	bool							frameCaptureInited;
	uint							frameCaptureCount;
#endif

	// Particles
	struct {
		float						deltaTime;
		uint32_t					maxParticles;
		uint32_t					_padding1;
		uint32_t					_padding2;
	}								particleConstants;
	Emitter							emitters[MAX_EMITTERS];
	uint32_t						emitterCount;
	
	
	int								frameCount;

	// Lights
	std::vector<DirectionalLight>	directionalLights;
	std::vector<PointLight>			pointLights;

	// The matrices to go from model space to screen space
	// DirectX::XMFLOAT4X4 worldMatrix;
	// DirectX::XMFLOAT4X4 viewMatrix;
	// DirectX::XMFLOAT4X4 projectionMatrix;

	//
	Camera camera;
	bool cameraRotating;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;
};

