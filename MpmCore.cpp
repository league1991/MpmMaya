#include "stdafx.h"

GridField::GridField( const Vector3f& grid_size, const Vector3f& grid_min, const Vector3i& grid_division, int boundary)
{
	this->grid_size=grid_size;
	this->grid_min=grid_min;
	this->grid_division[0]=grid_division[0];
	this->grid_division[1]=grid_division[1];
	this->grid_division[2]=grid_division[2];
	this->grid_max=grid_min + grid_size.cwiseProduct(grid_division.cast<float>());
	this->boundary = boundary;

	int totalSize = grid_division[0]*grid_division[1]*grid_division[2];
	gridBuffer = new GridNode[totalSize];
	GridNode* pn = gridBuffer;

	grids.resize(boost::extents[grid_division[0]][grid_division[1]][grid_division[2]]);
	for(int i=0;i<grids.shape()[0];i++)
		for(int j=0;j<grids.shape()[1];j++)
			for(int k=0;k<grids.shape()[2];k++)
			{
				grids[i][j][k]= pn;
				pn++;
			}
}

GridField::GridField()
{
	grid_size.setZero();
	grid_division[0] = grid_division[1] = grid_division[2] = 0;
	boundary = 0;
	gridBuffer = NULL;
}

void GridField::clear()
{
// 	for(int i=0;i<grids.shape()[0];i++)
// 		for(int j=0;j<grids.shape()[1];j++)
// 			for(int k=0;k<grids.shape()[2];k++)
// 			{
// 				if(grids[i][j][k])
// 				{
// 					delete grids[i][j][k];
// 				}
// 			}
	if (gridBuffer)
	{
		delete[] gridBuffer;
		gridBuffer = NULL;
	}
}

GridField::~GridField()
{
	clear();
}

float MpmCore::NX_bspline( float m )
{
	m=fabs(m);
	float m2 = m*m;
	float m3 = m2*m;
	float ret;
	if(m<1.0f)
		ret= m3*0.5 - m2 + 2.0/3.0;
	else if(m<2.0f)
		ret= m3/(-6)+ m2 - m*2 + 4.0/3.0;
	else
		return 0;

	if(ret<0.0000001f)
		return 0; 
	return ret;
}

float MpmCore::dNX_bsplineslope( float m )
{
	float abs_m=fabs(m);
	if(abs_m<1)
		return 1.5*m*abs_m-2*m;
	else if(abs_m<2)
		return -m*abs_m/2+m*2-2*m/abs_m;
	else
		return 0;
}

float MpmCore::weight( Vector3f& d_xp )
{
	return NX_bspline(d_xp[0])*NX_bspline(d_xp[1])*NX_bspline(d_xp[2]);
}

Vector3f MpmCore::weight_gradientF( Vector3f& d_xp )
{
	float w0 = NX_bspline(d_xp[0]);
	float w1 = NX_bspline(d_xp[1]);
	float w2 = NX_bspline(d_xp[2]);
	return Vector3f(
		dNX_bsplineslope(d_xp[0])*w1*w2,
		dNX_bsplineslope(d_xp[1])*w2*w0,
		dNX_bsplineslope(d_xp[2])*w0*w1);
}

Matrix3f MpmCore::cauchy_stress( Matrix3f& Fe, Matrix3f& Fp, float particle_volume )
{
	float det_fe=Fe.determinant();
	float det_fp=Fp.determinant();

	//formula 2
	float miu=exp(ctrl_params.hardening*(1-det_fp))*ctrl_params.miu;
	float lambda=exp(ctrl_params.hardening*(1-det_fp))*ctrl_params.lambda;

	//polar decomposition FE=RE SE
	Eigen::JacobiSVD<Eigen::Matrix3f> svd(Fe, Eigen::ComputeFullU| Eigen::ComputeFullV);
	Eigen::Matrix3f Re=svd.matrixU()*svd.matrixV().transpose();

	Matrix3f I;
	I.setIdentity();
	//???? dx of fomular 1
	//return (Fe-Re)*Fe.transpose()*2*miu + Matrix3f(lambda*det_fe*(det_fe-1));
	return ((Fe-Re)*Fe.transpose()*2*miu + I*(lambda*det_fe*(det_fe-1)))*particle_volume*-1;
}

void MpmCore::init_particle_volume_velocity()
{
	for(int pit=0;pit<particles.size();pit++)
	{
		Vector3f p_grid_index_f=particles[pit]->getGridIdx(grid->grid_min,grid->grid_size);
		Vector3i p_grid_index_i=particles[pit]->getGridIdx_int(grid->grid_min,grid->grid_size);
		for(int z=-2; z<=2;z++)
		{
			for(int y=-2; y<=2;y++)
			{
				for(int x=-2; x<=2;x++)
				{
					Vector3i index=p_grid_index_i+Vector3i(x,y,z);
					if(inGrid(index, grid->grid_division))
					{
						Vector3f xp=(particles[pit]->position-grid->grid_size.cwiseProduct(Vector3f(index[0],index[1],index[2]))-grid->grid_min).cwiseQuotient(grid->grid_size);
						float weight_p=weight(xp);
						grid->getNode(index[0], index[1], index[2])->mass+=weight_p*particles[pit]->pmass;
					}
				}
			}
		}
	}

	for(int pit=0;pit<particles.size();pit++)
	{
		float density=0.f;
		Vector3f p_grid_index_f=particles[pit]->getGridIdx(grid->grid_min,grid->grid_size);
		Vector3i p_grid_index_i=particles[pit]->getGridIdx_int(grid->grid_min,grid->grid_size);
		for(int z=-2; z<=2;z++)
		{
			for(int y=-2; y<=2;y++)
			{
				for(int x=-2; x<=2;x++)
				{
					Vector3i index=p_grid_index_i+Vector3i(x,y,z);
					if(inGrid(index, grid->grid_division))
					{
						Vector3f xp=(particles[pit]->position-grid->grid_size.cwiseProduct(Vector3f(index[0],index[1],index[2]))-grid->grid_min).cwiseQuotient(grid->grid_size);
						float weight_p=weight(xp);
						density+=grid->getNode(index[0],index[1],index[2])->mass*weight_p/grid->grid_size[0]/grid->grid_size[1]/grid->grid_size[2];
					}
				}
			}
		}
		particles[pit]->volume=particles[pit]->pmass/density;
	}
}

