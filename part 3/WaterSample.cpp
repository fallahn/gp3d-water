#include <WaterSample.h>

WaterSample game;

namespace
{
	//camera movement consts
	const float mouseSpeed = 0.125f;
	const float mass = 1.8f;
	const float friction = 0.9f;
	const float moveSpeed = 5.5f;
	const gp::Vector3 camStartPosition(0.f, 10.f, -30.f);

	//render target consts
	const unsigned bufferSize = 512u;
	const gp::Vector4 clearColour(0.84f, 0.89f, 1.f, 1.f);
	const float waterOffset = 0.15f;
}

//ctor
WaterSample::WaterSample()
	: m_scene			(nullptr),
	m_cameraNode		(nullptr),
	m_reflectCamNode	(nullptr),
	m_inputMask			(0u),
	m_refractBuffer		(nullptr),
	m_refractBatch		(nullptr),
	m_reflectBuffer		(nullptr),
	m_reflectBatch		(nullptr),
	m_showBuffers		(true)
{

}


//public
bool WaterSample::mouseEvent(gp::Mouse::MouseEvent evt, int x, int y, int wheelDelta)
{
	switch (evt)
	{
	case gp::Mouse::MOUSE_MOVE:
	{
		auto xMovement = MATH_DEG_TO_RAD(-x * mouseSpeed);
		auto yMovement = MATH_DEG_TO_RAD(-y * mouseSpeed);

		m_cameraNode->rotateY(xMovement);
		m_cameraNode->getFirstChild()->rotateX(yMovement);

		m_reflectCamNode->rotateY(xMovement);
		m_reflectCamNode->getFirstChild()->rotateX(-yMovement);
	}
		return true;
	case gp::Mouse::MOUSE_PRESS_LEFT_BUTTON:
		m_inputMask |= Button::Forward;
		return true;
	case gp::Mouse::MOUSE_RELEASE_LEFT_BUTTON:
		m_inputMask &= ~Button::Forward;
		return true;
	case gp::Mouse::MOUSE_PRESS_RIGHT_BUTTON:
		m_inputMask |= Button::Back;
		return true;
	case gp::Mouse::MOUSE_RELEASE_RIGHT_BUTTON:
		m_inputMask &= ~Button::Back;
		return true;
	default: return false;
	}

	return false;
}

void WaterSample::keyEvent(gp::Keyboard::KeyEvent evt, int key)
{
	if (evt == gp::Keyboard::KEY_PRESS)
	{
		switch (key)
		{
		case gp::Keyboard::KEY_ESCAPE:
			exit();
			break;
		case gp::Keyboard::KEY_SPACE:
			m_showBuffers = !m_showBuffers;
			break;
		}
	}
}

//protected
void WaterSample::initialize()
{
	m_scene = gp::Scene::load("res/scenes/watersample.scene");
	m_waterHeight = m_scene->findNode("Water")->getTranslationY();

	//----set up a first person style camera----//
	m_cameraNode = gp::Node::create("cameraNode");
	m_cameraNode->setTranslation(camStartPosition);

	auto camPitchNode = gp::Node::create();
	gp::Matrix m;
	gp::Matrix::createLookAt(m_cameraNode->getTranslation(), gp::Vector3::zero(), gp::Vector3::unitY(), &m);
	camPitchNode->rotate(m);
	m_cameraNode->addChild(camPitchNode);
	m_scene->addNode(m_cameraNode);

	auto camera = gp::Camera::createPerspective(45.f, gp::Game::getInstance()->getAspectRatio(), 0.1f, 150.f);
	camPitchNode->setCamera(camera);
	m_scene->setActiveCamera(camera);
	SAFE_RELEASE(camera);
	SAFE_RELEASE(camPitchNode);

	//add a second camera do draw the reflections
	m_reflectCamNode = gp::Node::create("reflectCamNode");
	m_reflectCamNode->setTranslation(camStartPosition.x, -camStartPosition.y, camStartPosition.z);

	camPitchNode = gp::Node::create();
	gp::Matrix::createLookAt(m_reflectCamNode->getTranslation(), gp::Vector3::zero(), gp::Vector3::unitY(), &m);
	camPitchNode->rotate(m);
	m_reflectCamNode->addChild(camPitchNode);

	camera = gp::Camera::createPerspective(45.f, gp::Game::getInstance()->getAspectRatio(), 0.1f, 150.f);
	camPitchNode->setCamera(camera);
	SAFE_RELEASE(camera);
	SAFE_RELEASE(camPitchNode);

	//------------------------------------------//


	//---render buffer and preview for refraction---//
	m_refractBuffer = gp::FrameBuffer::create("refractBuffer", bufferSize, bufferSize);
	
	auto refractDepthTarget = gp::DepthStencilTarget::create("refractDepth", gp::DepthStencilTarget::DEPTH, bufferSize, bufferSize);
	m_refractBuffer->setDepthStencilTarget(refractDepthTarget);
	SAFE_RELEASE(refractDepthTarget);

	m_refractBatch = gp::SpriteBatch::create(m_refractBuffer->getRenderTarget()->getTexture());
	//----------------------------------------------//

	//---render buffer and preview for reflection---//
	m_reflectBuffer = gp::FrameBuffer::create("reflectBuffer", bufferSize, bufferSize);

	auto reflectDepthTarget = gp::DepthStencilTarget::create("reflectDepth", gp::DepthStencilTarget::DEPTH, bufferSize, bufferSize);
	m_reflectBuffer->setDepthStencilTarget(reflectDepthTarget);
	SAFE_RELEASE(reflectDepthTarget);

	m_reflectBatch = gp::SpriteBatch::create(m_reflectBuffer->getRenderTarget()->getTexture());
	//----------------------------------------------//

	//---bind uniforms to material properties---//
	auto groundMaterial = m_scene->findNode("Ground")->getModel()->getMaterial();
	groundMaterial->getParameter("u_clipPlane")->bindValue(this, &WaterSample::m_getClipPlane);
	groundMaterial->getParameter("u_directionalLightDirection[0]")->bindValue(this, &WaterSample::m_getLightDirection);

	auto waterMaterial = m_scene->findNode("Water")->getModel()->getMaterial();
	auto refractSampler = gp::Texture::Sampler::create(m_refractBuffer->getRenderTarget()->getTexture());
	waterMaterial->getParameter("u_refractionTexture")->setSampler(refractSampler);
	SAFE_RELEASE(refractSampler);

	auto reflectSampler = gp::Texture::Sampler::create(m_reflectBuffer->getRenderTarget()->getTexture());
	waterMaterial->getParameter("u_reflectionTexture")->setSampler(reflectSampler);
	SAFE_RELEASE(reflectSampler);

	waterMaterial->getParameter("u_worldViewProjectionReflectionMatrix")->bindValue(this, &WaterSample::m_getReflectionMatrix);
	//------------------------------------------//

	gp::Game::getInstance()->setMouseCaptured(true);
	gp::Game::getInstance()->setCursorVisible(false);
}

