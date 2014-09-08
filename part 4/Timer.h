#ifndef TIMER_H_
#define TIMER_H_

#include <Game.h>

#include <memory>

namespace gp = gameplay;

class Time final
{
public:
	explicit Time(double time = 0.0) : m_time(time){}

	float asSeconds() const { return static_cast<float>(m_time / 1000.0); }
	double asMilliseconds() const { return m_time; }

private:
	double m_time;
};

class Timer final
{
public:
	typedef std::unique_ptr<Timer> Ptr;

	Timer() : m_startTime(gp::Game::getGameTime()){}
	Time restart()
	{
		Time time = elapsed();
		m_startTime = gp::Game::getGameTime();
		return time;
	}

	Time elapsed() const { return Time(gp::Game::getGameTime() - m_startTime); }


private:
	double m_startTime;
};

#endif