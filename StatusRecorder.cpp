#include "stdafx.h"

bool StatusRecorder::getStatus( int ithFrame, MpmStatus& status )
{
	int offset = ithFrame - m_startFrame;
	if (offset < 0 || offset >= m_statusList.size())
	{
		return false;
	}
	else
	{
		status = m_statusList[offset];
	}
	return true;
}

bool StatusRecorder::addStatus( int ithFrame, const MpmStatus& status )
{
	int offset = ithFrame - m_startFrame;
	if (offset < 0 || offset > m_statusList.size())
	{
		return false;
	}
	else if (offset == m_statusList.size())
	{
		m_statusList.push_back(status);
	}
	else
	{
		m_statusList[offset] = status;
	}
	return true;
}

void StatusRecorder::init( int startFrame /*= 0*/ )
{
	m_startFrame = startFrame;
	m_statusList.clear();
}

int StatusRecorder::getStartFrame()
{
	return m_startFrame;
}

MpmStatus::MpmStatus( const vector<Particle*>& particles )
{
	m_particles.resize(particles.size());
	for (int i = 0; i < m_particles.size(); ++i)
	{
		m_particles[i] = *particles[i];
	}
}

void MpmStatus::draw()
{
	glBegin(GL_POINTS);
	for (int i = 0; i < m_particles.size(); ++i)
	{
		Particle* p = &m_particles[i];
		if(!p)
			continue;

		Vector3f pos = p->position;
		glVertex3f(pos[0],pos[1],pos[2]);
	}
	glEnd();
}

bool MpmStatus::copy( const vector<Particle*>& particles )
{
	if (m_particles.size() != particles.size())
	{
		return false;
	}
	for (int i=0; i< m_particles.size(); ++i)
	{
		if (!particles[i])
		{
			return false;
		}
		*particles[i] = m_particles[i];
	}
	return true;
}

MpmStatus::~MpmStatus()
{
// 	for (int i = 0; i < m_particles.size(); ++i)
// 	{
// 		if (m_particles[i])
// 		{
// 			delete m_particles[i];
// 		}
// 	}
	m_particles.clear();
}

void MpmStatus::getParticlePos( int idx, Vector3f& pos )
{
	if (idx >= 0 && idx < m_particles.size())
	{
		pos = m_particles[idx].position;
	}
}


void MpmStatus::getParticleVelocity( int idx, Vector3f& vel)
{
	if (idx >= 0 && idx < m_particles.size())
	{
		vel = m_particles[idx].velocity;
	}
}

int MpmStatus::getParticleCount()
{
	return m_particles.size();
}
