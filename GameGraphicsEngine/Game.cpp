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
	rockSRV->Release();
	woodSRV->Release();
	//normalMapBarkSRV->Release();
	skySRV->Release();


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

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	LoadSpriteFont();
	CreateMatrices();
	//CreateBasicGeometry(); // Don't really need to be making the geometry until we're advancing to the Play state (at least for now)


	// Set Up Light		** We may be able to leave this out of Init(), and only set up the lights (and LoadShaders) in InitForPlay()
	dirLight.AmbietColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	dirLight.DiffuseColor = XMFLOAT4(1, 0, 0, 1);
	dirLight.Direction = XMFLOAT3(0, -1, 0);

	dirLight2.AmbietColor = XMFLOAT4(0.1, 0.1, 0.1, 1.0f);
	dirLight2.DiffuseColor = XMFLOAT4(1, 1, 1, 1);
	dirLight2.Direction = XMFLOAT3(0, 0, 1);

	pixelShader->SetData("light", &dirLight, sizeof(DirectionalLight));
	pixelShader->SetData("light2", &dirLight2, sizeof(DirectionalLight));

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// Initializes game for transition from Start to Play state
void Game::InitForPlay() {
	ResetRenderStates();
	LoadShaders();
	CreateBasicGeometry();

	// Also need to reset the lighting, since leaving it out here causes the geometry to not show up
	dirLight.AmbietColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	dirLight.DiffuseColor = XMFLOAT4(1, 0, 0, 1);
	dirLight.Direction = XMFLOAT3(0, -1, 0);

	dirLight2.AmbietColor = XMFLOAT4(0.1, 0.1, 0.1, 1.0f);
	dirLight2.DiffuseColor = XMFLOAT4(1, 1, 1, 1);
	dirLight2.Direction = XMFLOAT3(0, 0, 1);

	pixelShader->SetData("light", &dirLight, sizeof(DirectionalLight));
	pixelShader->SetData("light2", &dirLight2, sizeof(DirectionalLight));
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

	// Create geometry for aiming reticle.  It's just 2 tris, so we should be fine to just do it here on the stack
	// This can probably be optimized even further, though (see particle system/ geometry shader slides)
	Vertex aimingVerts[] =
	{
		{ XMFLOAT3(+0.01f, +0.0f, +0.0f), XMFLOAT3(), XMFLOAT3() },
		{ XMFLOAT3(+0.0f, +0.01f, +0.0f), XMFLOAT3(), XMFLOAT3() },
		{ XMFLOAT3(-0.01f, +0.0f, +0.0f), XMFLOAT3(), XMFLOAT3() },
		{ XMFLOAT3(+0.0f, -0.01f, +0.0f), XMFLOAT3(), XMFLOAT3() },
	};

	UINT aimingIndices[] = { 2, 1, 0, 0, 3, 2 };

	// Load reticle mesh and entity
	reticleMesh = new Mesh(aimingVerts, 4, aimingIndices, 6, device);
	reticleEntity = new Entity(reticleMesh, reticleMat);

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

	switch (currentState)
	{
	case Game::START:
		// Draw text
		spriteBatch->Begin();
		spriteFont->DrawString(spriteBatch, L"Press Enter to start", XMFLOAT2(0.0f, 0.0f), Colors::Black);
		spriteBatch->End();

		break;
	case Game::PLAY:
		// Draw aiming reticle
		vertexBuffer = reticleMesh->GetVertexBuffer();
		indexBuffer = reticleMesh->GetIndexBuffer();
		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		reticleEntity->PrepareMaterial(player->GetViewMat(), player->GetProjectionMat());

		context->DrawIndexed(reticleMesh->GetIndexCount(), 0, 0);

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
		context->IAGetVertexBuffers(0, 1, &skyVB, &stride, &offset);
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