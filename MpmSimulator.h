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
};

