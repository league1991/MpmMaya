#include "stdafx.h"
#include "MpmSimulator.h"

const char* MpmSimulator::m_nodeName = "MpmSimulator";
MTypeId MpmSimulator::m_id(0x00108A60);


MObject MpmSimulator::s_boxMin;
MObject MpmSimulator::s_boxMax;
MObject	MpmSimulator::s_cellSize;
MObject MpmSimulator::s_nParticlePerCell;
MObject	MpmSimulator::s_boundary;

MObject	MpmSimulator::s_particleDensity;
MObject	MpmSimulator::s_youngs;
MObject	MpmSimulator::s_poisson;
MObject	MpmSimulator::s_hardening;
MObject	MpmSimulator::s_criticalCompression;
MObject	MpmSimulator::s_criticalStretch;
MObject	MpmSimulator::s_friction;
MObject	MpmSimulator::s_flip;
MObject	MpmSimulator::s_gravity;
MObject	MpmSimulator::s_deltaT;
MObject MpmSimulator::s_initParticle;

const char* MpmSimulator::s_boxMinName[2]={"boxMin","bMin"};
const char* MpmSimulator::s_boxMaxName[2]={"boxMax","bMax"};
const char* MpmSimulator::s_cellSizeName[2]={"cellSize","cSize"};
const char* MpmSimulator::s_nParticlePerCellName[2]={"particlePerCell", "ppc"};
const char* MpmSimulator::s_boundaryName[2]={"boundary","bnd"};

const char*	MpmSimulator::s_particleDensityName[2]={"particleDensity","pdensity"};
const char*	MpmSimulator::s_youngsName[2]={"youngsModulus","ym"};
const char*	MpmSimulator::s_poissonName[2]={"possionRatio","pr"};
const char*	MpmSimulator::s_hardeningName[2]={"hardeningFactor","hf"};
const char*	MpmSimulator::s_criticalCompressionName[2]={"criticalCompression","cc"};
const char*	MpmSimulator::s_criticalStretchName[2]={"criticalStretch","cs"};
const char*	MpmSimulator::s_frictionName[2]={"friction","fr"};
const char*	MpmSimulator::s_flipName[2]={"flipPercent","fp"};
const char*	MpmSimulator::s_gravityName[2]={"gravity","gr"};
const char*	MpmSimulator::s_deltaTName[2]={"deltaTime","dt"};
const char* MpmSimulator::s_initParticleName[2]={"initParticle","iptl"};

MpmSimulator::MpmSimulator(void)
{
	m_box = MBoundingBox(MPoint(-1.1,-0.5,-1.1), MPoint(4.1,0.5,1.1));
}


MpmSimulator::~MpmSimulator(void)
{
}

int MpmSimulator::getCurFrame()
{
	int curFrame = 1;
	MGlobal::executeCommand("currentTime -q", curFrame);
	return curFrame;
}

void MpmSimulator::draw( M3dView & view, const MDagPath & path, M3dView::DisplayStyle style, M3dView:: DisplayStatus )
{
	view.beginGL();
	glPushAttrib(GL_CURRENT_BIT);

	drawIcon();

	// 保证在世界空间模拟，所以需要抵消自身矩阵
	MMatrix mat  = path.inclusiveMatrixInverse();
	double  matBuf[4][4];
	mat.get(matBuf);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMultMatrixd(&matBuf[0][0]);
	{
		drawCell();

		m_box = MBoundingBox(MPoint(-1.1,-0.5,-1.1), MPoint(4.1,0.5,1.1));

		MpmStatus status;
		if(m_core.getRecorder().getStatus(getCurFrame(), status))
		{
			status.draw();
		}
		

		Vector3f minPnt, cellSize;
		Vector3i nCell;
		m_core.getGridConfig(minPnt, cellSize, nCell);
		Vector3f maxPnt = minPnt + cellSize.cwiseProduct(nCell.cast<float>());

		MPoint boxMin = MPoint(minPnt[0],minPnt[1],minPnt[2])*mat;
		MPoint boxMax = MPoint(maxPnt[0],maxPnt[1],maxPnt[2])*mat;
		m_box = MBoundingBox(MPoint(-1.1,-0.5,-1.1), MPoint(4.1,0.5,1.1));
		m_box.expand(boxMin);
		m_box.expand(boxMax);
	}

	glPopAttrib();

	view.endGL();
}

void MpmSimulator::postConstructor()
{
	MStatus s;
	MFnDependencyNode nodeFn(thisMObject(), &s);

	if (s)
	{
		nodeFn.setName( "MpmSimulatorShape#", &s);
	}
}

