#ifndef WATER_SAMPLE_H_
#define WATER_SAMPLE_H_

#include <Gameplay.h>

namespace gp = gameplay;

class WaterSample final : public gp::Game
{
public:
	WaterSample();

	bool mouseEvent(gp::Mouse::MouseEvent evt, int x, int y, int wheelDelta) override;
	void keyEvent(gp::Keyboard::KeyEvent evt, int key) override;

protected:
	void initialize() override;
	void finalize() override;
	void update(float dt) override;
	void render(float dt) override;

private:

	enum Button
	{
		Forward = (1 << 0),
		Back = (1 << 2)
	};

	gp::Scene* m_scene;
	gp::Node* m_cameraNode;

	unsigned m_inputMask;
	gp::Vector3 m_cameraAcceleration;

	bool m_drawScene(gp::Node* node);
};

#endif