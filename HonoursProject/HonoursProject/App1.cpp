
#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Create Mesh object and shader object
	mesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	model = new Model(renderer->getDevice(), renderer->getDeviceContext(), "res/Teapot.obj");
	shoeA = new Model(renderer->getDevice(), renderer->getDeviceContext(), "res/3DShoe.obj");
	shoeB = new Model(renderer->getDevice(), renderer->getDeviceContext(), "res/3DShoeLeft.obj");

	textureMgr->loadTexture(L"black", L"res/black.png");
	textureMgr->loadTexture(L"surface", L"res/Sand512Tile.png");
	textureMgr->loadTexture(L"grey", L"res/Grey.png");

	hasReset = false;
	volumePong = false;
	inIterativeDisplacement = true;
	inDemo = false;

	threadCountHeight = 32;
	threadCountWidth = 32;
	sWidth = threadCountWidth * threadCountHeight;
	sHeight = threadCountHeight * threadCountWidth;

	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	manipulationShader = new ManipulationShader(renderer->getDevice(), hwnd);
	computeShader = new ComputeShader(renderer->getDevice(), hwnd, sWidth, sHeight);
	writeComputeShader = new WriteComputeShader(renderer->getDevice(), hwnd, sWidth, sHeight);
	displacementShader = new DisplacementShader(renderer->getDevice(), hwnd, sWidth, sHeight);
	displacementMethodBShader = new DisplacementMethodBShader(renderer->getDevice(), hwnd, sWidth, sHeight);
	penetrationShader = new PenetrationShader(renderer->getDevice(), hwnd, sWidth, sHeight);
	jumpFloodingShader = new JumpFloodingShader(renderer->getDevice(), hwnd, sWidth, sHeight);
	pongJumpFLoodShader = new PongJumpFloodShader(renderer->getDevice(), hwnd, sWidth, sHeight);
	jumpFloodInitShader = new JumpFloodInitializer(renderer->getDevice(), hwnd, sWidth, sHeight);
	volumePreservationShader = new VolumePreservationShader(renderer->getDevice(), hwnd, sWidth, sHeight);
	volumePreservationPongShader = new VolumePreservationPongShader(renderer->getDevice(), hwnd, sWidth, sHeight);
	normalMapShader = new NormalMapShader(renderer->getDevice(), hwnd, sWidth, sHeight);
	vpTestShader = new VolumePreservationTestShader(renderer->getDevice(), hwnd, sWidth, sHeight);

	orthoMeshLeft = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth/4, screenHeight/4, -screenWidth * .35f, -screenHeight * .35f);

	gpuProfiler = new GPUProfiler;
	gpuProfiler->Init(renderer->getDeviceContext(), renderer->getDevice());

	footprintPositionA = XMFLOAT4(20, 0, -10, 0);
	footprintPositionB = XMFLOAT4(10, 12.1, -30, 0);
	heightA = 0;
	heightB = 12.1;

	deformationMap = new DeformationMap(renderer->getDevice(), sWidth, sHeight);
	
	lightX = 39.f;
	lightY = -48.f;
	lightZ = 7.f;
	lightDirX = 0.f;
	lightDirY = 1.f;
	scale = 4.f;
	newLightDirX = 0.5f;
	newLightDirY = -.9f;
	newLightDirZ = 0.5f;
	teapotX = 35.f;
	teapotY = 4.f;
	teapotZ = -60.f;
	heightTreshold = 0.695f;

	int sceneWidth = 100;
	int sceneHeight = 100;

	light = new Light;
	light->setDiffuseColour(.0f, .0f, .0f, 1.0f);
	light->setDirection(lightDirX, 0, 0);
	light->setPosition(lightX, lightY, lightZ);
	light->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);

	directionalLight = new Light;
	directionalLight->setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	directionalLight->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLight->setDirection(newLightDirX, newLightDirY, newLightDirZ);

	depthLightX = 50.f;
	depthLightY = -100.f;
	depthLightZ = 50.f;
	depthDirX = -90;
	depthDirY = 0;
	depthDirZ = 0;

	depthCamera = new DepthCamera(input, sWidth, sHeight, wnd);
	depthCamera->setPos(depthLightX, depthLightY, depthLightZ);
	depthCamera->setRot(depthDirX, depthDirY, depthDirZ);
	depthCamera->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();
	gpuProfiler->Stop();
}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	light->setDirection(lightDirX, lightDirY, 0);
	light->setPosition(lightX, lightY, lightZ);

	depthCamera->setPos(depthLightX, depthLightY, depthLightZ);
	depthCamera->setRot(depthDirX, depthDirY, depthDirZ);
	directionalLight->setDirection(newLightDirX, newLightDirY, newLightDirZ);

	if (inDemo) {
		FootprintDemo();
	}

	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{
	gpuProfiler->BeginFrame();

	// Perform depth pass
	depthPass();
	gpuProfiler->CheckTimestamp(TIMESTAMP_DEPTH);
	ComputePass();
	gpuProfiler->CheckTimestamp(TIMESTAMP_COMPUTE_READ);
	PenetrationPass();
	gpuProfiler->CheckTimestamp(TIMESTAMP_COMPUTE_PENETRATION);
	JumpFloodingPass();
	gpuProfiler->CheckTimestamp(TIMESTAMP_COMPUTE_JUMPFLOOD);
	DisplacementPass();
	gpuProfiler->CheckTimestamp(TIMESTAMP_COMPUTE_DISPLACE);
	if (inIterativeDisplacement) {
		VolumePreservationPass();
	}
	gpuProfiler->CheckTimestamp(TIMESTAMP_COMPUTE_VOLUMEPRESERVE);
	WriteComputePass();
	gpuProfiler->CheckTimestamp(TIMESTAMP_COMPUTE_WRITE);
	// Render scene
	finalPass();
	gpuProfiler->CheckTimestamp(TIMESTAMP_SCENE);

	gpuProfiler->DataWaitandUpdate();
	
	VolumePreservationTest();
	
	renderer->endScene();
	gpuProfiler->EndFrame();

	
	return true;
}