void MpmSimulator::drawCell()
{
	Vector3f minPnt, cellSize;
	Vector3i nCell;
	m_core.getGridConfig(minPnt, cellSize, nCell);

	Vector3f maxPnt = minPnt + cellSize.cwiseProduct(nCell.cast<float>());

	glBegin(GL_LINE_LOOP);
	glVertex3f(minPnt[0],minPnt[1],minPnt[2]);
	glVertex3f(maxPnt[0],minPnt[1],minPnt[2]);
	glVertex3f(maxPnt[0],maxPnt[1],minPnt[2]);
	glVertex3f(minPnt[0],maxPnt[1],minPnt[2]);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(minPnt[0],minPnt[1],maxPnt[2]);
	glVertex3f(maxPnt[0],minPnt[1],maxPnt[2]);
	glVertex3f(maxPnt[0],maxPnt[1],maxPnt[2]);
	glVertex3f(minPnt[0],maxPnt[1],maxPnt[2]);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(minPnt[0],minPnt[1],minPnt[2]);
	glVertex3f(minPnt[0],minPnt[1],maxPnt[2]);

	glVertex3f(maxPnt[0],minPnt[1],minPnt[2]);
	glVertex3f(maxPnt[0],minPnt[1],maxPnt[2]);

	glVertex3f(minPnt[0],maxPnt[1],minPnt[2]);
	glVertex3f(minPnt[0],maxPnt[1],maxPnt[2]);

	glVertex3f(maxPnt[0],maxPnt[1],minPnt[2]);
	glVertex3f(maxPnt[0],maxPnt[1],maxPnt[2]);
	glEnd();

	glBegin(GL_LINES);
	for (int i = 0; i < nCell[0]; ++i)
	{
		glVertex3f(minPnt[0]+i*cellSize[0], minPnt[1], minPnt[2]);
		glVertex3f(minPnt[0]+i*cellSize[0], minPnt[1], maxPnt[2]);
	}
	for (int i = 0; i < nCell[2]; ++i)
	{
		glVertex3f(minPnt[0], minPnt[1], minPnt[2]+i*cellSize[2]);
		glVertex3f(maxPnt[0], minPnt[1], minPnt[2]+i*cellSize[2]);
	}
	glEnd();
}

void MpmSimulator::drawIcon()
{
	float p[][3] = {{0.5,0,-1},{0.8,0,1},{-1,0,1},{0.2,0,0.4}};

	glBegin(GL_LINE_LOOP);
	glVertex3fv(p[0]);
	glVertex3fv(p[1]);
	glVertex3fv(p[2]);
	glEnd();

	glBegin(GL_LINES);
	glVertex3fv(p[0]);
	glVertex3fv(p[3]);

	glVertex3fv(p[1]);
	glVertex3fv(p[3]);

	glVertex3fv(p[2]);
	glVertex3fv(p[3]); 
	glEnd();

	// draw "MPM"
	glBegin(GL_LINE_STRIP);
	glVertex3f(1,0,1);
	glVertex3f(1,0,0);
	glVertex3f(1.4,0,0.7);
	glVertex3f(1.8,0,0);
	glVertex3f(1.8,0,1);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f(2,0,0.5);
	glVertex3f(2.8,0,0.5);
	glVertex3f(2.8,0,0);
	glVertex3f(2,0,0);
	glVertex3f(2,0,1);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3f(3,0,1);
	glVertex3f(3,0,0);
	glVertex3f(3.4,0,0.7);
	glVertex3f(3.8,0,0);
	glVertex3f(3.8,0,1);
	glEnd();
}
MBoundingBox MpmSimulator::boundingBox() const
{
	return m_box;
}

MStatus MpmSimulator::compute( const MPlug& plug, MDataBlock& data )
{
	MStatus s;
	return s;
}

void* MpmSimulator::creator()
{
	return new MpmSimulator;
}

