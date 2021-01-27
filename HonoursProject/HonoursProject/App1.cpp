
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
	model = new Model(renderer->getDevice(), renderer->getDeviceContext(), "res/teapot.obj");
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	textureMgr->loadTexture(L"snow", L"res/snowtex.png");
	textureMgr->loadTexture(L"snowheight", L"res/snowheight.png");
	textureMgr->loadTexture(L"black", L"res/black.png");
	textureMgr->loadTexture(L"height", L"res/height.png");
	textureMgr->loadTexture(L"bunny", L"res/bunny.png");

	hasStarted = false;

	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	manipulationShader = new ManipulationShader(renderer->getDevice(), hwnd);
	computeShader = new ComputeShader(renderer->getDevice(), hwnd, screenWidth, screenHeight);
	writeComputeShader = new WriteComputeShader(renderer->getDevice(), hwnd, screenWidth, screenHeight);
	vpTestShader = new VolumePreservationTestShader(renderer->getDevice(), hwnd, screenWidth, screenHeight);

	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth/4, screenHeight/4, screenWidth * .35f, screenHeight * .35f);
	orthoMeshLeft = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth/4, screenHeight/4, -screenWidth * .35f, -screenHeight * .35f);

	gpuProfiler = new GPUProfiler;
	gpuProfiler->Init(renderer->getDeviceContext(), renderer->getDevice());
	
	int sceneWidth = 100;
	int sceneHeight = 100;
	
	renderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	deformationMap = new DeformationMap(renderer->getDevice(), screenWidth, screenHeight);
	
	lightX = 39.f;
	lightY = -48.f;
	lightZ = 7.f;
	lightDirX = 0.f;
	lightDirY = 1.f;
	scale = 6.f;
	newLightDirX = 0.5f;
	newLightDirY = -.9f;
	newLightDirZ = .5f;
	teapotX = 35.f;
	teapotY = 5.f;
	teapotZ = -60.f;

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

	blackCamera = new Camera;
	blackCamera->setPosition(depthLightX, depthLightY, depthLightZ);
	blackCamera->setRotation(depthDirX, depthDirY, depthDirZ);
	blackCamera->update();

	depthCamera = new DepthCamera(input, sWidth, sHeight, wnd);
	depthCamera->setPos(depthLightX, depthLightY, depthLightZ);
	depthCamera->setRot(depthDirX, depthDirY, depthDirZ);
	depthCamera->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

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
	gpuProfiler->CheckTimestamp(TIMESTAMP_COMPUTE_WRITE);
	WriteComputePass();
	gpuProfiler->CheckTimestamp(TIMESTAMP_COMPUTE_READ);

	// Render scene
	finalPass();
	gpuProfiler->CheckTimestamp(TIMESTAMP_SCENE);

	gpuProfiler->DataWaitandUpdate();
	
	renderer->endScene();
	gpuProfiler->EndFrame();

	//VolumePreservationTest();
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
	
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, writeComputeShader->getSRV(),
		textureMgr->getTexture(L"black"), deformationMap->getDepthMapSRV(), scale);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(teapotX + 10, teapotY, teapotZ);
	//XMMATRIX scaleMatrix = XMMatrixScaling(15.5f, 15.5f, 15.5f);
	//worldMatrix = XMMatrixMultiply(XMMatrixMultiply(worldMatrix, scaleMatrix), XMMatrixTranslation(teapotX + 10, teapotY, teapotZ));
	// Render model
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, textureMgr->getTexture(L"black"),
		textureMgr->getTexture(L"black"), textureMgr->getTexture(L"black"), 0);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	worldMatrix = XMMatrixTranslation(teapotX - 25, teapotY, teapotZ);
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, textureMgr->getTexture(L"black"),
		textureMgr->getTexture(L"black"), textureMgr->getTexture(L"black"), 0);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	worldMatrix = XMMatrixTranslation(teapotX + 45, teapotY, teapotZ);
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, textureMgr->getTexture(L"black"),
		textureMgr->getTexture(L"black"), textureMgr->getTexture(L"black"), 0);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::ComputePass() {

	// Check if the compute pass has already been called
	// if it hasnt, pass in a black image for the height map and the deformation map
	// this compute shader then adds the two inputs together
	if (!hasStarted) {
		computeShader->setShaderParameters(renderer->getDeviceContext(), textureMgr->getTexture(L"black"), deformationMap->getDepthMapSRV());
		computeShader->compute(renderer->getDeviceContext(), ceil(sWidth / 256.f), sHeight, 1);
		computeShader->unbind(renderer->getDeviceContext());
		hasStarted = true;
	}

	// if it has, pass in the result of the write compute pass and the the deformation map
	else if (hasStarted) {
		computeShader->setShaderParameters(renderer->getDeviceContext(), writeComputeShader->getSRV(), deformationMap->getDepthMapSRV());
		computeShader->compute(renderer->getDeviceContext(), ceil(sWidth / 256.f), sHeight, 1);
		computeShader->unbind(renderer->getDeviceContext());
	}

	computeResult = computeShader->getSRV();
}