void App1::depthPass()
{
	// Set the render target to be the render to texture.
	deformationMap->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	depthCamera->generateViewMatrix();
	XMMATRIX lightViewMatrix = depthCamera->getViewMatrix();	
	XMMATRIX lightProjectionMatrix = depthCamera->getOrthoMatrix();

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0, 0, 0);

	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, writeComputeShader->getSRV(),
		textureMgr->getTexture(L"black"), textureMgr->getTexture(L"black"), scale);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render models
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(teapotX + 25, teapotY, teapotZ);
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, textureMgr->getTexture(L"black"),
		textureMgr->getTexture(L"black"), textureMgr->getTexture(L"black"), 0);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	worldMatrix = XMMatrixTranslation(footprintPositionA.x, footprintPositionA.y, footprintPositionA.z);
	shoeA->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, textureMgr->getTexture(L"black"),
		textureMgr->getTexture(L"black"), textureMgr->getTexture(L"black"), 0);
	depthShader->render(renderer->getDeviceContext(), shoeB->getIndexCount());

	worldMatrix = XMMatrixTranslation(footprintPositionB.x, footprintPositionB.y, footprintPositionB.z);
	shoeB->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, textureMgr->getTexture(L"black"),
		textureMgr->getTexture(L"black"), textureMgr->getTexture(L"black"), 0);
	depthShader->render(renderer->getDeviceContext(), shoeB->getIndexCount());


	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::ComputePass() {

	// Check if the compute pass has already been called
	// if it hasnt, pass in a grey image for the height map, else the result of the compute shader so process can start agin with result of previous
	if (!hasReset) {
		computeShader->setShaderParameters(renderer->getDeviceContext(), textureMgr->getTexture(L"grey"));
		computeShader->compute(renderer->getDeviceContext(), threadCountWidth, threadCountHeight, 1);
		computeShader->unbind(renderer->getDeviceContext());
		hasReset = true;
	}

	else if (hasReset) {
		computeShader->setShaderParameters(renderer->getDeviceContext(), writeComputeShader->getSRV());
		computeShader->compute(renderer->getDeviceContext(), threadCountWidth, threadCountHeight, 1);
		computeShader->unbind(renderer->getDeviceContext());
	}
}

void App1::PenetrationPass()
{
	penetrationShader->setShaderParameters(renderer->getDeviceContext(), writeComputeShader->getSRV(), deformationMap->getDepthMapSRV());
	penetrationShader->compute(renderer->getDeviceContext(), threadCountWidth, threadCountHeight, 1);
	penetrationShader->unbind(renderer->getDeviceContext());
	
}

