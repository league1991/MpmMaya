#pragma once

class MpmSimulator: public MPxLocatorNode
{
public:
	MpmSimulator(void);
	~MpmSimulator(void);
	void				postConstructor();

	virtual void		draw( M3dView & view, const MDagPath & path, M3dView::DisplayStyle style, M3dView:: DisplayStatus );
	virtual bool		isBounded() const{return true;}
	virtual MBoundingBox boundingBox() const;

	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );
	static  void*		creator();
	static  MStatus		initialize();

	static MTypeId		m_id;
	static const char*  m_nodeName;

	bool				initSolver();
	bool				stepSolver();

private:
	void				drawIcon();
	void				drawCell();
	int					getCurFrame();
	bool				initParticle();
	MBoundingBox		m_box;
	MpmCore				m_core;

	static MObject		s_boxMin;			// 
	static MObject		s_boxMax;
	static MObject		s_cellSize;
	static MObject		s_nParticlePerCell;
	static MObject		s_boundary;

	// physical param
	static MObject		s_particleMass;
	static MObject		s_youngs;
	static MObject		s_poisson;
	static MObject		s_hardening;
	static MObject		s_criticalCompression;
	static MObject		s_criticalStretch;
	static MObject		s_friction;
	static MObject		s_flip;
	static MObject		s_gravity;
	static MObject		s_deltaT;

	// input
	static MObject		s_initParticle;

	static const char*  s_boxMinName[2];
	static const char*	s_boxMaxName[2];
	static const char*  s_cellSizeName[2];
	static const char*	s_nParticlePerCellName[2];
	static const char*  s_boundaryName[2];

	static const char*  s_particleMassName[2];
	static const char*	s_youngsName[2];
	static const char*	s_poissonName[2];
	static const char*	s_hardeningName[2];
	static const char*	s_criticalCompressionName[2];
	static const char*	s_criticalStretchName[2];
	static const char*	s_frictionName[2];
	static const char*	s_flipName[2];
	static const char*	s_gravityName[2];
	static const char*	s_deltaTName[2];

	static const char*	s_initParticleName[2];
};