void MpmCore::parallel_from_particles_to_grid()
{
	int totalCell = grid->grid_division[0] * grid->grid_division[1] * grid->grid_division[2];
	tbb::parallel_for(tbb::blocked_range<int>(0, totalCell, 2500), [&](tbb::blocked_range<int>& r)
	{
		GridNode* pCell = grid->getNode(0,0,0) + r.begin();
		for(int idx = r.begin(); idx != r.end(); idx++, pCell++)
		{
			pCell->mass=0;
			pCell->external_force.setZero();
			pCell->velocity_old.setZero();
			pCell->velocity_new.setZero();
			pCell->active=false;
		}
	});
	
	std::vector<ParticleTemp>& ptclTemp = m_particleTemp;
	if (m_particleTemp.size() != particles.size())
	{
		ptclTemp.resize(particles.size());
	}

	tbb::parallel_for(tbb::blocked_range<int>(0, particles.size(), 2500), [&](tbb::blocked_range<int>& r)
	{
		for(int pit = r.begin(); pit != r.end(); pit++)
		{
			Particle* ptcl = particles[pit];
			ParticleTemp& ptclRes = ptclTemp[pit];
			if (!ptcl->isValid)
				continue;

			Vector3f p_grid_index_f=ptcl->getGridIdx(grid->grid_min,grid->grid_size);
			Vector3i p_grid_index_i=ptcl->getGridIdx_int(grid->grid_min,grid->grid_size);
			Matrix3f cauchyStress = cauchy_stress(ptcl->Fe, ptcl->Fp, ptcl->volume);

			Vector3i cornerIdx = p_grid_index_i + Vector3i(-neighbour,-neighbour,-neighbour);
			ptclRes.cornerCell = grid->getNode(cornerIdx[0], cornerIdx[1], cornerIdx[2]);

			for(int z=-neighbour, ithNeigh = 0; z<=neighbour;z++)
			{
				for(int y=-neighbour; y<=neighbour;y++)
				{
					for(int x=-neighbour; x<=neighbour;x++, ithNeigh++)
					{
						Vector3i index=p_grid_index_i+Vector3i(x,y,z);
						if(inGrid(index, grid->grid_division))
						{
							Vector3f xp=(ptcl->position - grid->grid_size.cwiseProduct(Vector3f(index[0],index[1],index[2])) - 
								grid->grid_min).cwiseQuotient(grid->grid_size);

							ptclRes.weight[ithNeigh] = weight(xp) * ptcl->pmass;
							Vector3f gradientWeight = weight_gradientF(xp).cwiseQuotient(grid->grid_size);
							ptclRes.gradientWeight[ithNeigh] = cauchyStress * gradientWeight;
						}
						else
						{
							ptclRes.weight[ithNeigh] = -1.f;
						}
					}
				}
			}
		}
	});

	const int nx = grid->grid_division[0];
	const int ny = grid->grid_division[1];
	const int w  = neighbour*2+1;
	for(int pit=0;pit<particles.size();pit++)
	{
		if(!particles[pit]->isValid)
			continue;
		Particle* ptcl = particles[pit];
		ParticleTemp& ptclRes = ptclTemp[pit];
		GridNode* pCell = ptclRes.cornerCell;

		for(int z=-neighbour, ithNeigh = 0; z<=neighbour; z++, pCell += nx*(ny-w))
		{
			for(int y=-neighbour; y<=neighbour; y++, pCell += nx-w)
			{
				for(int x=-neighbour; x<=neighbour;x++, ithNeigh++, pCell++)
				{
					if(ptclRes.weight[ithNeigh] > 0.f)
					{
						float weight_p = ptclRes.weight[ithNeigh];
						pCell->mass += weight_p;
						//now velocity is v*m. we need to divide m before use it
						pCell->velocity_old += weight_p * ptcl->velocity;
						//fomular 6
						pCell->external_force += ptclRes.gradientWeight[ithNeigh];
					}
				}
			}
		}
	}
}

void MpmCore::from_particles_to_grid()
{
	for(int x=0;x<grid->grid_division[0];x++)
	{
		for(int y=0;y<grid->grid_division[1];y++)
		{
			for(int z=0;z<grid->grid_division[2];z++)
			{
				GridNode* pCell = grid->getNode(x,y,z);
				pCell->mass=0;
				pCell->external_force.setZero();
				pCell->velocity_old.setZero();
				pCell->velocity_new.setZero();
				pCell->active=false;
			}
		}
	}

	for(int pit=0;pit<particles.size();pit++)
	{
		if(!particles[pit]->isValid)
			continue;
		Particle* ptcl = particles[pit];
		Vector3f p_grid_index_f=ptcl->getGridIdx(grid->grid_min,grid->grid_size);
		Vector3i p_grid_index_i=ptcl->getGridIdx_int(grid->grid_min,grid->grid_size);

		Matrix3f cauchyStress=cauchy_stress(ptcl->Fe, ptcl->Fp, ptcl->volume);
		for(int z=-2; z<=2;z++)
		{
			for(int y=-2; y<=2;y++)
			{
				for(int x=-2; x<=2;x++)
				{
					Vector3i index=p_grid_index_i+Vector3i(x,y,z);
					if(inGrid(index, grid->grid_division))
					{
						Vector3f xp=(ptcl->position - grid->grid_size.cwiseProduct(Vector3f(index[0],index[1],index[2])) - grid->grid_min).cwiseQuotient(grid->grid_size);
						float weight_p=weight(xp);
						Vector3f gradient_weight=weight_gradientF(xp).cwiseQuotient(grid->grid_size);

						GridNode* pCell = grid->getNode(index[0], index[1], index[2]);
						pCell->mass+=weight_p*ptcl->pmass;
						//now velocity is v*m. we need to divide m before use it
						pCell->velocity_old+=weight_p*ptcl->pmass*ptcl->velocity;
						//fomular 6
						pCell->external_force+=cauchyStress*gradient_weight;
					}
				}
			}
		}
	}
}

