// This code assumes files are in "ImGui" subfolder!
// Adjust as necessary for your own folder structure and project setup
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Mesh.h"
#include "BufferStructs.h"

#include <DirectXMath.h>


// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>
#include <memory>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());
	// Pick a style (uncomment one of these 3)
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateGeometry();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		Graphics::Context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);



		unsigned int size = sizeof(VertexShaderData);
		size = (size + 15) / 16 * 16; // Integer division is necessary here!

		// Describe the constant buffer and create it
		D3D11_BUFFER_DESC cbDesc = {};
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.ByteWidth = size;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;
		Graphics::Device->CreateBuffer(&cbDesc, 0, vsConstantBuffer.GetAddressOf());

		Graphics::Context->VSSetConstantBuffers(
			0,		// Which slot (register) to bind the buffer to?
			1,		// How many are we activating?  Can set more than one at a time, if we need
			vsConstantBuffer.GetAddressOf());
	}
}



// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		Graphics::Device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		Graphics::Device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 black = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 grey = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);

	Vertex verticesOne[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	};

	unsigned int indicesOne[] = { 0, 1, 2 };


	//Shape two
	Vertex verticesTwo[] =
	{
		{ XMFLOAT3(-0.80f, +0.30f, 0.0f), blue },	// Top left
		{ XMFLOAT3(-0.80f, +0.10f, 0.0f), black },	// Bottom left
		{ XMFLOAT3(-0.40f, +0.20f, 0.0f), blue },	// Bottom right
		{ XMFLOAT3(-0.40f, +0.30f, 0.0f), black }		// Top right
	};

	unsigned int indicesTwo[] =
	{
		0, 3, 2,	// Ensure clockwise winding order
		0, 2, 1		// for both triangles
	};

	// Shape three
	Vertex verticesThree[] =
	{
		{ XMFLOAT3(+0.50f, +0.50f, 0.0f), blue },
		{ XMFLOAT3(+0.60f, +0.60f, 0.0f), blue },
		{ XMFLOAT3(+0.60f, +0.40f, 0.0f), red },
		{ XMFLOAT3(+0.70f, +0.70f, 0.0f), red },
		{ XMFLOAT3(+0.80f, +0.60f, 0.0f), red },
		{ XMFLOAT3(+0.90f, +0.50f, 0.0f), red },
		{ XMFLOAT3(+0.80f, +0.40f, 0.0f), blue },
		{ XMFLOAT3(+0.70f, +0.30f, 0.0f), grey },
	};
	unsigned int indicesThree[] =
	{
		0, 1, 2,	// Ensure clockwise winding order
		1, 3, 4,
		4, 5, 6,
		6, 7, 2,
		2, 4, 6
	};


	std::shared_ptr<Mesh> triangle = std::make_shared<Mesh>(ARRAYSIZE(indicesOne), ARRAYSIZE(verticesOne), verticesOne, indicesOne, "Triangle");
	std::shared_ptr<Mesh> shapeTwo = std::make_shared<Mesh>(ARRAYSIZE(indicesTwo), ARRAYSIZE(verticesTwo), verticesTwo, indicesTwo, "Quad");
	std::shared_ptr<Mesh> shapeThree = std::make_shared<Mesh>(ARRAYSIZE(indicesThree), ARRAYSIZE(verticesThree), verticesThree, indicesThree, "Shape");

	meshes.push_back(triangle);
	meshes.push_back(shapeTwo);
	meshes.push_back(shapeThree);

	std::shared_ptr<GameEntity> g1 = std::make_shared<GameEntity>(triangle);
	std::shared_ptr<GameEntity> g2 = std::make_shared<GameEntity>(triangle);
	std::shared_ptr<GameEntity> g3 = std::make_shared<GameEntity>(shapeTwo);
	std::shared_ptr<GameEntity> g4 = std::make_shared<GameEntity>(shapeThree);
	std::shared_ptr<GameEntity> g5 = std::make_shared<GameEntity>(shapeThree);


	// Adjust transforms
	g1->GetTransform()->Rotate(0, 0, 0.1f);
	g2->GetTransform()->MoveAbsolute(-1.2f, -0.3f, 0.0f);
	g4->GetTransform()->MoveAbsolute(-0.5f, 0.1f, 0.0f);
	g5->GetTransform()->MoveAbsolute(0.1f, -1.0f, 0.0f);

	entities.push_back(g1);
	entities.push_back(g2);
	entities.push_back(g3);
	entities.push_back(g4);
	entities.push_back(g5);
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{

	// Put this all in a helper method that is called from Game::Update()
// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();
	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
	BuildUI();

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();


	//I,J,K,L to move box around
	if (Input::KeyPress(75))
		entities[2]->GetTransform()->MoveRelative(0,-0.1,0);
	if (Input::KeyPress(73))
		entities[2]->GetTransform()->MoveRelative(0, 0.1, 0);
	if (Input::KeyPress(74))
		entities[2]->GetTransform()->MoveRelative(-0.1, 0, 0);
	if (Input::KeyPress(76))
		entities[2]->GetTransform()->MoveRelative(0.1, 0, 0);

	float scale = (float)sin(totalTime * 5) * 0.5f + 1.0f;
	entities[0]->GetTransform()->SetScale(scale, scale, scale);
	entities[0]->GetTransform()->Rotate(0, 0, deltaTime * 1.0f);
	entities[1]->GetTransform()->SetPosition((float)sin(totalTime), 0, 0);
}