void WaterSample::finalize()
{
	gp::Game::getInstance()->setMouseCaptured(false);
	gp::Game::getInstance()->setCursorVisible(true);

	SAFE_DELETE(m_reflectBatch);
	SAFE_RELEASE(m_reflectBuffer);

	SAFE_DELETE(m_refractBatch);
	SAFE_RELEASE(m_refractBuffer);

	SAFE_RELEASE(m_reflectCamNode);
	SAFE_RELEASE(m_cameraNode);
	SAFE_RELEASE(m_scene);
}

void WaterSample::update(float dt)
{
	//move the camera by applying a force
	gp::Vector3 force;
	if (m_inputMask & Button::Forward)
		force += m_cameraNode->getFirstChild()->getForwardVectorWorld();
	if (m_inputMask & Button::Back)
		force -= m_cameraNode->getFirstChild()->getForwardVectorWorld();

	m_cameraAcceleration += force / mass;
	m_cameraAcceleration *= friction;
	if (m_cameraAcceleration.lengthSquared() < 0.01f)
		m_cameraAcceleration = gp::Vector3::zero();

	m_cameraNode->translate(m_cameraAcceleration * moveSpeed * (dt / 1000.f));

	//make sure the reflection camera follows
	auto position = m_cameraNode->getTranslation();
	position.y = -position.y + m_waterHeight * 2.f;
	m_reflectCamNode->setTranslation(position);
	//-------------------------------------
}

void WaterSample::render(float dt)
{
	//update the refract buffer
	auto defaultBuffer = m_refractBuffer->bind();
	auto defaultViewport = getViewport();
	setViewport(gp::Rectangle(bufferSize, bufferSize));

	m_clipPlane.y = -1.f;
	m_clipPlane.w = m_waterHeight + waterOffset;
	clear(CLEAR_COLOR_DEPTH, clearColour, 1.0f, 0);
	m_scene->visit(this, &WaterSample::m_drawScene, false);

	//switch plane direction and camera, and update reflection buffer
	m_reflectBuffer->bind();
	m_clipPlane.y = 1.f;
	m_clipPlane.w = -m_waterHeight + waterOffset;

	auto defaultCam = m_scene->getActiveCamera();
	m_scene->setActiveCamera(m_reflectCamNode->getFirstChild()->getCamera());
	//store the mvp matrix here as it is only valid when reflect camera is active
	m_worldViewProjectionReflection = m_scene->findNode("Water")->getWorldViewProjectionMatrix();
	clear(CLEAR_COLOR_DEPTH, clearColour, 1.0f, 0);
	m_scene->visit(this, &WaterSample::m_drawScene, false);

	//draw the final scene
	defaultBuffer->bind();
	setViewport(defaultViewport);
	m_clipPlane = gp::Vector4::zero();
	m_scene->setActiveCamera(defaultCam);
	clear(CLEAR_COLOR_DEPTH, clearColour, 1.0f, 0);
	m_scene->visit(this, &WaterSample::m_drawScene, true);

	//draw preview if enabled
	if (m_showBuffers)
	{
		m_refractBatch->start();
		m_refractBatch->draw(gp::Vector3(0.f, 4.f, 0.f), gp::Rectangle(bufferSize, bufferSize), gp::Vector2(426.f, 240.f));
		m_refractBatch->finish();

		m_reflectBatch->start();
		m_reflectBatch->draw(gp::Vector3(430.f, 4.f, 0.f), gp::Rectangle(bufferSize, bufferSize), gp::Vector2(426.f, 240.f));
		m_reflectBatch->finish();
	}
}

//private
bool WaterSample::m_drawScene(gp::Node* node, bool drawWater)
{
	std::string id = node->getId();
	if (!drawWater && id == "Water") return true;

	auto model = node->getModel();
	if (model)
		model->draw();

	return true;
}