void App1::JumpFloodingPass()
{
	bool ping = true;
	bool hasStarted = false;

	jumpFloodInitShader->setShaderParameters(renderer->getDeviceContext(), penetrationShader->getSRV());
	jumpFloodInitShader->compute(renderer->getDeviceContext(), threadCountWidth, threadCountHeight, 1);
	jumpFloodInitShader->unbind(renderer->getDeviceContext());

	int totalCount = threadCountWidth * threadCountHeight;
	for (int i = totalCount; i >= 1; i /= 2) {

		if (ping) {
			if (!hasStarted) {
				// write compute pass is given the result of the standard compute pass and just outputs the same image
				jumpFloodingShader->setShaderParameters(renderer->getDeviceContext(), jumpFloodInitShader->getSRV(), i);
				jumpFloodingShader->compute(renderer->getDeviceContext(), threadCountWidth, threadCountHeight, 1);
				jumpFloodingShader->unbind(renderer->getDeviceContext());
				hasStarted = true;
				ping = false;
			}
			else {
				// write compute pass is given the result of the standard compute pass and just outputs the same image
				jumpFloodingShader->setShaderParameters(renderer->getDeviceContext(), pongJumpFLoodShader->getSRV(), i);
				jumpFloodingShader->compute(renderer->getDeviceContext(), threadCountWidth, threadCountHeight, 1);
				jumpFloodingShader->unbind(renderer->getDeviceContext());
				ping = false;
			}
		}
		else {
			// write compute pass is given the result of the standard compute pass and just outputs the same image
			pongJumpFLoodShader->setShaderParameters(renderer->getDeviceContext(), jumpFloodingShader->getSRV(), i);
			pongJumpFLoodShader->compute(renderer->getDeviceContext(), threadCountWidth, threadCountHeight, 1);
			pongJumpFLoodShader->unbind(renderer->getDeviceContext());
			ping = true;
		}
	}
}
void App1::DisplacementPass() {

	if (inIterativeDisplacement) {
		displacementShader->setShaderParameters(renderer->getDeviceContext(), computeShader->getSRV(), jumpFloodingShader->getSRV(), penetrationShader->getSRV());
		displacementShader->compute(renderer->getDeviceContext(), threadCountWidth, threadCountHeight, 1);
		displacementShader->unbind(renderer->getDeviceContext());
	}
	else {
		displacementMethodBShader->setShaderParameters(renderer->getDeviceContext(), computeShader->getSRV(), jumpFloodingShader->getSRV(), penetrationShader->getSRV());
		displacementMethodBShader->compute(renderer->getDeviceContext(), threadCountWidth, threadCountHeight, 1);
		displacementMethodBShader->unbind(renderer->getDeviceContext());
	}
}

void App1::VolumePreservationPass()
{
	bool ping = true;
	bool hasStarted = false;

	for (int i = 0; i < 16; i++) {

		if (ping) {
			if (!hasStarted) {
				volumePreservationShader->setShaderParameters(renderer->getDeviceContext(), computeShader->getSRV(), displacementShader->getSRV(), heightTreshold);
				volumePreservationShader->compute(renderer->getDeviceContext(), threadCountWidth, threadCountHeight, 1);
				volumePreservationShader->unbind(renderer->getDeviceContext());
				hasStarted = true;
				ping = false;
			}
			else {
				volumePreservationShader->setShaderParameters(renderer->getDeviceContext(), computeShader->getSRV(), volumePreservationPongShader->getSRV(),  heightTreshold);
				volumePreservationShader->compute(renderer->getDeviceContext(), threadCountWidth, threadCountHeight, 1);
				volumePreservationShader->unbind(renderer->getDeviceContext());
				ping = false;
			}
		}
		else {
			volumePreservationPongShader->setShaderParameters(renderer->getDeviceContext(), computeShader->getSRV(), volumePreservationShader->getSRV(), heightTreshold);
			volumePreservationPongShader->compute(renderer->getDeviceContext(), threadCountWidth, threadCountHeight, 1);
			volumePreservationPongShader->unbind(renderer->getDeviceContext());
			ping = true;
		}
	}
}

void App1::GenerateNormalsPass()
{
	normalMapShader->setShaderParameters(renderer->getDeviceContext(), writeComputeShader->getSRV(), jumpFloodingShader->getSRV(), computeShader->getSRV());
	normalMapShader->compute(renderer->getDeviceContext(), threadCountWidth, threadCountHeight, 1);
	normalMapShader->unbind(renderer->getDeviceContext());

}