void MpmCore::compute_grid_velocity()
{
	//fstream fs("G://snow//log.txt",std::ios_base::app);
	//fs<<"frame "<<ctrl_params.frame<<":"<<endl;
	for(int x=0;x<grid->grids.shape()[0];x++)
		for(int y=0;y<grid->grids.shape()[1];y++)
			for(int z=0;z<grid->grids.shape()[2];z++)
			{
				GridNode* cell = grid->getNode(x,y,z);
				if(cell->mass>0.f)
				{
					//as velocity is v*m in step 1. we need to divide m before use it
					cell->velocity_old/=cell->mass;
					cell->external_force+=ctrl_params.gravity*cell->mass;
					//v=v+f/m*deltaT
					cell->velocity_new=cell->velocity_old+cell->external_force/cell->mass*ctrl_params.deltaT;
					cell->active=true;
					//fs<<x<<","<<y<<","<<z<<":"<<grid->grids[x][y][z]->velocity_old[0]<<","<<grid->grids[x][y][z]->velocity_old[1]<<","<<grid->grids[x][y][z]->velocity_old[2]<<"|"<<
					//						//external_force_old[0]<<","<<external_force_old[1]<<","<<external_force_old[2]<<"|"<<
					//						grid->grids[x][y][z]->mass<<endl;

				}
			}
			//fs<<endl;
			//fs.flush();
			//fs.close();
}

bool MpmCore::getSDFNormal( Vector3i& grid_idx, Vector3f& out_sdf_normal )
{
	const int idxX = grid_idx[0];
	const int idxY = grid_idx[1];
	const int idxZ = grid_idx[2];

	GridNode* cell = grid->getNode(idxX, idxY, idxZ);
 	if(cell->collision_sdf<0)
 		return false;
	out_sdf_normal[0]=grid->getNode(idxX-1,idxY,idxZ)->collision_sdf - grid->getNode(idxX+1,idxY,idxZ)->collision_sdf;
	out_sdf_normal[1]=grid->getNode(idxX,idxY-1,idxZ)->collision_sdf - grid->getNode(idxX,idxY+1,idxZ)->collision_sdf;
	out_sdf_normal[2]=grid->getNode(idxX,idxY,idxZ-1)->collision_sdf - grid->getNode(idxX,idxY,idxZ+1)->collision_sdf;
	if(out_sdf_normal.norm()<=0.00000001)
		return false;

	out_sdf_normal.normalize();
	return true;
}

bool MpmCore::getSDFNormal_box( Vector3f& grid_idx_new, Vector3f& out_sdf_normal )
{
	out_sdf_normal= Vector3f(0,0,0);
	if(grid_idx_new[0]>=grid->grid_division[0]-grid->boundary)
		out_sdf_normal[0]=-1;
	if(grid_idx_new[0]<grid->boundary)
		out_sdf_normal[0]=1;

	if(grid_idx_new[1]>=grid->grid_division[1]-grid->boundary)
		out_sdf_normal[1]=-1;
	if(grid_idx_new[1]<grid->boundary)
		out_sdf_normal[1]=1;

	if(grid_idx_new[2]>=grid->grid_division[2]-grid->boundary)
		out_sdf_normal[2]=-1;
	if(grid_idx_new[2]<grid->boundary)
		out_sdf_normal[2]=1;

	if(out_sdf_normal.norm()<=0.00000001)
		return false;
	out_sdf_normal.normalize();
	return true;
}

bool MpmCore::updateVelocityWithSolvingCollision( Vector3f& collider_velocity, Vector3f& grid_velocity, Vector3f& sdf_normal, Vector3f& out_velocity )
{
	Vector3f v_rel=grid_velocity-collider_velocity;
	float vn=v_rel.dot(sdf_normal);
	if(vn>=0)
		return false;

	out_velocity=v_rel-sdf_normal*vn;
	float stickness=vn*ctrl_params.frictionCoeff;
	float out_v_length=out_velocity.norm();
	if(out_v_length<=-stickness)
		out_velocity=collider_velocity;//stick
	else
		out_velocity=out_velocity+out_velocity*stickness/out_v_length+collider_velocity;//friction
	return true;
}

void MpmCore::solve_grid_collision()
{
	for(int x=1;x<grid->grids.shape()[0]-1;x++)
		for(int y=1;y<grid->grids.shape()[1]-1;y++)
			for(int z=1;z<grid->grids.shape()[2]-1;z++)
			{
				GridNode* cell = grid->getNode(x,y,z);
				if(!cell->active)
					continue;
				Vector3i index(x,y,z);
				Vector3f sdf_normal;
				if(getSDFNormal(index,sdf_normal))
				{
					//fs<<x<<","<<y<<","<<z<<","<<sdf_normal[0]<<","<<sdf_normal[1]<<","<<sdf_normal[2]<<endl;
					Vector3f updated_v;
					if( updateVelocityWithSolvingCollision( cell->collision_velocity, cell->velocity_new, sdf_normal, updated_v) )
					{
						cell->velocity_new=updated_v;
					}
				}
			}
}