float color[4] = { 0.4f, 0.6f, 0.75f, 1.0f };
int number = 0;
float colorTint[4] = { 1.0f, 0.5f, 0.5f, 1.0f };
float offSet[3] = { 0.25f, 0.0f, 0.0f };

void Game::BuildUI() {
	if (windowOpen)
	{
		ImGui::ShowDemoWindow();
	}

	ImGui::Begin("My First Window"); {

		//ImGui::Text("test");
		ImGui::Text("Framerate: %f fps", ImGui::GetIO().Framerate);
		ImGui::Text("Window Resolution: %dx%d", Window::Width(), Window::Height());
		ImGui::Spacing();

		if (ImGui::Button(windowOpen ? "Hide ImGui Demo Window" : "Show ImGui Demo Window"))
			windowOpen = !windowOpen;

		ImGui::Spacing();

		if (ImGui::TreeNode("Expand Background Color Picker")) {
			float red = color[0];
			float blue = color[1];
			float green = color[2];
			float alpha = color[3];

			ImGui::SliderFloat("Red", &red, 0, 1);
			ImGui::SliderFloat("Blue", &blue, 0, 1);
			ImGui::SliderFloat("Green", &green, 0, 1);
			ImGui::SliderFloat("Alpha", &alpha, 0, 1);

			XMFLOAT4 newColor(red, blue, green, alpha);
			ImGui::ColorEdit4("RGBA color editor", &newColor.x);

			color[0] = newColor.x;
			color[1] = newColor.y;
			color[2] = newColor.z;
			color[3] = newColor.w;

			ImGui::TreePop();
		}

		ImGui::Spacing();

		if (ImGui::TreeNode("Number Counter")) {
			// Create a button and test for a click
			if (ImGui::Button("Press to increment"))
			{
				if (number >= 0 && number < 10) {
					number++; // Adds to number when clicked
				}
			}
			if (ImGui::Button("Press to decrement"))
			{
				if (number > 0) {
					number--; // Subtracts to number when clicked
				}
			}
			if (ImGui::TreeNode("Table")) {
				if (ImGui::BeginTable("test", 4))
				{
					for (int row = 0; row < 4; row++)
					{
						ImGui::TableNextRow();
						for (int column = 0; column < 4; column++)
						{
							ImGui::TableSetColumnIndex(column);
							ImGui::Text("Row %d Column %d", row, column);
							ImGui::Text("Counter Number: %d", number);
						}
					}
					ImGui::EndTable();
				}
				ImGui::Spacing();
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}

		ImGui::Spacing();

		if (ImGui::TreeNode("Meshes")) {
			for (int i =0; i < meshes.size(); i++)
			{
				int triangles = ((meshes[i]->GetIndexCount()) / 3);
				ImGui::PushID(entities[i].get());
				if (ImGui::TreeNode("Mesh Node", "Mesh: %s", meshes[i]->GetName())) {
					ImGui::Text("Vertice: %d", meshes[i]->GetVertexCount());
					ImGui::Text("Indices: %d", meshes[i]->GetIndexCount());
					ImGui::Text("Triangles: %d", triangles);
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}

		ImGui::Spacing();

		if (ImGui::TreeNode("VertexShaderExternal")) 
		{

			XMFLOAT4 newColor(colorTint);
			ImGui::ColorEdit4("RGBA color editor", &newColor.x);
			colorTint[0] = newColor.x;
			colorTint[1] = newColor.y;
			colorTint[2] = newColor.z;
			colorTint[3] = newColor.w;

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Transform"))
		{
			for (int i = 0; i < entities.size(); i++) {
				ImGui::PushID(entities[i].get());

				if (ImGui::TreeNode("Entity Node", "Entity %d", i))
				{
					XMFLOAT3 position = entities[i]->GetTransform()->GetPosition();
					XMFLOAT3 rotation = entities[i]->GetTransform()->GetPitchYawRoll();
					XMFLOAT3 scale = entities[i]->GetTransform()->GetScale();

					if (ImGui::DragFloat3("Position", &position.x, 0.01f)) 
						entities[i]->GetTransform()->SetPosition(position);
					if (ImGui::DragFloat3("Rotation", &rotation.x, 0.01f)) 
						entities[i]->GetTransform()->SetRotation(rotation);
					if (ImGui::DragFloat3("Scale", &scale.x, 0.01f)) 
						entities[i]->GetTransform()->SetScale(scale);
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			
			ImGui::TreePop();
		}
	}
	ImGui::End();

}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------

void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
		// - These things should happen ONCE PER FRAME
		// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(), color);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// DRAW geometry
	// Loop through the game entities and draw each one
	// - Note: A constant buffer has already been bound to
	//   the vertex shader stage of the pipeline (see Init above)
	for (auto& e : entities)
	{
		e->gDraw(vsConstantBuffer);
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Draw the UI after everything else
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}



