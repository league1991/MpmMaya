#include "stdafx.h"

bool StatusRecorder::getStatus( int ithFrame, MpmStatus& status )
{
	map<int, MpmStatus>::iterator pS = m_statusList.find(ithFrame);
	if (pS != m_statusList.end())
	{
		status = pS->second;
		return true;
	}
	return true;
}

const MpmStatus* StatusRecorder::getStatusPtr( int ithFrame )
{
	map<int, MpmStatus>::iterator pS = m_statusList.find(ithFrame);
	if (pS != m_statusList.end())
	{
		return &pS->second;
	}
	return NULL;
}

bool StatusRecorder::addStatus( int ithFrame, const MpmStatus& status )
{
	m_statusList[ithFrame] = status;
	return true;
}

void StatusRecorder::clear( int startFrame /*= 0*/ )
{
	m_statusList.clear();
}

bool StatusRecorder::readStatus(const char* fileName, int ithFrame, bool append )
{
	if (append)
	{
		StatusMap::iterator p = m_statusList.find(ithFrame);
		if (p != m_statusList.end())
		{
			MpmStatus& status = p->second;
			ifstream file(fileName, ios::in|ios::binary);
			if (file)
			{
				status.readStatus(file, append);
				file.close();
				return true;
			}
			return false;
		}
	}
	MpmStatus status;
	ifstream file(fileName, ios::in|ios::binary);
	if (file)
	{
		status.readStatus(file, append);
		file.close();
		m_statusList[ithFrame] = status;
		return true;
	}

	return false;
}

bool StatusRecorder::writeStatus(const char* fileName, int ithFrame )
{
	StatusMap::iterator p = m_statusList.find(ithFrame);
	if (p != m_statusList.end())
	{
		ofstream file(fileName, ios::out|ios::binary);
		if (file)
		{
			p->second.writeStatus(file);
			file.close();
			return true;
		}
	}
	return false;
}

MpmStatus::MpmStatus( const deque<Particle>& particles, const vector<Matrix4f>& mat, const GridField* grid )
{
	m_particles = particles;
	m_matrices  = mat;
	
	int nCell = grid->grid_division[0] * grid->grid_division[1] * grid->grid_division[2];
	m_gridData.resize(nCell);
	for (int i = 0; i < nCell; ++i)
	{
		GridNode* node = grid->gridBuffer + i;
		GridData& data = m_gridData[i];

		data.m_collisionSdf = node->collision_sdf;
		data.m_collisionVelocity = node->collision_velocity;
	}
}

void MpmStatus::draw()const
{
	float minSpeed = FLT_MAX;
	float maxSpeed = -FLT_MAX;
	for (int i = 0; i < m_particles.size(); ++i)
	{
		const Vector3f& v = m_particles[i].velocity;
		float speed = v.norm();
		minSpeed = speed < minSpeed ? speed : minSpeed;
		maxSpeed = speed > maxSpeed ? speed : maxSpeed;
	}
	Vector3f blue(0, 163.f/255.f, 232.f/255.f);
	Vector3f white(1.f, 1.f, 1.f);
	glBegin(GL_POINTS);
	for (int i = 0; i < m_particles.size(); ++i)
	{
		const Particle* p = &m_particles[i];
		if(!p)
			continue;

		Vector3f pos = p->position;
		float speed = p->velocity.norm();
		float speedRatio = (speed - minSpeed) / (maxSpeed - minSpeed + 0.001f);
		Vector3f clr = blue * (1 - speedRatio) + white * speedRatio;
		glColor3f(clr[0], clr[1], clr[2]);
		glVertex3f(pos[0],pos[1],pos[2]);
	}
	glEnd();
}

bool MpmStatus::copyGrid( GridField* grid )const
{
	if (!grid)
	{
		return false;
	}
	int nCell = grid->grid_division[0] * grid->grid_division[1] * grid->grid_division[2];
	if (nCell == m_gridData.size())
	{
		GridNode* cell = grid->gridBuffer;
		for (int i = 0; i < nCell; ++i, ++cell)
		{
			cell->collision_sdf_prev = m_gridData[i].m_collisionSdf;
			cell->collision_velocity_prev = m_gridData[i].m_collisionVelocity;
		}
	}
	return true;
}

bool MpmStatus::copyMatrices(vector<Matrix4f>& mat)
{
	mat = m_matrices;
	return true;
}

bool MpmStatus::copy( deque<Particle>& particles)const
{
	particles = m_particles;
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
	m_gridData.clear();
	m_matrices.clear();
}

void MpmStatus::getParticlePos( int idx, Vector3f& pos )const
{
	if (idx >= 0 && idx < m_particles.size())
	{
		pos = m_particles[idx].position;
	}
}


void MpmStatus::getParticleVelocity( int idx, Vector3f& vel)const
{
	if (idx >= 0 && idx < m_particles.size())
	{
		vel = m_particles[idx].velocity;
	}
}

int MpmStatus::getParticleCount()const
{
	return m_particles.size();
}

void MpmStatus::writeStatus( ofstream& file ) const
{
	int nPtcl = m_particles.size();
	file.write((char*)&nPtcl, sizeof(int));
	for (int i =0; i < nPtcl; ++i)
	{
		const Particle& ptcl = m_particles[i];
		file.write((char*)&ptcl, sizeof(Particle));
	}

	unsigned flag = DATA_MATRIX;
	file.write((char*)&flag, sizeof(flag));
	unsigned nMat = m_matrices.size();
	file.write((char*)&nMat, sizeof(nMat));
	if (nMat > 0)
	{
		file.write((char*)&m_matrices[0], sizeof(Matrix4f) * nMat);
	}

	flag = DATA_GRID;
	file.write((char*)&flag, sizeof(flag));
	unsigned nCell = m_gridData.size();
	file.write((char*)&nCell, sizeof(nCell));
	if (nCell > 0)
	{
		file.write((char*)&m_gridData[0], sizeof(GridData) * nCell);
	}

}

void MpmStatus::readStatus( ifstream& file, bool append )
{
	int nPtcl;
	file.read((char*)&nPtcl, sizeof(int));
	if (!append)
		m_particles.clear();
	for (int i =0; i < nPtcl; ++i)
	{
		Particle ptcl;
		file.read((char*)&ptcl, sizeof(Particle));
		ptcl.pid = m_particles.size();
		m_particles.push_back(ptcl);
	}

	unsigned flag;
	while(1)
	{
		file.read((char*)&flag, sizeof(flag));
		if (!file)
		{
			break;
		}

		unsigned num;
		if (flag == DATA_MATRIX)
		{
			file.read((char*)&num, sizeof(num));
			if (num > 0)
			{
				m_matrices.resize(num);
				file.read((char*)&m_matrices[0], sizeof(Matrix4f)*num);
			}
		}
		else if (flag == DATA_GRID)
		{
			file.read((char*)&num, sizeof(num));
			if (num > 0)
			{
				m_gridData.resize(num);
				file.read((char*)&m_gridData[0], sizeof(GridData)*num);
			}
		}
	}

	file.close();
}
