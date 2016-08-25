#pragma once

using namespace std;




struct GridNode
{
	float mass;
	Vector3f velocity_new;
	Vector3f velocity_old;
	//float density;
	float collision_sdf;
	Vector3f collision_velocity;
	Vector3f external_force;
	bool active;

	GridNode();
};
struct GridField
{
	boost::multi_array<GridNode*,3> grids;
	Vector3f grid_size;//interval grid_dims
	Vector3f grid_min, grid_max;
	Vector3i grid_division;//int
	int		 boundary;

	GridField();
	~GridField();

	void clear();

	GridField(const Vector3f& grid_size, const Vector3f& grid_min, const Vector3i& grid_division, int boundary);
};

struct control_parameters
{
	float particleDensity;

	//ctrl param
	float init_youngs_modulus;
	float poissons_ratio;
	float hardening;
	float critical_compression;
	float critical_stretch;
	//float init_density;

	//lame coefficient. computed from init_youngs_modulus and poissons_ratio
	float miu;
	float lambda;

	//for PIC and FLIP interplote
	float flip_percent;

	//float pmass;
	//float CFL;
	float frictionCoeff;

	//float division_size;
	//float max_velocity;

	Vector3f gravity;
	//Vector3f box_min;
	//Vector3f box_max;

	//time interval
	float deltaT;
	int frame;

	void setting_1();
	void initLame();
};

class MpmCore
{
public:
	MpmCore();
	~MpmCore();

	bool	init(	const Vector3f& gridMin,
					const Vector3f& gridMax,
					const Vector3f& gridCellSize,
					int gridBoundary = 2,
					int ithFrame = 0);

	void	setConfigure(float young, float possion, float hardening, float criticalComp, float criticalStretch, float friction, float flipPercent, float deltaT, float particleDensity, const Vector3f& gravity);
	void	createBall(const Vector3f& center, float radius, int nParticlePerCell, int ithFrame);

	bool	for_each_frame(int ithFrame);

	const vector<Particle*>& getParticle();
	void	getGridConfig(Vector3f& minPnt, Vector3f& cellSize, Vector3i& cellNum);

	template<typename GridType, typename GridPtrType> 
	bool	addParticleGrid(typename GridPtrType& pGrid, int nParticlePerCell = 1)
	{
		float cellVolume = grid->grid_size[0] * grid->grid_size[1] * grid->grid_size[2];
		float pmass= ctrl_params.particleDensity * cellVolume / nParticlePerCell;
		Vector3f init_velocity(-100.0f, -100.0f, 0);
		GridType::ConstAccessor acc = openvdb::gridConstPtrCast<GridType>(pGrid)->getAccessor();
		openvdb::tools::GridSampler<GridType::ConstAccessor, openvdb::tools::BoxSampler>
			interpolator(acc, pGrid->transform());
		for (int i = 0; i < grid->grid_division[0]; ++i)
		{
			for (int j = 0; j < grid->grid_division[1]; ++j)
			{
				for (int k = 0; k < grid->grid_division[2]; ++k)
				{
					for (int p = 0; p < nParticlePerCell; ++p)
					{
						Vector3f jitter(rand(), rand(), rand());
						jitter = jitter.cwiseProduct(grid->grid_size) / float(RAND_MAX);
						Vector3f pos = grid->grid_min + grid->grid_size.cwiseProduct(Vector3f(i,j,k)) + jitter;
 						GridType::ValueType val = interpolator.wsSample(openvdb::Vec3d(pos[0],pos[1],pos[2]));
						//PRINT_F("v %d %d %d    %f", i,j,k, val);
						if (val < 0)
						
							particles.push_back(new Particle(particles.size(), pos, init_velocity, pmass));
					}
				}
			}
		}
		return false;
	}

	StatusRecorder&		getRecorder();
private:
	control_parameters	ctrl_params;
	vector<Particle*>	particles;
	StatusRecorder      m_recorder;

	GridField*			grid;

	bool inGrid(Vector3i& index, Vector3i& grid_division);

	void clear();

	float NX_bspline(float m);

	float dNX_bsplineslope(float m);

	float weight(Vector3f& d_xp);

	Vector3f weight_gradientF(Vector3f& d_xp);

	Matrix3f cauchy_stress(Matrix3f& Fe, Matrix3f& Fp, float particle_volume);

	//for 1st frame only. Compute particles volume and velocity
	void init_particle_volume_velocity();

	//step 1 transfer mass and velocity to grid. And compute force of grid according to (6)
	void from_particles_to_grid();

	//step 3 4 compute grid force and update grid velocity
	void compute_grid_velocity();

	bool getSDFNormal(Vector3i& grid_idx, Vector3f& out_sdf_normal);

	bool getSDFNormal_box(Vector3f& grid_idx_new, Vector3f& out_sdf_normal);

	bool updateVelocityWithSolvingCollision(Vector3f& collider_velocity, Vector3f& grid_velocity, Vector3f& sdf_normal, Vector3f& out_velocity);

	//step 5 handle grid collision
	void solve_grid_collision();

	float clamp(float value, float low, float high);

	//step 7 update deformation gradient
	void compute_deformation_gradient_F();

	//step 8 transfer velocity to particle
	void from_grid_to_particle();

	//step 9 handle particle collision
	void solve_particle_collision();

	//step 10 update particle position
	void update_position();

	// init functions
	void create_grid();
	void createGrid(const Vector3f& gridMin, const Vector3f& gridMax, const Vector3f& gridCellSize, int boundary = 2);

	void create_snow_ball();
	void create_snow_ball_2();
};