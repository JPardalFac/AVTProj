#pragma once

#include <iostream>
#include "AVTmathLib.h"

class camera
{
public:
	float eye[3] = { 0, 10, 0 }; //{ 0, 8, -1 };	//make sure this values are within the ortho[] values!, depending on the up vector, the component of the ortho box that will be compared to the values of this vector will vary
	float at[3] =  { 0, 0, 0 };
	float up[3] = { 0, 0, -1 }; 
	float fixedPerspectiveFov = 120;
	float movingPerspectiveFov = 60;

	float orthoBox[6] = {-20, 20, -20, 20, -5, 20}; //l,r,b,t,n,f
	enum CameraType {FIXEDORTHO, FIXEDPERSPECTIVE, MOVINGPERSPECTIVE};
	CameraType activeCam = MOVINGPERSPECTIVE;

	void setCameraType(CameraType newActiveCam);
	void setFixedOrtho();
	void setFixedPerspective(float* ratio);
	void setMovingPerspective(float* ratio);
	void updateLookAt(float carPos[]);
	void updateProjection(float* ratio);
	void sendCamCoords(float x, float y, float z);
	camera();
	~camera();
};

