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
	MBoundingBox		m_box;
	MpmCore				m_core;

	static MObject		s_boxMin;			// 
	static MObject		s_boxMax;
	static MObject		s_cellSize;
	static MObject		s_nParticlePerCell;
	static MObject		s_boundary;

	static const char*  s_boxMinName[2];
	static const char*	s_boxMaxName[2];
	static const char*  s_cellSizeName[2];
	static const char*	s_nParticlePerCellName[2];
	static const char*  s_boundaryName[2];
};

