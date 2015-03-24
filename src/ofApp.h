#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"

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
	void exit();

    ofVideoGrabber grabber;
    ofFbo fbo, fbo2;
    ofVec2f camSize;
    float time, timeLastFrame, frameTime;
    
    ofBlendMode chosenBlendmode;
    ofPoint center;
    
    ofShader vignetteShader;
    
    ofImage vignetteMaskImage;
};