float MpmCore::clamp( float value, float low, float high )
{
	if(value < low)
		return low;
	if(value> high)
		return high;
	return value;
}

void MpmCore::compute_deformation_gradient_F()
{
	for(int pit=0;pit<particles.size();pit++)
	{
		Particle* ptcl = particles[pit];
		Vector3f p_grid_index_f = ptcl->getGridIdx(grid->grid_min,grid->grid_size);
		Vector3i p_grid_index_i = ptcl->getGridIdx_int(grid->grid_min,grid->grid_size);

		Matrix3f velocity_gradient;
		velocity_gradient.setZero();

		for(int z=-2; z<=2;z++)
		{
			for(int y=-2; y<=2;y++)
			{
				for(int x=-2; x<=2;x++)
				{
					Vector3i index=p_grid_index_i+Vector3i(x,y,z);
					if(inGrid(index, grid->grid_division))
					{
						Vector3f xp=(ptcl->position - grid->grid_size.cwiseProduct(Vector3f(index[0],index[1],index[2]))-grid->grid_min).cwiseQuotient(grid->grid_size);
						Vector3f gradient_weight=weight_gradientF(xp);
						//fomular in step 4
						velocity_gradient+=Eigen::Vector3f(grid->getNode(index[0],index[1],index[2])->velocity_new)*
							Eigen::Vector3f(gradient_weight).transpose();
					}
				}
			}
		}

		//fomular 11
		Eigen::Matrix3f Fe_new=(Eigen::Matrix3f::Identity()+velocity_gradient*ctrl_params.deltaT)*ptcl->Fe;
		Eigen::Matrix3f F_new=Fe_new*ptcl->Fp;
		Eigen::JacobiSVD<Eigen::Matrix3f> svd(Fe_new, Eigen::ComputeFullV | Eigen::ComputeFullU);

		Matrix3f clamped_S;
		clamped_S.setZero();
		clamped_S(0,0)=clamp(svd.singularValues()(0), 1-ctrl_params.critical_compression, 1+ ctrl_params.critical_stretch);
		clamped_S(1,1)=clamp(svd.singularValues()(1), 1-ctrl_params.critical_compression, 1+ ctrl_params.critical_stretch);
		clamped_S(2,2)=clamp(svd.singularValues()(2), 1-ctrl_params.critical_compression, 1+ ctrl_params.critical_stretch);

		Matrix3f clamped_S_inv;
		clamped_S_inv.setZero();
		clamped_S_inv(0,0)=1/clamped_S(0,0);
		clamped_S_inv(1,1)=1/clamped_S(1,1);
		clamped_S_inv(2,2)=1/clamped_S(2,2);

		Eigen::Matrix3f U=svd.matrixU();
		Eigen::Matrix3f V=svd.matrixV();

		//fomular 12
		ptcl->Fe = U*clamped_S    *V.transpose();
		ptcl->Fp = V*clamped_S_inv*U.transpose()*F_new;
	}
}


void MpmCore::parallel_compute_deformation_gradient_F()
{
	tbb::parallel_for(tbb::blocked_range<int>(0, particles.size(), 2500), [&](tbb::blocked_range<int>& r)
	{
		for(int pit = r.begin(); pit != r.end(); pit++)
		{
			Particle* ptcl = particles[pit];
			Vector3f p_grid_index_f = ptcl->getGridIdx(grid->grid_min,grid->grid_size);
			Vector3i p_grid_index_i = ptcl->getGridIdx_int(grid->grid_min,grid->grid_size);

			Matrix3f velocity_gradient;
			velocity_gradient.setZero();

			for(int z=-2; z<=2;z++)
			{
				for(int y=-2; y<=2;y++)
				{
					for(int x=-2; x<=2;x++)
					{
						Vector3i index=p_grid_index_i+Vector3i(x,y,z);
						if(inGrid(index, grid->grid_division))
						{
							Vector3f xp=(ptcl->position - grid->grid_size.cwiseProduct(Vector3f(index[0],index[1],index[2]))-grid->grid_min).cwiseQuotient(grid->grid_size);
							Vector3f gradient_weight=weight_gradientF(xp);
							//fomular in step 4
							GridNode* node = grid->getNode(index[0], index[1], index[2]);
							velocity_gradient+=Eigen::Vector3f(node->velocity_new)*
								Eigen::Vector3f(gradient_weight).transpose();
						}
					}
				}
			}

			//fomular 11
			Eigen::Matrix3f Fe_new=(Eigen::Matrix3f::Identity()+velocity_gradient*ctrl_params.deltaT)*ptcl->Fe;
			Eigen::Matrix3f F_new=Fe_new*ptcl->Fp;
			Eigen::JacobiSVD<Eigen::Matrix3f> svd(Fe_new, Eigen::ComputeFullV | Eigen::ComputeFullU);

			Matrix3f clamped_S;
			clamped_S.setZero();
			clamped_S(0,0)=clamp(svd.singularValues()(0), 1-ctrl_params.critical_compression, 1+ ctrl_params.critical_stretch);
			clamped_S(1,1)=clamp(svd.singularValues()(1), 1-ctrl_params.critical_compression, 1+ ctrl_params.critical_stretch);
			clamped_S(2,2)=clamp(svd.singularValues()(2), 1-ctrl_params.critical_compression, 1+ ctrl_params.critical_stretch);

			Matrix3f clamped_S_inv;
			clamped_S_inv.setZero();
			clamped_S_inv(0,0)=1/clamped_S(0,0);
			clamped_S_inv(1,1)=1/clamped_S(1,1);
			clamped_S_inv(2,2)=1/clamped_S(2,2);

			Eigen::Matrix3f U=svd.matrixU();
			Eigen::Matrix3f V=svd.matrixV();

			//fomular 12
			ptcl->Fe = U*clamped_S    *V.transpose();
			ptcl->Fp = V*clamped_S_inv*U.transpose()*F_new;
		}

	});
}

