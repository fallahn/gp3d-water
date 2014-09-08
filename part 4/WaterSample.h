#ifndef WATER_SAMPLE_H_
#define WATER_SAMPLE_H_

#include <Gameplay.h>
#include <Timer.h>

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
		Back    = (1 << 1),
		Left    = (1 << 2),
		Right   = (1 << 3)
	};

	gp::Scene* m_scene;
	gp::Node* m_cameraNode;
	gp::Node* m_reflectCamNode;

	unsigned m_inputMask;
	gp::Vector3 m_cameraAcceleration;

	gp::FrameBuffer* m_refractBuffer;
	gp::SpriteBatch* m_refractBatch;

	gp::FrameBuffer* m_reflectBuffer;
	gp::SpriteBatch* m_reflectBatch;

	bool m_showBuffers;

	float m_waterHeight;
	gp::Vector4 m_clipPlane;
	const gp::Vector4& m_getClipPlane() const
	{
		return m_clipPlane;
	}

	gp::Matrix m_worldViewProjectionReflection;
	const gp::Matrix& m_getReflectionMatrix() const
	{
		return m_worldViewProjectionReflection;
	}
	Timer::Ptr m_timer;
	float m_getTime() const
	{
		return m_timer->elapsed().asSeconds() * 0.1f;
	}

	bool m_drawScene(gp::Node* node, bool drawWater);
};

#endif