void App1::WriteComputePass() {

	if (inIterativeDisplacement) {
		writeComputeShader->setShaderParameters(renderer->getDeviceContext(), volumePreservationPongShader->getSRV());
		writeComputeShader->compute(renderer->getDeviceContext(), threadCountWidth, threadCountHeight, 1);
		writeComputeShader->unbind(renderer->getDeviceContext());
	}
	else {
		writeComputeShader->setShaderParameters(renderer->getDeviceContext(), displacementMethodBShader->getSRV());
		writeComputeShader->compute(renderer->getDeviceContext(), threadCountWidth, threadCountHeight, 1);
		writeComputeShader->unbind(renderer->getDeviceContext());
	}
	hasReset = true;
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	// Render floor
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0, 0, 0);
	mesh->sendData(renderer->getDeviceContext());

	manipulationShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, writeComputeShader->getSRV(),
		textureMgr->getTexture(L"surface"), scale, directionalLight);
	manipulationShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render model
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(teapotX + 25, teapotY, teapotZ);
	model->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"black"));
	textureShader->render(renderer->getDeviceContext(), model->getIndexCount());

	worldMatrix = XMMatrixTranslation(footprintPositionA.x, footprintPositionA.y, footprintPositionA.z);
	shoeA->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"black"));
	textureShader->render(renderer->getDeviceContext(), shoeA->getIndexCount());

	worldMatrix = XMMatrixTranslation(footprintPositionB.x, footprintPositionB.y, footprintPositionB.z);
	shoeB->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"black"));
	textureShader->render(renderer->getDeviceContext(), shoeB->getIndexCount());

	renderer->setZBuffer(false);
	worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering

	orthoMeshLeft->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, writeComputeShader->getSRV());
	textureShader->render(renderer->getDeviceContext(), orthoMeshLeft->getIndexCount());
	renderer->setZBuffer(true);

	gui();
	//renderer->endScene();
}

bool App1::VolumePreservationTest()
{
	vpTestShader->setShaderParameters(renderer->getDeviceContext(), writeComputeShader->getSRV(), volume);
	vpTestShader->compute(renderer->getDeviceContext(), threadCountWidth, threadCountHeight, 1);
	vpTestShader->unbind(renderer->getDeviceContext());
	return true;
}

void App1::FootprintDemo()
{

	FootprintMovement(downA, heightA, footprintPositionA);
	FootprintMovement(downB, heightB, footprintPositionB);

	if (!hasReset) {
		footprintPositionA = XMFLOAT4(20, 0, -10, 0);
		footprintPositionB = XMFLOAT4(10, 12.1, -30, 0);
	}
}

void App1::FootprintMovement(bool& direction, float& height, XMFLOAT4& vector)
{

	if (height <= 4) {
		direction = true;
	}
	if (height > 12) {
		direction = false;
	}

	if (direction) {
		height += timer->getTime() * 8;
	}
	else {
		height -= timer->getTime() * 8;
	}
	if (height > 8) {
		vector.z += timer->getTime() * 23;
	}
	vector.y = height;

}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);
	// Build UI
	ImGui::Text("Volume: %.5f", vpTestShader->totalVolume);
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	ImGui::Checkbox("Demo mode", &inDemo);
	ImGui::Checkbox("Iterative Displacement Mode", &inIterativeDisplacement);
	ImGui::Checkbox("Reset", &hasReset);
	ImGui::SliderFloat("Height Treshold: ", &heightTreshold, 0.f, .750f);
	ImGui::SliderFloat("Object X: ", &teapotX, -100.f, 100.f);
	ImGui::SliderFloat("Object Y: ", &teapotY, 4.f, 100.f);
	ImGui::SliderFloat("Object Z: ", &teapotZ, -100.f, 100.f);

	float totalTime = 0.0f;
	for (TIMESTAMP timestamp = TIMESTAMP_BEGIN; timestamp < TIMESTAMP_TOTAL; timestamp = TIMESTAMP(timestamp + 1)) {
		totalTime += gpuProfiler->GetAvgTimings(timestamp);
	}

	ImGui::Text("Draw time: %0.2f ms\n"
		"   DEPTH: %0.2f ms\n"
		"   COMPUTE READ: %0.2f ms\n"
		"   COMPUTE PENETRATION: %0.2f ms\n"
		"   COMPUTE JUMP FLOOD: %0.2f ms\n"
		"   COMPUTE DISPLACMENT: %0.2f ms\n"
		"   COMPUTE VOLUME: %0.2f ms\n"
		"   COMPUTE WRITE: %0.2f ms\n"
		"   SCENE: %0.2f ms\n",

		1000.0f * totalTime,
		1000.0f * gpuProfiler->GetAvgTimings(TIMESTAMP_DEPTH),
		1000.0f * gpuProfiler->GetAvgTimings(TIMESTAMP_COMPUTE_READ),
		1000.0f * gpuProfiler->GetAvgTimings(TIMESTAMP_COMPUTE_PENETRATION),
		1000.0f * gpuProfiler->GetAvgTimings(TIMESTAMP_COMPUTE_JUMPFLOOD),
		1000.0f * gpuProfiler->GetAvgTimings(TIMESTAMP_COMPUTE_DISPLACE),
		1000.0f * gpuProfiler->GetAvgTimings(TIMESTAMP_COMPUTE_VOLUMEPRESERVE),
		1000.0f * gpuProfiler->GetAvgTimings(TIMESTAMP_COMPUTE_WRITE),
		1000.0f * gpuProfiler->GetAvgTimings(TIMESTAMP_SCENE));

	ImGui::Text("FPS: %.2f", timer->getFPS());

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

