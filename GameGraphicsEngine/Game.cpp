#include "Game.h"
#include "Vertex.h"

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
	vertexShader = 0;
	pixelShader = 0;
	aimingVertexShader = 0;
	aimingPixelShader = 0;
	skyVertexShader = 0;
	skyPixelShader = 0;
	shadowVS = 0;

	reticleMesh = 0;
	reticleEntity = 0;
	reticleMat = 0;

	spriteBatch = 0;
	spriteFont = 0;

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

	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	delete vertexShader;
	delete pixelShader;
	delete aimingVertexShader;
	delete aimingPixelShader;
	delete skyVertexShader;
	delete skyPixelShader;
	delete shadowVS;
	delete bloomPS;
	delete bloomVS;

	// Delete the Mesh's to clear memory
	if (sphereMesh) { delete(sphereMesh); }
	if (helixMesh) { delete(helixMesh); }
	if (cubeMesh) { delete(cubeMesh); }
	if (reticleMesh) { delete (reticleMesh); }
	// Delete Entities
	//if (entityOne) { delete(entityOne); }
	//if (entityTwo){ delete(entityTwo); }
	//if (entityThree) { delete(entityThree); }
	if (reticleEntity) { delete (reticleEntity); }
	if (floorEntity) { delete (floorEntity); }

	// Delete the Camera
	if (debugCamera) { delete (debugCamera); }

	// Delete Player
	if (player) { delete (player); }

	// Delete Targets
	if (targets.size() > 0) {
		for (unsigned int i = 0; i < targets.size(); ++i) {
			delete targets[i];
		}
	}

	// Delete Material
	if (mat1) { delete (mat1); }
	if (mat2) { delete (mat2); }
	if (reticleMat) { delete (reticleMat); }

	// Delete SpriteFont stuff
	if (spriteBatch) { delete spriteBatch; }
	if (spriteFont) { delete spriteFont; }

	// Release textures and sampler
	sampler->Release();
	shadowSampler->Release();
	rockSRV->Release();
	woodSRV->Release();
	//normalMapBarkSRV->Release();
	skySRV->Release();
	shadowSRV->Release();
	shadowDSV->Release();
	shadowRasterizer->Release();

	bloomSRV->Release();
	bloomRTV->Release();

}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Initialize to START game state
	currentState = START;

	// Load Textures
	CreateWICTextureFromFile(device, context, L"Debug/Assets/Textures/MossRock.tif", 0, &rockSRV);
	CreateWICTextureFromFile(device, context, L"Debug/Assets/Textures/TreeBark.tif", 0, &woodSRV);
	CreateWICTextureFromFile(device, context, L"Debug/Assets/Textures/TreeBarkNormals.tif", 0, &normalMapBarkSRV);
	CreateDDSTextureFromFile(device, L"Debug/Assets/Textures/SkyBox.dds", 0, &skySRV);

	D3D11_RASTERIZER_DESC rsDesc = {};
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_FRONT;
	rsDesc.DepthClipEnable = true;
	device->CreateRasterizerState(&rsDesc, &skyRastState);

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&dsDesc, &skyDepthState);

	// Create a Sampler State for Texture Sampling
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Device create sampler state
	device->CreateSamplerState(&samplerDesc, &sampler);

	// Create post process resources -----------------------------------------
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D* ppTexture;
	device->CreateTexture2D(&textureDesc, 0, &ppTexture);

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(ppTexture, &rtvDesc, &bloomRTV);

	// Create the Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	device->CreateShaderResourceView(ppTexture, &srvDesc, &bloomSRV);

	// We don't need the texture reference itself no mo'
	ppTexture->Release();
	
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	LoadSpriteFont();
	CreateMatrices();
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// Initializes game for transition from Start to Play state
void Game::InitForPlay() {
	shadowMapSize = 1024;

	ResetRenderStates();
	LoadShaders();
	CreateBasicGeometry();

	// Initialize the lighting
	dirLight.AmbietColor = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	dirLight.DiffuseColor = XMFLOAT4(1, 0, 0, 1);
	dirLight.Direction = XMFLOAT3(0, -1, 0);

	dirLight2.AmbietColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	dirLight2.DiffuseColor = XMFLOAT4(1, 1, 1, 1);
	dirLight2.Direction = XMFLOAT3(0, 0, 1);

	pixelShader->SetData("light", &dirLight, sizeof(DirectionalLight));
	pixelShader->SetData("light2", &dirLight2, sizeof(DirectionalLight));

	// ***** Shadow stuff *****
	// Texture to hold the shadow map
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapSize;
	shadowDesc.Height = shadowMapSize;
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality= 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	ID3D11Texture2D* shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, &shadowTexture);

	// Depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC shadDSDesc = {};
	shadDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(shadowTexture, &shadDSDesc, &shadowDSV);

	// Create shadow map's SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC shadSRVDesc = {};
	shadSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shadSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shadSRVDesc.Texture2D.MipLevels = 1;
	shadSRVDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(shadowTexture, &shadSRVDesc, &shadowSRV);
	// shadowTexture was only needed to create SRV, so release it here
	shadowTexture->Release();

	// Create special ('comparison') sampler state for shadows
	D3D11_SAMPLER_DESC shadowSampleDesc = {};
	shadowSampleDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampleDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampleDesc.BorderColor[0] = 1.0f;
	shadowSampleDesc.BorderColor[1] = 1.0f;
	shadowSampleDesc.BorderColor[2] = 1.0f;
	shadowSampleDesc.BorderColor[3] = 1.0f;
	device->CreateSamplerState(&shadowSampleDesc, &shadowSampler);

	// Create rasterizer state
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000;
	shadowRastDesc.DepthBiasClamp = 0.0f;
	shadowRastDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);
	// ***** End shadow stuff *****
}