MStatus MpmSimulator::initialize()
{
	MStatus s;
	MFnNumericAttribute nAttr;
	MFnMatrixAttribute  mAttr;
	MFnTypedAttribute   tAttr;
	MFnEnumAttribute	eAttr;
	MFnUnitAttribute	uAttr;
	MFnCompoundAttribute cAttr;

	{
		s_boxMin = nAttr.create(s_boxMinName[0], s_boxMinName[1], MFnNumericData::k3Float, -2, &s);
		nAttr.setStorable(true);
		nAttr.setWritable(true);
		s = addAttribute(s_boxMin);
		CHECK_MSTATUS_AND_RETURN_IT(s);

		s_boxMax = nAttr.create(s_boxMaxName[0], s_boxMaxName[1], MFnNumericData::k3Float, 2, &s);
		nAttr.setStorable(true);
		nAttr.setWritable(true);
		s = addAttribute(s_boxMax);
		CHECK_MSTATUS_AND_RETURN_IT(s);

		s_cellSize = nAttr.create(s_cellSizeName[0], s_cellSizeName[1], MFnNumericData::k3Float, 0.1, &s);
		nAttr.setStorable(true);
		nAttr.setWritable(true);
		nAttr.setMin(0.01);
		s = addAttribute(s_cellSize);
		CHECK_MSTATUS_AND_RETURN_IT(s);

		s_nParticlePerCell = nAttr.create(s_nParticlePerCellName[0], s_nParticlePerCellName[1], MFnNumericData::kShort, 2, &s);
		nAttr.setStorable(true);
		nAttr.setWritable(true);
		nAttr.setMin(1);
		nAttr.setMax(20);
		s = addAttribute(s_nParticlePerCell);
		CHECK_MSTATUS_AND_RETURN_IT(s);

		s_boundary = nAttr.create(s_boundaryName[0], s_boundaryName[1], MFnNumericData::kShort, 2, &s);
		nAttr.setStorable(true);
		nAttr.setWritable(true);
		nAttr.setMin(0);
		nAttr.setMax(10);
		s = addAttribute(s_boundary);
		CHECK_MSTATUS_AND_RETURN_IT(s);
	}

	{
		s_particleDensity = nAttr.create(s_particleDensityName[0], s_particleDensityName[1], MFnNumericData::kFloat, 1000, &s);
		nAttr.setMin(1e-5);
		nAttr.setMax(1e12);
		nAttr.setSoftMin(10);
		nAttr.setSoftMax(10000);
		s = addAttribute(s_particleDensity);
		CHECK_MSTATUS_AND_RETURN_IT(s);

		s_youngs = nAttr.create(s_youngsName[0], s_youngsName[1], MFnNumericData::kFloat, 4.8e4, &s);
		nAttr.setMin(1e-5);
		nAttr.setMax(1e12);
		nAttr.setSoftMin(1e4);
		nAttr.setSoftMax(10e4);
		s = addAttribute(s_youngs);
		CHECK_MSTATUS_AND_RETURN_IT(s);

		s_poisson = nAttr.create(s_poissonName[0], s_poissonName[1], MFnNumericData::kFloat, 0.2, &s);
		nAttr.setMin(1e-3);
		nAttr.setMax(1e2);
		nAttr.setSoftMin(0.05);
		nAttr.setSoftMax(0.5);
		s = addAttribute(s_poisson);
		CHECK_MSTATUS_AND_RETURN_IT(s);

		s_hardening = nAttr.create(s_hardeningName[0], s_hardeningName[1], MFnNumericData::kFloat, 15.f, &s);
		nAttr.setMin(0);
		nAttr.setMax(1e4);
		nAttr.setSoftMin(3);
		nAttr.setSoftMax(30);
		s = addAttribute(s_hardening);
		CHECK_MSTATUS_AND_RETURN_IT(s);
		
		s_criticalCompression = nAttr.create(s_criticalCompressionName[0], s_criticalCompressionName[1], MFnNumericData::kFloat, 0.019, &s);
		nAttr.setMin(0);
		nAttr.setMax(10);
		nAttr.setSoftMin(0.001);
		nAttr.setSoftMax(0.1);
		s = addAttribute(s_criticalCompression);
		CHECK_MSTATUS_AND_RETURN_IT(s);

		s_criticalStretch = nAttr.create(s_criticalStretchName[0], s_criticalStretchName[1], MFnNumericData::kFloat, 0.0075, &s);
		nAttr.setMin(0);
		nAttr.setMax(10);
		nAttr.setSoftMin(0.001);
		nAttr.setSoftMax(0.01);
		s = addAttribute(s_criticalStretch);
		CHECK_MSTATUS_AND_RETURN_IT(s);

		s_friction = nAttr.create(s_frictionName[0], s_frictionName[1], MFnNumericData::kFloat, 1.f, &s);
		nAttr.setMin(0);
		nAttr.setMax(1e5);
		nAttr.setSoftMin(0.1);
		nAttr.setSoftMax(5.0);
		s = addAttribute(s_friction);
		CHECK_MSTATUS_AND_RETURN_IT(s);

		s_flip = nAttr.create(s_flipName[0], s_flipName[1], MFnNumericData::kFloat, 0.95, &s);
		nAttr.setMin(0);
		nAttr.setMax(10);
		nAttr.setSoftMin(0.1);
		nAttr.setSoftMax(1.5);
		s = addAttribute(s_flip);
		CHECK_MSTATUS_AND_RETURN_IT(s);
		
		s_gravity = nAttr.create(s_gravityName[0], s_gravityName[1], MFnNumericData::k3Float, 0, &s);
		nAttr.setMin(-1e6);
		nAttr.setMax(1e6);
		s = addAttribute(s_gravity);
		CHECK_MSTATUS_AND_RETURN_IT(s);

		s_deltaT = nAttr.create(s_deltaTName[0], s_deltaTName[1], MFnNumericData::kFloat, 0.001f, &s);
		nAttr.setMin(1e-3);
		nAttr.setMax(10);
		nAttr.setSoftMin(0.01);
		nAttr.setSoftMax(0.1);
		s = addAttribute(s_deltaT);
		CHECK_MSTATUS_AND_RETURN_IT(s);
	}
	{
		s_initParticle = tAttr.create(s_initParticleName[0], s_initParticleName[1], OpenVDBData::id, MObject::kNullObj, &s);
		tAttr.setReadable(false);
		tAttr.setArray(true);
		tAttr.setUsesArrayDataBuilder(true);
		s = addAttribute(s_initParticle);
		CHECK_MSTATUS_AND_RETURN_IT(s);
	}
	return s;
}

