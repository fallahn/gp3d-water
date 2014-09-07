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
		Back    = (1 << 1)
	};

	gp::Scene* m_scene;
	gp::Node* m_cameraNode;

	unsigned m_inputMask;
	gp::Vector3 m_cameraAcceleration;

	gp::FrameBuffer* m_refractBuffer;
	gp::SpriteBatch* m_refractBatch;

	bool m_showBuffers;

	float m_waterHeight;
	gp::Vector4 m_clipPlane;
	const gp::Vector4& m_getClipPlane() const
	{
		return m_clipPlane;
	}
	gp::Vector3 m_getLightDirection() const
	{
		return gp::Vector3(0.f, -1.f, 0.f);
	}

	bool m_drawScene(gp::Node* node, bool drawWater);
};

#endif