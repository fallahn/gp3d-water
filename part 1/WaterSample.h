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
		Back    = (1 << 1),
		Left    = (1 << 2),
		Right   = (1 << 3)
	};

	gp::Scene* m_scene;
	gp::Node* m_cameraNode;

	unsigned m_inputMask;
	gp::Vector3 m_cameraAcceleration;

	bool m_drawScene(gp::Node* node);
};

#endif