bool MpmSimulator::initSolver()
{
	int frame = getCurFrame();
	MPlug boxMinPlug = Global::getPlug(this, s_boxMinName[0]);
	MPlug boxMaxPlug = Global::getPlug(this, s_boxMaxName[0]);
	MPlug cellSizePlug = Global::getPlug(this, s_cellSizeName[0]);
	MPlug nParticlePlug = Global::getPlug(this, s_nParticlePerCellName[0]);
	MPlug boundaryPlug = Global::getPlug(this, s_boundaryName[0]);

	MPlug youngsPlug = Global::getPlug(this, s_youngsName[0]);
	MPlug possionPlug = Global::getPlug(this, s_poissonName[0]);
	MPlug hardenPlug = Global::getPlug(this, s_hardeningName[0]);
	MPlug cCompPlug = Global::getPlug(this, s_criticalCompressionName[0]);
	MPlug cStrePlug = Global::getPlug(this, s_criticalStretchName[0]);
	MPlug frictionPlug = Global::getPlug(this, s_frictionName[0]);
	MPlug flipPlug = Global::getPlug(this, s_flipName[0]);
	MPlug gravityPlug = Global::getPlug(this, s_gravityName[0]);
	MPlug deltaTPlug = Global::getPlug(this, s_deltaTName[0]);
	MPlug particleDensityPlug = Global::getPlug(this, s_particleDensityName[0]);

	MStatus s;
	Vector3f gridMin, gridMax, cellSize;
	s = Global::getFloat(boxMinPlug, &gridMin[0], 3);
	CHECK_MSTATUS_AND_RETURN(s, false);
	s = Global::getFloat(boxMaxPlug, &gridMax[0], 3);
	CHECK_MSTATUS_AND_RETURN(s, false);
	s = Global::getFloat(cellSizePlug, &cellSize[0], 3);
	CHECK_MSTATUS_AND_RETURN(s, false);
	int nParticle = nParticlePlug.asInt();
	int boundary = boundaryPlug.asInt();
	
	if (!cellSize.norm())
		return false;
	bool res = m_core.init(gridMin.cwiseMin(gridMax), gridMax.cwiseMax(gridMin), cellSize.cwiseAbs(), boundary, frame);
	//m_core.createBall(Vector3f(0,0,0), 1, nParticle, frame);
	res &= initParticle();
	if (res)
	{
		MGlobal::displayInfo("init succeed!");
	}
	else
	{
		MGlobal::displayInfo("init fail!");
	}
	return res;
}

bool MpmSimulator::initParticle()
{
	MPlug initParticlePlug = Global::getPlug(this, s_initParticleName[0]);
	MPlug nParticlePlug = Global::getPlug(this, s_nParticlePerCellName[0]);
	MStatus s;

	int nPPC = nParticlePlug.asInt();

	for (unsigned i = 0; i < initParticlePlug.numElements(&s); i++)
	{
		MPlug ithElementPlug = initParticlePlug.elementByPhysicalIndex(i, &s);
		if (!s)
			continue;

		int logicalIdx     = ithElementPlug.logicalIndex(&s);
		MObject valObj;
		s = ithElementPlug.getValue(valObj);
		if (!s)
			continue;

		OpenVDBData* data;
		s = Global::getVDBData(valObj, data);
		
		if (!s || !data)
			continue;

		std::vector<openvdb::GridBase::ConstPtr> grids;
		mvdb::getGrids(grids, *data, "");
		for (int ithGrid = 0; ithGrid < grids.size(); ++ithGrid)
		{
			openvdb::GridBase::ConstPtr& pGridBase = grids[i];
			if (!pGridBase)
				continue;
			else if (pGridBase->isType<openvdb::FloatGrid>())
				m_core.addParticleGrid<openvdb::FloatGrid>(pGridBase, nPPC);
			else if (pGridBase->isType<openvdb::DoubleGrid>())
				m_core.addParticleGrid<openvdb::DoubleGrid>(pGridBase, nPPC);
		}
	}
	return true;
}


bool MpmSimulator::stepSolver()
{
	int frame = getCurFrame();
	return m_core.for_each_frame(frame);
}