void MpmCore::parallel_from_grid_to_particle()
{
	tbb::parallel_for(tbb::blocked_range<int>(0, particles.size(), 2500), [&](tbb::blocked_range<int>& r)
	{
		for(int pit = r.begin(); pit != r.end(); pit++)
		{
			Particle* ptcl = particles[pit];
			if(!ptcl->isValid)
				continue;
			//Vector3f p_grid_index_f=particles[pit]->getGridIdx(grid->grid_center,grid->grid_size);
			Vector3i p_grid_index_i= ptcl->getGridIdx_int(grid->grid_min,grid->grid_size);
			Vector3f v_PIC(0,0,0);
			Vector3f v_FLIP= ptcl->velocity;
			//Matrix3f cauchyStress=cauchy_stress(particles[pit]->Fe, particles[pit]->Fp)*particles[pit]->volume*-1;
			for(int z=-2; z<=2;z++)
			{
				for(int y=-2; y<=2;y++)
				{
					for(int x=-2; x<=2;x++)
					{
						Vector3i index=p_grid_index_i+Vector3i(x,y,z);
						if(inGrid(index, grid->grid_division))
						{
							GridNode* pCell = grid->getNode(index[0], index[1], index[2]);
							Vector3f xp=(ptcl->position-grid->grid_size.cwiseProduct(Vector3f(index[0],index[1],index[2]))-grid->grid_min).cwiseQuotient(grid->grid_size);
							float weight_p=weight(xp);
							v_PIC  += pCell->velocity_new*weight_p;
							v_FLIP += (pCell->velocity_new - pCell->velocity_old)*weight_p;
						}
					}
				}
			}
			ptcl->velocity=v_PIC*(1-ctrl_params.flip_percent)+v_FLIP*ctrl_params.flip_percent;
			if(pit==0)
				cout<<"vel:"<<ptcl->velocity[0]<<" "<<ptcl->velocity[1]<<" "<<ptcl->velocity[2]<<endl;
		}
	});
}

void MpmCore::from_grid_to_particle()
{
	for(int pit=0;pit<particles.size();pit++)
	{
		Particle* ptcl = particles[pit];
		if(!ptcl->isValid)
			continue;
		//Vector3f p_grid_index_f=particles[pit]->getGridIdx(grid->grid_center,grid->grid_size);
		Vector3i p_grid_index_i= ptcl->getGridIdx_int(grid->grid_min,grid->grid_size);
		Vector3f v_PIC(0,0,0);
		Vector3f v_FLIP= ptcl->velocity;
		//Matrix3f cauchyStress=cauchy_stress(particles[pit]->Fe, particles[pit]->Fp)*particles[pit]->volume*-1;
		for(int z=-2; z<=2;z++)
		{
			for(int y=-2; y<=2;y++)
			{
				for(int x=-2; x<=2;x++)
				{
					Vector3i index=p_grid_index_i+Vector3i(x,y,z);
					if(inGrid(index, grid->grid_division))
					{
						GridNode* pCell = grid->getNode(index[0], index[1], index[2]);// grid->grids[index[0]][index[1]][index[2]];
						Vector3f xp=(ptcl->position-grid->grid_size.cwiseProduct(Vector3f(index[0],index[1],index[2]))-grid->grid_min).cwiseQuotient(grid->grid_size);
						float weight_p=weight(xp);
						v_PIC  += pCell->velocity_new*weight_p;
						v_FLIP += (pCell->velocity_new - pCell->velocity_old)*weight_p;
					}
				}
			}
		}
		ptcl->velocity=v_PIC*(1-ctrl_params.flip_percent)+v_FLIP*ctrl_params.flip_percent;
		if(pit==0)
			cout<<"vel:"<<ptcl->velocity[0]<<" "<<ptcl->velocity[1]<<" "<<ptcl->velocity[2]<<endl;
	}
}

void MpmCore::solve_particle_collision()
{
	for(int pit=0;pit<particles.size();pit++)
	{
		if(!particles[pit]->isValid)
			continue;
		Vector3f p_grid_index_f=particles[pit]->getGridIdx(grid->grid_min,grid->grid_size);
		Vector3i p_grid_index_i=particles[pit]->getGridIdx_int(grid->grid_min,grid->grid_size);
		Vector3f p_grid_index_i_new=(particles[pit]->position+particles[pit]->velocity*ctrl_params.deltaT-grid->grid_min).cwiseQuotient(grid->grid_size);
		Vector3i index_check_1=p_grid_index_i-Vector3i(1,1,1);
		Vector3i index_check_2=p_grid_index_i+Vector3i(1,1,1);
		if(!inGrid(p_grid_index_i,grid->grid_division)||!inGrid(index_check_1,grid->grid_division)||!inGrid(index_check_2,grid->grid_division))
		{
			particles[pit]->isValid=false;
			continue;
		}

		float sdf=0;
		Vector3f sdf_normal;
		Vector3f velocity_collider(0,0,0);
		for(int z=p_grid_index_i.z(); z<=p_grid_index_i.z()+1;z++)
		{
			for(int y=p_grid_index_i.y(); y<=p_grid_index_i.y()+1;y++)
			{
				for(int x=p_grid_index_i.x(); x<=p_grid_index_i.x()+1;x++)
				{
					float weight_sdf=fabs( ( p_grid_index_f.z()-z)*
												   ( p_grid_index_f.y()-y)*
												   ( p_grid_index_f.x()-x) );
					Vector3i index(x,y,z);
					Vector3i index_check_1=Vector3i(x-1,y-1,z-1);
					Vector3i index_check_2=Vector3i(x+1,y+1,z+1);
					if(inGrid(index, grid->grid_division)&&inGrid(index_check_1,grid->grid_division)&&inGrid(index_check_2,grid->grid_division))
					{
						Vector3f temp_normal;
						getSDFNormal(index,temp_normal);
						sdf_normal+=temp_normal*weight_sdf;
						GridNode* cell = grid->getNode(index[0], index[1], index[2]);
						sdf+=cell->collision_sdf*weight_sdf;
						velocity_collider+= cell->collision_velocity*weight_sdf;
					}
				}
			}
		}


		//if(getSDFNormal_box(p_grid_index_i_new,sdf_normal))
		if(sdf>0)
		{
			Vector3f updated_v;
			sdf_normal.normalize();
			if( updateVelocityWithSolvingCollision( velocity_collider, particles[pit]->velocity, sdf_normal, updated_v) )
			{
				particles[pit]->velocity=updated_v;
			}
		}
	}
}

