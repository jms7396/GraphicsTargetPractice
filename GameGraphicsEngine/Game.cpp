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

	// Delete the Mesh's to clear memory
	if (sphereMesh) { delete(sphereMesh); }
	if (helixMesh) { delete(helixMesh); }
	if (cubeMesh) { delete(cubeMesh); }
	//if (entityOne) { delete(entityOne); }
	//if (entityTwo){ delete(entityTwo); }
	//if (entityThree) { delete(entityThree); }

	// Delete the Camera
	//if (gameCamera) { delete (gameCamera); }

	// Delete Player
	if (player) { delete (player); }

	// Delete Material
	if (mat1) { delete (mat1); }
	if (mat2) { delete (mat2); }

	// Delete SpriteFont stuff
	if (spriteBatch) { delete spriteBatch; }
	if (spriteFont) { delete spriteFont; }

	// Release textures and sampler
	sampler->Release();
	rockSRV->Release();
	woodSRV->Release();
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
	if(!pixelShader->LoadShaderFile(L"Debug/PixelShader.cso"))	
		pixelShader->LoadShaderFile(L"PixelShader.cso");

	pixelShader->SetSamplerState("Sampler", sampler);
	pixelShader->SetShaderResourceView("Rocks", rockSRV);
	pixelShader->SetShaderResourceView("Wood", woodSRV);

	// Load our shaders into our materials
	mat1 = new Material(pixelShader, vertexShader, rockSRV, sampler);
	mat2 = new Material(pixelShader, vertexShader, woodSRV, sampler);

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
	//gameCamera = new Camera();
	//gameCamera->SetProjectionMat(width, height);

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

	LoadTargets();

	/*
	// Creating Entities using Meshes
	entityOne = new Entity(sphereMesh, mat2);
	entityTwo = new Entity(cubeMesh, mat1);
	entityThree = new Entity(helixMesh, mat1);

	entityOne->SetPosition(DirectX::XMFLOAT3(+1.5f, +0.0f, +0.0f));
	entityOne->SetScale(DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f));
	entityTwo->SetPosition(DirectX::XMFLOAT3(-1.5f, +0.0f, +0.0f));
	entityTwo->SetScale(DirectX::XMFLOAT3(+1.5f, +1.5f, +1.0f));
	entityThree->SetPosition(DirectX::XMFLOAT3(+0.0f, -1.5f, +0.0f));
	entityThree->SetRotation(DirectX::XMFLOAT3(+0.0f, +0.0f, +1.0f));
	*/
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
	//gameCamera->SetProjectionMat(width, height);
	player->SetProjectionMat(width, height);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
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
		// Update entities only during PLAY
		/*
		entityOne->SetRotation(XMFLOAT3(entityOne->GetRotation().x, entityOne->GetRotation().y + 0.0001, entityOne->GetRotation().z));
		entityOne->FinalizeMatrix();

		entityTwo->SetPosition(XMFLOAT3(entityTwo->GetPosition().x, sin(totalTime), entityTwo->GetPosition().z));
		entityTwo->FinalizeMatrix();

		entityThree->SetScale(XMFLOAT3(sin(totalTime) + 1, sin(totalTime) + 1, sin(totalTime) + 1));
		entityThree->FinalizeMatrix();
		*/

		break;
	case Game::PAUSE:
		break;
	case Game::END:
		break;
	default:
		break;
	}

	//gameCamera->Update(deltaTime);
	player->Update(deltaTime);
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
	const float color[4] = {0.4f, 0.6f, 0.75f, 0.0f};

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

	switch (currentState)
	{
	case Game::START:
		// Draw text
		spriteBatch->Begin();
		spriteFont->DrawString(spriteBatch, L"Press Enter to start", XMFLOAT2(0.0f, 0.0f), Colors::Black);
		spriteBatch->End();

		break;
	case Game::PLAY:
		for (int i = 0; i < targets.size(); i++) {
			vector<Entity*> targetEntitys = targets[i]->GetTarget();
			for (int j = 0; j < targetEntitys.size(); j++) {
				// Draw the entity
				vertexBuffer = targetEntitys[j]->GetMesh()->GetVertexBuffer();
				indexBuffer = targetEntitys[j]->GetMesh()->GetIndexBuffer();
				context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
				context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

				// Prepare the shader for the material being used
				targetEntitys[j]->PrepareMaterial(player->GetViewMat(), player->GetProjectionMat());
				context->DrawIndexed(
					targetEntitys[j]->GetMesh()->GetIndexCount(),	//The Number of indices to use (gotten from mesh data)
					0,												// Offset to the first index to look at
					0);												// Offset to add to each index when looking at vertices
			}
		}
		/*
		// Drawing the Sphere
		vertexBuffer = entityOne->GetMesh()->GetVertexBuffer();
		indexBuffer = entityOne->GetMesh()->GetIndexBuffer();
		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Prepare the shaders for the material being used
		entityOne->PrepareMaterial(player->GetViewMat(), player->GetProjectionMat());

		context->DrawIndexed(
			entityOne->GetMesh()->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
			0,     // Offset to the first index we want to use
			0);    // Offset to add to each index when looking up vertices

				   // Drawing a Cube
		vertexBuffer = entityTwo->GetMesh()->GetVertexBuffer();
		indexBuffer = entityTwo->GetMesh()->GetIndexBuffer();
		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Prepare the shaders for the material being used
		entityTwo->PrepareMaterial(player->GetViewMat(), player->GetProjectionMat());

		context->DrawIndexed(
			entityTwo->GetMesh()->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
			0,     // Offset to the first index we want to use
			0);    // Offset to add to each index when looking up vertices

				   // Drawing a helix
		vertexBuffer = entityThree->GetMesh()->GetVertexBuffer();
		indexBuffer = entityThree->GetMesh()->GetIndexBuffer();
		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		// Prepare the shaders for the material being used
		entityThree->PrepareMaterial(player->GetViewMat(), player->GetProjectionMat());
		

		context->DrawIndexed(
			entityThree->GetMesh()->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
			0,     // Offset to the first index we want to use
			0);    // Offset to add to each index when looking up vertices
		*/
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

void Game::LoadTargets(){
	for (int i = 0; i < 3; i++) {
		targets.push_back(new Target(DirectX::XMFLOAT3(+1.0f*(i - 1), +0.0f, 0.0f), sphereMesh, mat1, mat2));
	}
	for (int i = 0; i < targets.size(); i++) {
		vector<Entity*> targetEntitys = targets[i]->GetTarget();
		for (int j = 0; j < targetEntitys.size(); j++) {
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
		// The Left mouse button is clicked
		if (x != prevMousePos.x || y != prevMousePos.y) {
			float deltaX = x - prevMousePos.x;
			float deltaY = y - prevMousePos.y;
			//gameCamera->RotateCamera(deltaX, deltaY);
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