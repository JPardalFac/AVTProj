#include "Object.h"

void Object::init(int id, float pos[3], float rotAxis[3], float rot) {

	objId = id;

	//set obj position
	position[0] = pos[0];
	position[1] = pos[1];
	position[2] = pos[2];

	//set rotation axis
	rotationAxis[0] = rotAxis[0];
	rotationAxis[1] = rotAxis[1];
	rotationAxis[2] = rotAxis[2];

	//set rotation value
	rotation = rot;
}

void Object::move(int dir)
{
}

void Object::rotate(int dir)
{
}

//void Object::setColor(MyMesh mesh[], float amb[4], float diff[4], float spec[4], float emissive[4],float shininess,int texcount) {
//	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
//	memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
//	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
//	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
//	mesh[objId].mat.shininess = shininess;
//	mesh[objId].mat.texCount = texcount;
//		createCube();
//}