void MpmCore::update_position()
{
	for(int pit=0;pit<particles.size();pit++)
		particles[pit]->position+=particles[pit]->velocity*ctrl_params.deltaT;
}

bool MpmCore::for_each_frame(int ithFrame, float deltaTime, int nSubstep)
{
	// 更新粒子位置
	int start = m_recorder.getStartFrame();
	MpmStatus status;
	bool res = m_recorder.getStatus(ithFrame, status);
	if (res)
	{
		res = status.copy(particles);
	}
	if (!res)
	{
		return false;
	}

	clock_t t0 = clock();
	clock_t t1 = t0;
	int msArray[8] = {0,0,0,0, 0,0,0,0};
	ctrl_params.deltaT = deltaTime / nSubstep;
	for (int ithStep = 0; ithStep < nSubstep; ++ithStep)
	{
		if(ctrl_params.frame!=0)
		{
			//from_particles_to_grid();
			parallel_from_particles_to_grid();
			t1 = clock();
			msArray[0] += t1 - t0;
			t0 = t1;
		}

		if(ctrl_params.frame==0)
		{
			init_particle_volume_velocity();
			t1 = clock();
			msArray[1] += t1 - t0;
			t0 = t1;
		}

		compute_grid_velocity();
		t1 = clock();
		msArray[2] += t1 - t0;
		t0 = t1;

		solve_grid_collision();
		t1 = clock();
		msArray[3] += t1 - t0;
		t0 = t1;

		parallel_compute_deformation_gradient_F();
		t1 = clock();
		msArray[4] += t1 - t0;
		t0 = t1;

		parallel_from_grid_to_particle();
		t1 = clock();
		msArray[5] += t1 - t0;
		t0 = t1;

		solve_particle_collision();
		t1 = clock();
		msArray[6] += t1 - t0;
		t0 = t1;

		update_position();
		t1 = clock();
		msArray[7] += t1 - t0;
		t0 = t1;
	}

	PRINT_F("from_particles_to_grid:		%f", float(msArray[0])/CLOCKS_PER_SEC);
	PRINT_F("init_particle_volume_velocity:	%f", float(msArray[1])/CLOCKS_PER_SEC);
	PRINT_F("compute_grid_velocity:			%f", float(msArray[2])/CLOCKS_PER_SEC);
	PRINT_F("solve_grid_collision:			%f", float(msArray[3])/CLOCKS_PER_SEC);
	PRINT_F("compute_deformation_gradient:	%f", float(msArray[4])/CLOCKS_PER_SEC);
	PRINT_F("from_grid_to_particle:			%f", float(msArray[5])/CLOCKS_PER_SEC);
	PRINT_F("solve_particle_collision:		%f", float(msArray[6])/CLOCKS_PER_SEC);
	PRINT_F("update_position:				%f", float(msArray[7])/CLOCKS_PER_SEC);
	ctrl_params.frame++;

	// 将模拟结果记下来
	m_recorder.addStatus(ithFrame+1, MpmStatus(particles));
	return true;
}

void MpmCore::createGrid(const Vector3f& gridMin,
						 const Vector3f& gridMax,
						 const Vector3f& gridCellSize,
						 int boundary)
{
	PRINT_F("boundary %d", boundary);
	Vector3i gridDimClamp = (gridMax-gridMin).cwiseQuotient(gridCellSize).cwiseMax(Vector3f(5,5,5)).cast<int>();
	Vector3f cellSize = (gridMax - gridMin).cwiseQuotient(gridDimClamp.cast<float>());
	
	grid = new GridField(cellSize, gridMin, gridDimClamp, boundary);
	const int bId[3][2] = {
		{boundary, gridDimClamp[0]-1-boundary},
		{boundary, gridDimClamp[1]-1-boundary},
		{boundary, gridDimClamp[2]-1-boundary},
	};
	float depth[3];
	for (int i = 0; i < gridDimClamp[0]; ++i)
	{
		depth[0] = min(i-bId[0][0],bId[0][1]-i) * cellSize[0];

		for (int j = 0; j < gridDimClamp[1]; ++j)
		{
			depth[1] = min(j-bId[1][0],bId[1][1]-j) * cellSize[1];

			for (int k =0; k < gridDimClamp[2]; ++k)
			{
				depth[2] = min(k-bId[2][0],bId[2][1]-k) * cellSize[2];

				float d = min(min(depth[0], depth[1]), depth[2]);
				if (d > 0)
				{
					continue;
				}

				grid->getNode(i,j,k)->collision_sdf= d * -1;
				//PRINT_F("depth %d %d %d,  %f", i,j,k,grid->grids[i][j][k]->collision_sdf);
			}
		}
	}
}

