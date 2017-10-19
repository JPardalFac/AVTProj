#pragma once
#include <sstream>
#include <iostream>
//#include "basic_geometry.h"

class Object {
protected:
	enum direction { back, forward, left, right };
	
public:
	int objId;
	float position[3];
	float rotationAxis[3];
	float rotation;
	

	virtual void init(int id, float pos[3], float rotAxis[3], float rot);
	//virtual void setColor(MyMesh mesh[],float amb[4],float diff[4],float spec[4],float emissive[4],float shininess,int texcount);
	//virtual void createObj();
	virtual void move(int dir);
	virtual void rotate(int dir);
	

};