// Resets various render states to their default values
void Game::ResetRenderStates() {
	// Once we set up our own states for these, we can reset to those, rather than the defaults
	context->OMSetDepthStencilState(0, 0);
	context->RSSetState(0);
	//context->OMSetBlendState(0, 0, 0); // Resetting the Blend State to default causes the geometry to vanish. Need to look into later
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
	if (!vertexShader->LoadShaderFile(L"Debug/VertexShader.cso"))
		vertexShader->LoadShaderFile(L"VertexShader.cso");

	pixelShader = new SimplePixelShader(device, context);
	if (!pixelShader->LoadShaderFile(L"Debug/PixelShader.cso"))
		pixelShader->LoadShaderFile(L"PixelShader.cso");

	pixelShader->SetSamplerState("Sampler", sampler);
	pixelShader->SetShaderResourceView("Rocks", rockSRV);
	pixelShader->SetShaderResourceView("Wood", woodSRV);
	pixelShader->SetShaderResourceView("NormalMap", normalMapBarkSRV);

	aimingVertexShader = new SimpleVertexShader(device, context);
	if (!aimingVertexShader->LoadShaderFile(L"Debug/AimingVertexShader.cso"))
		aimingVertexShader->LoadShaderFile(L"AimingVertexShader.cso");

	aimingPixelShader = new SimplePixelShader(device, context);
	if (!aimingPixelShader->LoadShaderFile(L"Debug/AimingPixelShader.cso"))
		aimingPixelShader->LoadShaderFile(L"AimingPixelShader.cso");

	skyVertexShader = new SimpleVertexShader(device, context);
	if (!skyVertexShader->LoadShaderFile(L"Debug/CubeVertexShader.cso"))
		skyVertexShader->LoadShaderFile(L"CubeVertexShader.cso");

	skyPixelShader = new SimplePixelShader(device, context);
	if (!skyPixelShader->LoadShaderFile(L"Debug/CubePixelShader.cso"))
		skyPixelShader->LoadShaderFile(L"CubePixelShader.cso");

	shadowVS = new SimpleVertexShader(device, context);
	if (!shadowVS->LoadShaderFile(L"Debug/ShadowVertexShader.cso"))
		shadowVS->LoadShaderFile(L"ShadowVertexShader.cso");

	bloomVS = new SimpleVertexShader(device, context);
	if (!bloomVS->LoadShaderFile(L"Debug/BloomVS.cso"))
		bloomVS->LoadShaderFile(L"BloomVS.cso");

	bloomPS = new SimplePixelShader(device, context);
	if (!bloomPS->LoadShaderFile(L"Debug/BloomPS.cso"))
		bloomPS->LoadShaderFile(L"BloomPS.cso");

	// Load our shaders into our materials
	mat1 = new Material(pixelShader, vertexShader, rockSRV, sampler);
	mat2 = new Material(pixelShader, vertexShader, woodSRV, sampler);
	reticleMat = new Material(aimingPixelShader, aimingVertexShader, 0, 0);

	// You'll notice that the code above attempts to load each
	// compiled shader file (.cso) from two different relative paths.

	// This is because the "working directory" (where relative paths begin)
	// will be different during the following two scenarios:
	//  - Debugging in VS: The "Project Directory" (where your .cpp files are) 
	//  - Run .exe directly: The "Output Directory" (where the .exe & .cso files are)

	// Checking both paths is the easiest way to ensure both 
	// scenarios work correctly, although others exist
}