void MpmCore::create_grid()
{
	Vector3f min(-1.7, -1.38, -1.7);
	Vector3i dimensions(201, 201, 201);
	float radius=0.017f;
	Vector3f size(radius, radius, radius);

	grid=new GridField(size, min, dimensions,10);

	//set sdf for box
	for(int i=-100;i<=100;i++)
		for(int j=-100;j<=100;j++)
			for(int k=-100;k<100;k++)
			{
				float dis=( 90 - max( max(abs(i), abs(j)), abs(k) ) )*radius;
				grid->grids[i+100][j+100][k+100]->collision_sdf=-dis;
			}
}

void MpmCore::addBall(const Vector3f& center, float radius, int nParticlePerCell, int ithFrame)
{
	float cellVolume = grid->grid_size[0] * grid->grid_size[1] * grid->grid_size[2];
	float pmass= ctrl_params.particleDensity * cellVolume / nParticlePerCell;
	Vector3f init_velocity(-1.0f, -1.0f, 0);
	Vector3i gridDim(grid->grid_division[0],grid->grid_division[1],grid->grid_division[2]);
	for (int i =0; i < gridDim[0]; ++i)
	{
		for (int j = 0; j < gridDim[1]; ++j)
		{
			for (int k = 0; k < gridDim[2]; ++k)
			{
				Vector3f pos = grid->grid_min + grid->grid_size.cwiseProduct(Vector3f(i,j,k));
				if ((pos - center).squaredNorm() <= radius * radius)
				{
					for (int ithP = 0; ithP < nParticlePerCell; ++ithP)
					{
						Vector3f jitter(rand(), rand(), rand());
						jitter /= float(RAND_MAX);
						jitter = jitter.cwiseProduct(grid->grid_size);
						Vector3f posJittered = pos + jitter;
						particles.push_back(new Particle(particles.size(), posJittered, init_velocity, pmass));
					}
				}
			}
		}
	}
	
}

void MpmCore::create_snow_ball()
{
	Vector3f snow_ball_center(0.f, 1.f, 0.f);
	Vector3i snow_ball_dimensions(40, 40, 40);
	float pmass=0.0001;
	Vector3f init_velocity(-100.0f, -100.0f, 0);
	float radius=0.017f;

	particles.clear();

	int pid=0;
	float sphereRadius = radius * (float)snow_ball_dimensions[0] / 2.0f;
	for (int x = -snow_ball_dimensions[0]/2; x <= snow_ball_dimensions[0]/2; x++) 
	{
		for (int y = -snow_ball_dimensions[1]/2; y <= snow_ball_dimensions[1]/2; y++) 
		{
			for (int z = -snow_ball_dimensions[2]/2; z <= snow_ball_dimensions[2]/2; z++) 
			{
				// generate a jittered point
				float r1 = 0.001f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				float r2 = 0.001f + static_cast <float>(rand()) / static_cast <float> (RAND_MAX);
				float r3 = 0.001f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				Vector3f jitter = Vector3f(r1, r2, r3) * radius;

				Vector3f pos = snow_ball_center + Vector3f(float(x), float(y), float(z)) * radius + jitter;
				// see if pos is inside the sphere
				if ((pos - snow_ball_center).norm() < sphereRadius) 
				{
					particles.push_back(new Particle(pid,pos, init_velocity, pmass));
					pid++;
				}
			}
		}
	}
}

void MpmCore::addTwoBalls(int nParticlePerCell)
{
	float cellVolume = grid->grid_size[0] * grid->grid_size[1] * grid->grid_size[2];
	float pmass= ctrl_params.particleDensity * cellVolume / nParticlePerCell;
	Vector3f init_velocity(-100.0f, -100.0f, 0);
	Vector3f snow_ball_center_1(0.5f, 1.0f, 0.f);
	Vector3f snow_ball_center_2(-0.5f, 1.0f, 0.f);
	Vector3i snow_ball_dimensions(40, 40, 40);
	Vector3f init_velocity_1(-10.f, 0.0f, 0);
	Vector3f init_velocity_2(10.f, 0.0f, 0);
	float radius=0.017f;

	particles.clear();

	int pid=0;
	float sphereRadius = radius * (float)snow_ball_dimensions.x() / 2.0f;
	for (int x = -snow_ball_dimensions.x()/2; x <= snow_ball_dimensions.x()/2; x++) 
	{
		for (int y = -snow_ball_dimensions.y()/2; y <= snow_ball_dimensions.y()/2; y++) 
		{
			for (int z = -snow_ball_dimensions.z()/2; z <= snow_ball_dimensions.z()/2; z++) 
			{
				// generate a jittered point
				float r1 = 0.001f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				float r2 = 0.001f + static_cast <float>(rand()) / static_cast <float> (RAND_MAX);
				float r3 = 0.001f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				Vector3f jitter = Vector3f(r1, r2, r3) * radius;

				Vector3f pos = snow_ball_center_1 + Vector3f(float(x), float(y), float(z)) * radius + jitter;
				// see if pos is inside the sphere
				if ((pos - snow_ball_center_1).norm() < sphereRadius) 
				{
					particles.push_back(new Particle(pid,pos, init_velocity_1, pmass));
					pid++;
				}
			}
		}
	}

	for (int x = -snow_ball_dimensions.x()/2; x <= snow_ball_dimensions.x()/2; x++) 
	{
		for (int y = -snow_ball_dimensions.y()/2; y <= snow_ball_dimensions.y()/2; y++) 
		{
			for (int z = -snow_ball_dimensions.z()/2; z <= snow_ball_dimensions.z()/2; z++) 
			{
				// generate a jittered point
				float r1 = 0.001f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				float r2 = 0.001f + static_cast <float>(rand()) / static_cast <float> (RAND_MAX);
				float r3 = 0.001f + static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				Vector3f jitter = Vector3f(r1, r2, r3) * radius;

				Vector3f pos = snow_ball_center_2 + Vector3f(float(x), float(y), float(z)) * radius + jitter;
				// see if pos is inside the sphere
				if ((pos - snow_ball_center_2).norm() < sphereRadius) 
				{
					particles.push_back(new Particle(pid,pos, init_velocity_2, pmass));
					pid++;
				}
			}
		}
	}
}

