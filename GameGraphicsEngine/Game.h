#pragma once

#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>
#include "Mesh.h"
#include "Entity.h"
#include "Camera.h"
#include "Material.h"
#include "Lights.h"
#include "WICTextureLoader.h"
#include <SpriteFont.h>
#include "Player.h"
#include "Target.h"
#include "DDSTextureLoader.h"

class Game 
	: public DXCore
{
public:

	enum GAME_STATES {
		START,
		PLAY,
		PAUSE,
		END
	};

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
	void LoadSpriteFont();
	void CreateMatrices();
	void CreateBasicGeometry();

	void InitForPlay();
	void ResetRenderStates();
	void LoadTargets();

	// GameStates enum to track where we are in the game loop
	GAME_STATES currentState;

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	// Shaders for aiming reticle
	SimpleVertexShader* aimingVertexShader;
	SimplePixelShader* aimingPixelShader;

	// Shaders for the CubeMap/Skybox
	SimpleVertexShader* skyVertexShader;
	SimplePixelShader* skyPixelShader;

	// The matrices to go from model space to screen space
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;

	// Mesh variables
	Mesh* sphereMesh;
	Mesh* helixMesh;
	Mesh* cubeMesh;
	Mesh* reticleMesh; // Simple diamond-shaped mesh to serve as aiming reticle

	// Entity variables
	Entity* entityOne;
	Entity* entityTwo;
	Entity* entityThree;
	Entity* reticleEntity;

	// Camera variable
	Camera* gameCamera;

	// Material variables
	Material* mat1;
	Material* mat2;
	Material* reticleMat;

	// Create a Light
	DirectionalLight dirLight;
	DirectionalLight dirLight2;

	// Textures
	ID3D11ShaderResourceView* rockSRV;
	ID3D11ShaderResourceView* woodSRV;
	ID3D11ShaderResourceView* normalMapBarkSRV;
	ID3D11ShaderResourceView* skySRV;
	ID3D11SamplerState* sampler;

	// Render States
	ID3D11RasterizerState* skyRastState;
	ID3D11DepthStencilState* skyDepthState;

	// Spritefont members (for text)
	SpriteBatch* spriteBatch;
	SpriteFont* spriteFont;

	// Player
	Player* player;

	// Target Array
	vector<Target*> targets;

	bool needNewTarget = false;
	bool spacePressed = false;
};