// As the name suggests, this loads and initializes a SpriteFont and SpriteBatch
// so that we can draw text to the program window
void Game::LoadSpriteFont() {
	spriteBatch = new SpriteBatch(context);
	spriteFont = new SpriteFont(device, L"courier.spritefont");
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
	XMMATRIX W = XMMatrixIdentity();
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(W)); // Transpose for HLSL!

	// Set up the Game Camera with the View Matrix and Projection Matrix
	debugCamera = new Camera();
	debugCamera->SetProjectionMat(width, height);
	debugCamera->SetActive(false);

	player = new Player();
	player->SetProjectionMat(width, height);

	// Set up shadow matricies
	XMMATRIX shadView = XMMatrixLookAtLH(	XMVectorSet(0, 20, -20, 0),
											XMVectorSet(0, 0, 0, 0),
											XMVectorSet(0, 1, 0, 0));
	XMStoreFloat4x4(&shadowViewMatrix, XMMatrixTranspose(shadView));
	XMMATRIX shadProj = XMMatrixOrthographicLH(10, 10, 0.1f, 100.0f);
	XMStoreFloat4x4(&shadowProjectionMatrix, XMMatrixTranspose(shadProj));
}


// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	// Loading 3D Models
	sphereMesh = new Mesh("Debug/Assets/Models/sphere.obj", device);
	helixMesh = new Mesh("Debug/Assets/Models/helix.obj", device);
	cubeMesh = new Mesh("Debug/Assets/Models/cube.obj", device);

	// Create geometry for aiming reticle.  It's just 2 tris, so we should be fine to just do it here
	// This can probably be optimized even further, though (see particle system/ geometry shader slides)
	Vertex aimingVerts[] =
	{
		{ XMFLOAT3(+0.01f, +0.0f, +0.0f), XMFLOAT3(), XMFLOAT3() },
		{ XMFLOAT3(+0.0f, +0.02f, +0.0f), XMFLOAT3(), XMFLOAT3() },
		{ XMFLOAT3(-0.01f, +0.0f, +0.0f), XMFLOAT3(), XMFLOAT3() },
		{ XMFLOAT3(+0.0f, -0.02f, +0.0f), XMFLOAT3(), XMFLOAT3() },
	};

	UINT aimingIndices[] = { 2, 1, 0, 0, 3, 2 };

	// Load reticle mesh and entity
	reticleMesh = new Mesh(aimingVerts, 4, aimingIndices, 6, device);
	reticleEntity = new Entity(reticleMesh, reticleMat);

	// Load floor entity
	floorEntity = new Entity(cubeMesh, mat2); // Can probably optimize to make floor 2 tris instead of a complete cube
	floorEntity->SetScale(DirectX::XMFLOAT3(20.0f, 1.0f, 20.0f));
	floorEntity->SetPosition(DirectX::XMFLOAT3(0.0f, -3.5f, 0.0f));
	floorEntity->FinalizeMatrix();

	LoadTargets();
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
	debugCamera->SetProjectionMat(width, height);
	player->SetProjectionMat(width, height);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	XMFLOAT3 reticlePos;
	player->Update(deltaTime);
	debugCamera->Update(deltaTime);
	if (GetAsyncKeyState('B'))
	{
		player->SetActive(false);
		debugCamera->SetActive(true);
		debug = true;
	}
	else if (GetAsyncKeyState('G'))
	{
		player->SetActive(true);
		debugCamera->SetActive(false);
		debug = false;
	}

	switch (currentState)
	{
	case Game::START:
		// Advance to PLAY when Enter is pressed
		if (GetAsyncKeyState(VK_RETURN))
		{
			currentState = PLAY;
			InitForPlay();
		}

		break;
	case Game::PLAY:
		// Move the reticle to the correct position
		reticlePos = XMFLOAT3(player->GetPosition().x + player->GetDirection().x,
			player->GetPosition().y + player->GetDirection().y,
			player->GetPosition().z + player->GetDirection().z);
		reticleEntity->SetPosition(reticlePos);
		reticleEntity->FinalizeMatrix();
		if (GetAsyncKeyState(VK_SPACE) & 0x8000)
		{
			spacePressed = true;
		}
		else
		{
			if (spacePressed)
			{
				for (unsigned int i = 0; i < targets.size(); i++)
				{
					if (targets[i]->CheckShot(player->GetDirection(), player->GetPosition()))
					{
						i = targets.size() + 1;
					}
				}
				bool newTargets = true;
				for (unsigned int i = 0; i < targets.size(); i++)
				{
					if (targets[i]->GetActive())
					{
						newTargets = false;
					}
				}
				if (newTargets) needNewTarget = true;
			}
			spacePressed = false;
		}
		for (unsigned int i = 0; i < targets.size(); i++)
		{
			targets[i]->Update(deltaTime);
		}
		if (needNewTarget)
		{
			bool newTargets = true;
			for (unsigned int i = 0; i < targets.size(); i++)
			{
				if (targets[i]->GetActive()  || targets[i]->GetFall())
				{
					newTargets = false;
				}
			}
			if (newTargets)
			{
				for (unsigned int i = 0; i < targets.size(); i++)
				{
					delete targets[i];
				}
				targets.clear();
				LoadTargets();
			}
		}
		break;
	case Game::PAUSE:
		break;
	case Game::END:
		break;
	default:
		break;
	}

	
	// Quit if the escape key is pressed -- do this regardless of currentState
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(bloomRTV, color); // Clear post process target too
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	// Set buffers in the input assembler
	//  - Do this ONCE PER OBJECT you're drawing, since each object might
	//    have different geometry.
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	ID3D11Buffer* vertexBuffer = 0;
	ID3D11Buffer* indexBuffer = 0;
	ID3D11Buffer* skyVB = 0;
	ID3D11Buffer* skyIB = 0;
	ID3D11Buffer* nothing = 0;

	switch (currentState)
	{
	case Game::START:
		// Draw text
		spriteBatch->Begin();
		spriteFont->DrawString(spriteBatch, L"Press Enter to start", XMFLOAT2(0.0f, 0.0f), Colors::Black);
		spriteBatch->End();

		break;
	case Game::PLAY:
		// Draw shadows first
		RenderShadowMap();

		// Draw aiming reticle
		vertexBuffer = reticleMesh->GetVertexBuffer();
		indexBuffer = reticleMesh->GetIndexBuffer();
		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		vertexShader->SetMatrix4x4("shadowView", shadowViewMatrix);
		vertexShader->SetMatrix4x4("shadowProjection", shadowProjectionMatrix);

		pixelShader->SetShaderResourceView("ShadowMap", shadowSRV);
		pixelShader->SetSamplerState("ShadowSampler", shadowSampler);

		reticleEntity->PrepareMaterial(player->GetViewMat(), player->GetProjectionMat());

		context->DrawIndexed(reticleMesh->GetIndexCount(), 0, 0);

		// Draw floor
		vertexBuffer = floorEntity->GetMesh()->GetVertexBuffer();
		indexBuffer = floorEntity->GetMesh()->GetIndexBuffer();
		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		if (debug)
		{
			floorEntity->PrepareMaterial(debugCamera->GetViewMat(), debugCamera->GetProjectionMat());
		}
		else
		{
			floorEntity->PrepareMaterial(player->GetViewMat(), player->GetProjectionMat());
		}
		context->DrawIndexed(floorEntity->GetMesh()->GetIndexCount(), 0, 0);

		// Draw targets.  The proper shaders are already being set during this process
		for (unsigned int i = 0; i < targets.size(); i++) {
			vector<Entity*> targetEntitys = targets[i]->GetTarget();
			for (unsigned int j = 0; j < targetEntitys.size(); j++) {
				// Draw the entity
				vertexBuffer = targetEntitys[j]->GetMesh()->GetVertexBuffer();
				indexBuffer = targetEntitys[j]->GetMesh()->GetIndexBuffer();
				context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
				context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

				// Prepare the shader for the material being used
				if (debug)
				{
					targetEntitys[j]->PrepareMaterial(debugCamera->GetViewMat(), debugCamera->GetProjectionMat());
					context->DrawIndexed(
						targetEntitys[j]->GetMesh()->GetIndexCount(),	//The Number of indices to use (gotten from mesh data)
						0,												// Offset to the first index to look at
						0);												// Offset to add to each index when looking at vertices
				}
				else
				{
					targetEntitys[j]->PrepareMaterial(player->GetViewMat(), player->GetProjectionMat());
					context->DrawIndexed(
						targetEntitys[j]->GetMesh()->GetIndexCount(),	//The Number of indices to use (gotten from mesh data)
						0,												// Offset to the first index to look at
						0);												// Offset to add to each index when looking at vertices
				}
				
			}
		}
		skyVB = cubeMesh->GetVertexBuffer();
		skyIB = cubeMesh->GetIndexBuffer();
		context->IASetVertexBuffers(0, 1, &skyVB, &stride, &offset);
		context->IASetIndexBuffer(skyIB, DXGI_FORMAT_R32_UINT, 0);
		if (debug)
		{
			skyVertexShader->SetMatrix4x4("view", debugCamera->GetViewMat());
			skyVertexShader->SetMatrix4x4("projection", debugCamera->GetProjectionMat());
		}
		else
		{
			skyVertexShader->SetMatrix4x4("view", player->GetViewMat());
			skyVertexShader->SetMatrix4x4("projection", player->GetProjectionMat());
		}
		
		skyVertexShader->CopyAllBufferData();
		skyVertexShader->SetShader();

		skyPixelShader->SetShaderResourceView("Sky", skySRV);
		skyPixelShader->CopyAllBufferData();
		skyPixelShader->SetShader();

		context->RSSetState(skyRastState);
		context->OMSetDepthStencilState(skyDepthState, 0);

		context->DrawIndexed(cubeMesh->GetIndexCount(), 0, 0);

		context->RSSetState(0);
		context->OMSetDepthStencilState(0, 0);
		pixelShader->SetShaderResourceView("ShadowMap", 0);

		// Reset the states!
		context->RSSetState(0);
		context->OMSetDepthStencilState(0, 0);

		// Make sure output goes to my post process target
		context->OMSetRenderTargets(1, &bloomRTV, depthStencilView);

		// Now ---- post process
		context->OMSetRenderTargets(1, &backBufferRTV, 0);

		// Set up post process shaders
		bloomVS->SetShader();

		bloomPS->SetShader();
		bloomPS->SetShaderResourceView("Pixels", bloomSRV);
		bloomPS->SetSamplerState("Sampler", sampler);
		bloomPS->SetInt("blurAmount", 9);
		bloomPS->SetFloat("pixelWidth", 1.0f / width);
		bloomPS->SetFloat("pixelHeight", 1.0f / height);
		bloomPS->CopyAllBufferData();

		// Now actually draw
		context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
		context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

		context->Draw(3, 0);

		bloomPS->SetShaderResourceView("Pixels", 0);

		break;
	case Game::PAUSE:
		break;
	case Game::END:
		break;
	default:
		break;
	}

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);
}

