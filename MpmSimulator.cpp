#include "stdafx.h"
#include "MpmSimulator.h"

const char* MpmSimulator::m_nodeName = "MpmSimulator";
MTypeId MpmSimulator::m_id(0x00108A60);

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
	for (int i = 0; i < nCell[1]; ++i)
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
	return s;
}

bool MpmSimulator::initSolver()
{
	int frame = getCurFrame();
	bool res = m_core.init(frame);
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

bool MpmSimulator::stepSolver()
{
	int frame = getCurFrame();
	return m_core.for_each_frame(frame);
}

