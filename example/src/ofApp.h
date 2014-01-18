#pragma once

#include "ofMain.h"

#include "ofxKazmath.h"

#include <kazmath/mat4.h>
#include <kazmath/vec3.h>

struct Vertex
{
	float x, y, z;
	
	Vertex(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}
};



class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
	
	
		ofImage ofLogo; 
		ofLight light;
		ofEasyCam cam;


		
};