void Game::RenderShadowMap() 
{
	// Set up render targets
	context->OMSetRenderTargets(0, 0, shadowDSV);
	context->ClearDepthStencilView(shadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	context->RSSetState(shadowRasterizer);

	// Create a viewport matching the render target size
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)shadowMapSize;
	viewport.Height = (float)shadowMapSize;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	// Set Shadow Vertex Shader and turn off Pixel Shader
	shadowVS->SetShader();
	shadowVS->SetMatrix4x4("view", shadowViewMatrix);
	shadowVS->SetMatrix4x4("projection", shadowProjectionMatrix);
	context->PSSetShader(0, 0, 0);

	// Loop through and draw Targets (the only things that will cast shadows)
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	for (unsigned int i = 0; i < targets.size(); ++i) 
	{
		vector<Entity*> targetEntities = targets[i]->GetTarget();
		for (unsigned int j = 0; j < targetEntities.size(); ++j)
		{
			// Get buffer data from this entity's mesh
			Entity* currentEntity = targetEntities[j];
			ID3D11Buffer* vertexBuffer = currentEntity->GetMesh()->GetVertexBuffer();
			ID3D11Buffer* indexBuffer = currentEntity->GetMesh()->GetIndexBuffer();

			// Set buffers in assembler
			context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
			context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
			shadowVS->SetMatrix4x4("world", currentEntity->GetWorldMatrix());
			shadowVS->CopyAllBufferData();

			// Draw the entity to the shadow map
			context->DrawIndexed(currentEntity->GetMesh()->GetIndexCount(), 0, 0);
		}
	}

	// Change everything back
	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	context->RSSetViewports(1, &viewport);
	context->RSSetState(0);
}

void Game::LoadTargets() {
	for (int i = 0; i < 3; i++) {
		targets.push_back(new Target(DirectX::XMFLOAT3(+1.1f*(i - 1), +1.0f, +0.0f), sphereMesh, mat1, mat2));
	}
	for (unsigned int i = 0; i < targets.size(); i++) {
		vector<Entity*> targetEntitys = targets[i]->GetTarget();
		for (unsigned int j = 0; j < targetEntitys.size(); j++) {
			targetEntitys[j]->FinalizeMatrix();
		}
	}
}


#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
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
	// Add any custom code here...
	if (buttonState & 0x0001)
	{
		if (currentState == GAME_STATES::PLAY)
		{
			if (debug)
			{
				debugCamera->RotateCamera(x - prevMousePos.x, y - prevMousePos.y);
			}
			else
			{
				player->RotatePlayer(x - prevMousePos.x, y - prevMousePos.y);
				XMFLOAT3 reticlePos = XMFLOAT3(player->GetPosition().x + player->GetDirection().x,
					player->GetPosition().y + player->GetDirection().y,
					player->GetPosition().z + player->GetDirection().z);
				reticleEntity->SetPosition(reticlePos);
				reticleEntity->FinalizeMatrix();
			}
		}
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