void App1::WriteComputePass() {

	// write compute pass is given the result of the standard compute pass and just outputs the same image
	writeComputeShader->setShaderParameters(renderer->getDeviceContext(), computeShader->getSRV());
	writeComputeShader->compute(renderer->getDeviceContext(), ceil(sWidth / 256.f), sHeight, 1);
	writeComputeShader->unbind(renderer->getDeviceContext());
	hasStarted = true;
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

	//worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	manipulationShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, writeComputeShader->getSRV(),
		textureMgr->getTexture(L"snowheight"), renderTexture->getShaderResourceView(), scale, directionalLight);
	manipulationShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render model
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(teapotX + 10, teapotY, teapotZ);
	//XMMATRIX scaleMatrix = XMMatrixScaling(5.5f, 5.5f, 5.5f);
	//worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	//XMMATRIX scaleMatrix = XMMatrixScaling(15.5f, 15.5f, 15.5f);
	//worldMatrix = XMMatrixMultiply(XMMatrixMultiply(worldMatrix, scaleMatrix), XMMatrixTranslation(teapotX + 10, teapotY, teapotZ));

	// Rendering Teapots
	model->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"));
	textureShader->render(renderer->getDeviceContext(), model->getIndexCount());

	worldMatrix = XMMatrixTranslation(teapotX - 25, teapotY, teapotZ);
	model->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"));
	textureShader->render(renderer->getDeviceContext(), model->getIndexCount());

	worldMatrix = XMMatrixTranslation(teapotX + 45, teapotY, teapotZ);
	model->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"));
	textureShader->render(renderer->getDeviceContext(), model->getIndexCount());


	renderer->setZBuffer(false);
	worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering

	orthoMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, deformationMap->getDepthMapSRV() /*renderTexture->getShaderResourceView()*/);
	textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	
	orthoMeshLeft->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, writeComputeShader->getSRV());
	textureShader->render(renderer->getDeviceContext(), orthoMeshLeft->getIndexCount());
	renderer->setZBuffer(true);

	gui();
	//renderer->endScene();
}

bool App1::VolumePreservationTest()
{
	vpTestShader->setShaderParameters(renderer->getDeviceContext(), nullptr, nullptr, 0, computeShader->getSRV(), volume);
	vpTestShader->compute(renderer->getDeviceContext(), ceil(sWidth / 256.f), sHeight, 1);
	vpTestShader->unbind(renderer->getDeviceContext());

	if (vpTestShader->ReadFromGPU(renderer->getDevice(), renderer->getDeviceContext())) {
		int asdf = 0;
	}
	return true;
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	ImGui::SliderFloat("Scale: ", &scale, 0.f, 25.f);
	ImGui::SliderFloat("Move: ", &teapotX, 0.f, 100.f);
	ImGui::SliderFloat("NewLight X: ", &newLightDirX, -1.f, 1.f);
	ImGui::SliderFloat("NewLight Y: ", &newLightDirY, -1.f, 1.f);
	ImGui::SliderFloat("NewLight Z: ", &newLightDirZ, -1.f, 1.f);
	ImGui::SliderFloat("Teapot X: ", &teapotX, -100.f, 100.f);
	ImGui::SliderFloat("Teapot Y: ", &teapotY, -5.f, 100.f);
	ImGui::SliderFloat("TEapot Z: ", &teapotZ, -100.f, 100.f);
	ImGui::SliderFloat("DepthCamera X: ", &depthLightX, -100.f, 100.f);
	ImGui::SliderFloat("DepthCamera Y: ", &depthLightY, -100.f, 100.f);
	ImGui::SliderFloat("DepthCamera Z: ", &depthLightZ, -100.f, 100.f);
	ImGui::SliderFloat("DepthDir X: ", &depthDirX,-360.f, 360.f);
	ImGui::SliderFloat("DepthDir Y: ", &depthDirY,-360.f, 360.f);
	ImGui::SliderFloat("DepthDir Z: ", &depthDirZ,-360.f, 360.f);

	float totalTime = 0.0f;
	for (TIMESTAMP timestamp = TIMESTAMP_BEGIN; timestamp < TIMESTAMP_TOTAL; timestamp = TIMESTAMP(timestamp + 1)) {
		totalTime += gpuProfiler->GetAvgTimings(timestamp);
	}

	ImGui::Text("Draw time: %0.2f ms\n"
		"   DEPTH: %0.2f ms\n"
		"   COMPUTE READ: %0.2f ms\n"
		"   COMPUTE WRITE: %0.2f ms\n"
		"   SCENE: %0.2f ms\n"
		"GPU frame time: %0.2f ms\n",
		1000.0f * totalTime,
		1000.0f * gpuProfiler->GetAvgTimings(TIMESTAMP_DEPTH),
		1000.0f * gpuProfiler->GetAvgTimings(TIMESTAMP_COMPUTE_WRITE),
		1000.0f * gpuProfiler->GetAvgTimings(TIMESTAMP_COMPUTE_READ),
		1000.0f * gpuProfiler->GetAvgTimings(TIMESTAMP_SCENE));

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

