/*********************************************************************
Matt Marchant 2014
http://trederia.blogspot.com

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#include <WaterSample.h>

WaterSample game;

namespace
{
	//camera movement consts
	const float mouseSpeed = 0.125f;
	const float mass = 1.8f;
	const float friction = 0.9f;
	const float camSpeed = 5.5f;

	//render target consts
	const unsigned bufferSize = 512u;
	const gp::Vector4 clearColour(0.84f, 0.89f, 1.f, 1.f);
	const float waterOffset = 0.55f;
}

//ctor
WaterSample::WaterSample()
	: m_scene			(nullptr),
	m_cameraNode		(nullptr),
	m_inputMask			(0u),
	m_refractBuffer		(nullptr),
	m_refractBatch		(nullptr),
	m_showBuffers		(true)
{

}


//public
bool WaterSample::mouseEvent(gp::Mouse::MouseEvent evt, int x, int y, int wheelDelta)
{
	switch (evt)
	{
	case gp::Mouse::MOUSE_MOVE:
		m_cameraNode->rotateY(MATH_DEG_TO_RAD(-x * mouseSpeed));
		m_cameraNode->getFirstChild()->rotateX(MATH_DEG_TO_RAD(-y * mouseSpeed));
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
		case gp::Keyboard::KEY_W:
		case gp::Keyboard::KEY_UP_ARROW:
			m_inputMask |= Button::Forward;
			break;
		case gp::Keyboard::KEY_S:
		case gp::Keyboard::KEY_DOWN_ARROW:
			m_inputMask |= Button::Back;
			break;
		case gp::Keyboard::KEY_A:
		case gp::Keyboard::KEY_LEFT_ARROW:
			m_inputMask |= Button::Left;
			break;
		case gp::Keyboard::KEY_D:
		case gp::Keyboard::KEY_RIGHT_ARROW:
			m_inputMask |= Button::Right;
			break;
		}
	}
	else if (evt == gp::Keyboard::KEY_RELEASE)
	{
		switch (key)
		{
		case gp::Keyboard::KEY_W:
		case gp::Keyboard::KEY_UP_ARROW:
			m_inputMask &= ~Button::Forward;
			break;
		case gp::Keyboard::KEY_S:
		case gp::Keyboard::KEY_DOWN_ARROW:
			m_inputMask &= ~Button::Back;
			break;
		case gp::Keyboard::KEY_A:
		case gp::Keyboard::KEY_LEFT_ARROW:
			m_inputMask &= ~Button::Left;
			break;
		case gp::Keyboard::KEY_D:
		case gp::Keyboard::KEY_RIGHT_ARROW:
			m_inputMask &= ~Button::Right;
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
	m_cameraNode->setTranslation(0.f, 10.f, -30.f);

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
	//------------------------------------------//


	//---render buffer and preview for refraction---//
	m_refractBuffer = gp::FrameBuffer::create("refractBuffer", bufferSize, bufferSize);

	auto refractDepthTarget = gp::DepthStencilTarget::create("refractDepth", gp::DepthStencilTarget::DEPTH, bufferSize, bufferSize);
	m_refractBuffer->setDepthStencilTarget(refractDepthTarget);
	SAFE_RELEASE(refractDepthTarget);

	m_refractBatch = gp::SpriteBatch::create(m_refractBuffer->getRenderTarget()->getTexture());
	//----------------------------------------------//

	//----add a node to provide light direction-----//
	auto lightNode = gp::Node::create("lightNode");
	lightNode->rotateX(MATH_DEG_TO_RAD(-90));
	m_scene->addNode(lightNode);

	//---bind uniforms to material properties---//
	auto groundMaterial = m_scene->findNode("Ground")->getModel()->getMaterial();
	groundMaterial->getParameter("u_clipPlane")->bindValue(this, &WaterSample::m_getClipPlane);
	groundMaterial->getParameter("u_directionalLightDirection[0]")->bindValue(lightNode, &gp::Node::getForwardVectorView);

	auto waterMaterial = m_scene->findNode("Water")->getModel()->getMaterial();
	auto refractSampler = gp::Texture::Sampler::create(m_refractBuffer->getRenderTarget()->getTexture());
	waterMaterial->getParameter("u_refractionTexture")->setSampler(refractSampler);
	SAFE_RELEASE(refractSampler);
	//------------------------------------------//

	SAFE_RELEASE(lightNode);

	gp::Game::getInstance()->setMouseCaptured(true);
	gp::Game::getInstance()->setCursorVisible(false);
}

void WaterSample::finalize()
{
	gp::Game::getInstance()->setMouseCaptured(false);
	gp::Game::getInstance()->setCursorVisible(true);

	SAFE_DELETE(m_refractBatch);
	SAFE_RELEASE(m_refractBuffer);

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
	if (m_inputMask & Button::Left)
		force += m_cameraNode->getRightVectorWorld();
	if (m_inputMask & Button::Right)
		force -= m_cameraNode->getRightVectorWorld();

	if (force.lengthSquared() > 1.f) force.normalize();

	m_cameraAcceleration += force / mass;
	m_cameraAcceleration *= friction;
	if (m_cameraAcceleration.lengthSquared() < 0.01f)
		m_cameraAcceleration = gp::Vector3::zero();

	m_cameraNode->translate(m_cameraAcceleration * camSpeed * (dt / 1000.f));
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

	//draw the final scene
	defaultBuffer->bind();
	setViewport(defaultViewport);
	m_clipPlane = gp::Vector4::zero();
	clear(CLEAR_COLOR_DEPTH, clearColour, 1.0f, 0);
	m_scene->visit(this, &WaterSample::m_drawScene, true);

	//draw preview if enabled
	if (m_showBuffers)
	{
		m_refractBatch->start();
		m_refractBatch->draw(gp::Vector3(0.f, 4.f, 0.f), gp::Rectangle(bufferSize, bufferSize), gp::Vector2(426.f, 240.f));
		m_refractBatch->finish();
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