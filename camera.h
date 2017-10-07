#pragma once
class camera
{
public:
	enum CameraType {FIXEDORTHO, FIXEDPERSPECTIVE, MOVINGPERSPECTIVE};
	void fixedOrtho();
	void fixedPerspective();
	void movingPerspective();
	camera();
	~camera();
};

