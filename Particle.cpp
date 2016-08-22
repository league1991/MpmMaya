#include "stdafx.h"


Particle::Particle( int pid, Vector3f& position, Vector3f& velocity, float pmass )
{
	this->pid=pid;
	this->position=position;
	this->velocity=velocity;
	this->pmass=pmass;

	volume=0;
	Fe=Matrix3f::Identity();
	Fp=Matrix3f::Identity();
}