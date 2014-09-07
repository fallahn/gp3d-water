#include <WaterSample.h>

WaterSample game;

namespace
{
	const float mouseSpeed = 0.125f;
	const float mass = 1.8f;
	const float friction = 0.9f;
	const float moveSpeed = 5.5f;
}

//ctor
WaterSample::WaterSample()
	: m_scene	(nullptr),
	m_cameraNode(nullptr),
	m_inputMask	(0u)
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



	gp::Game::getInstance()->setMouseCaptured(true);
	gp::Game::getInstance()->setCursorVisible(false);
}

void WaterSample::finalize()
{
	gp::Game::getInstance()->setMouseCaptured(false);
	gp::Game::getInstance()->setCursorVisible(true);

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

	m_cameraNode->translate(m_cameraAcceleration * moveSpeed * (dt / 1000.f));
	//-------------------------------------
}

void WaterSample::render(float dt)
{
	clear(CLEAR_COLOR_DEPTH, gp::Vector4(0.84f, 0.89f, 1.f, 1.f), 1.0f, 0);
	m_scene->visit(this, &WaterSample::m_drawScene);
}

//private
bool WaterSample::m_drawScene(gp::Node* node)
{
	auto model = node->getModel();
	if (model)
		model->draw();

	return true;
}