bool MpmCore::initGrid(const Vector3f& gridMin,
				   const Vector3f& gridMax,
				   const Vector3f& gridCellSize,
				   int gridBoundary,
				   int ithFrame)
{
	PRINT_F("particletemp size: %d  vector3f size %d", sizeof(ParticleTemp), sizeof(Vector3f));
	clear();

	ctrl_params.setting_1();

	//create_snow_ball_2();
	//create_grid();

	createGrid(gridMin, gridMax, gridCellSize, gridBoundary);
	
	return true;
}


void MpmCore::getGridConfig( Vector3f& minPnt, Vector3f& cellSize, Vector3i& cellNum )
{
	if (!grid)
	{
		minPnt.setZero();
		cellSize.setZero();
		cellNum.setZero();
	}
	else
	{
		minPnt= grid->grid_min;
		cellSize = grid->grid_size;
		cellNum = Vector3i(grid->grid_division[0], grid->grid_division[1], grid->grid_division[2]);
	}
}

const vector<Particle*>& MpmCore::getParticle()
{
	return particles;
}

MpmCore::MpmCore()
{
	grid = NULL;
}

MpmCore::~MpmCore()
{
	clear();
}

void MpmCore::clear()
{
	if (grid)
	{
		delete grid;
		grid = NULL;
	}
	for (int i = 0; i < particles.size(); ++i)
	{
		if (particles[i])
		{
			delete particles[i];
		}
	}
	particles.clear();
	m_particleTemp.clear();
}

StatusRecorder& MpmCore::getRecorder()
{
	return m_recorder;
}


void MpmCore::setConfigure(float young,
						   float possion,
						   float hardening,
						   float criticalComp,
						   float criticalStretch,
						   float friction,
						   float flipPercent,
						   float deltaT,
						   float particleDensity,
						   const Vector3f& gravity)
{
	ctrl_params.init_youngs_modulus = young;
	ctrl_params.poissons_ratio = possion;
	ctrl_params.hardening = hardening;
	ctrl_params.critical_compression = criticalComp;
	ctrl_params.critical_stretch = criticalStretch;
	ctrl_params.frictionCoeff = friction;
	ctrl_params.flip_percent = flipPercent;
	ctrl_params.deltaT = deltaT;
	ctrl_params.frame = 0;
	ctrl_params.gravity = gravity;
	ctrl_params.particleDensity = particleDensity;
	ctrl_params.initLame();
	// youngs modulus:	54701.988281 // 
	// poissons ratio:	0.187086 // 
	// Lame coef mu:	23040.447266 // 
	// Lame coef lambda:	13775.505859 // 
	// hardening factor:	14.086093 // 
	// critical compression:	0.023291 // 
	// critical stretch:	0.007258 // 
	// friction coef:	1.268212 // 
	// flip percent:	0.869536 // 
	// time step:	0.037417 // 
	// gravity:	(0.000000,-9.800000,0.000000) // 

	PRINT_F("================== physical parameters ==================");
	PRINT_F("youngs modulus:\t\t%f", ctrl_params.init_youngs_modulus);
	PRINT_F("poissons ratio:\t\t%f", ctrl_params.poissons_ratio);
	PRINT_F("Lame coef mu:\t\t%f", ctrl_params.miu);
	PRINT_F("Lame coef lambda:\t%f", ctrl_params.lambda);
	PRINT_F("hardening factor:\t%f", ctrl_params.hardening);
	PRINT_F("critical compression:%f", ctrl_params.critical_compression);
	PRINT_F("critical stretch:\t%f", ctrl_params.critical_stretch);
	PRINT_F("friction coef:\t\t%f", ctrl_params.frictionCoeff);
	PRINT_F("flip percent:\t\t%f", ctrl_params.flip_percent);
	PRINT_F("time step:\t\t\t%f", ctrl_params.deltaT);
	PRINT_F("gravity:\t\t\t\t(%f,%f,%f)", ctrl_params.gravity[0],ctrl_params.gravity[1],ctrl_params.gravity[2]);
	PRINT_F("=========================================================");

}

bool MpmCore::commitInit( int ithFrame )
{
	m_recorder.init(ithFrame);
	m_recorder.addStatus(ithFrame, MpmStatus(particles));
	return false;
}

void control_parameters::setting_1()
{
	particleDensity = 1000;
	deltaT=5e-4f;
	frame=0;

	init_youngs_modulus=4.8e4f;
	poissons_ratio=0.2f;
	hardening=15.0f;
	critical_compression=0.019f;
	critical_stretch=0.0075f;
	frictionCoeff=1.f;

	//lame coeffients
	miu=init_youngs_modulus/(2*(1+poissons_ratio));
	lambda=(poissons_ratio*init_youngs_modulus)/((1+poissons_ratio)*(1-2*poissons_ratio));

	flip_percent=0.95f;

	gravity=Vector3f(0, -9.8f, 0);
}

void control_parameters::initLame()
{
	miu=init_youngs_modulus/(2*(1+poissons_ratio));
	lambda=(poissons_ratio*init_youngs_modulus)/((1+poissons_ratio)*(1-2*poissons_ratio));
}

GridNode::GridNode()
{
	mass=0;
	velocity_new = Vector3f(0.f,0.f,0.f);
	velocity_old = Vector3f(0.f,0.f,0.f);
	external_force = Vector3f(0.f,0.f,0.f);
	collision_velocity = Vector3f(0.f,0.f,0.f);
	collision_sdf=0;

	active=false;
}

