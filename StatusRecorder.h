#pragma once

using namespace std;
class MpmStatus
{
public:
	MpmStatus(){}
	MpmStatus(const vector<Particle*>& particles);
	~MpmStatus();
	void				draw();
	bool				copy(const vector<Particle*>& particles);
	int					getParticleCount();
	void			    getParticlePos(int idx, Vector3f& pos);
	void				getParticleVelocity( int idx, Vector3f& vel);
private:
	vector<Particle>	m_particles;
};

class StatusRecorder
{
public:
	StatusRecorder():m_startFrame(0){}

	void				init(int startFrame = 0);

	bool				addStatus(int ithFrame, const MpmStatus& status);
	bool				getStatus(int ithFrame, MpmStatus& status);

	int					getStartFrame();
private:
	int					m_startFrame;
	vector<MpmStatus>	m_statusList;
};