#pragma once

using namespace std;
class MpmStatus
{
public:
	MpmStatus(){}
	MpmStatus(const deque<Particle>& particles);
	~MpmStatus();
	void				draw();
	bool				copy(deque<Particle>& particles)const;
	int					getParticleCount()const;
	void			    getParticlePos(int idx, Vector3f& pos)const;
	void				getParticleVelocity( int idx, Vector3f& vel)const;
	const deque<Particle>& getParticle()const{return m_particles;}
	void				writeStatus(ofstream& file) const;
	void				readStatus(ifstream& file, bool append = false);
private:
	deque<Particle>	m_particles;
};

class StatusRecorder
{
public:
	typedef map<int,MpmStatus> StatusMap;
	StatusRecorder(){}

	void				clear(int startFrame = 0);

	bool				addStatus(int ithFrame, const MpmStatus& status);
	bool				getStatus(int ithFrame, MpmStatus& status);
	const MpmStatus*	getStatusPtr(int ithFrame);
	bool				readStatus(const char* fileName, int ithFrame, bool append);
	bool				writeStatus(const char* fileName, int ithFrame);
private:
	StatusMap			m_statusList;
};