#include "ofApp.h"

#include "ofxSuperLog.h"
#include "ofxTimeMeasurements.h"
#include "ofxRemoteUIVars.h"

//--------------------------------------------------------------
// callback function for ofxRemoteUI actions
bool bRecalcNow = true;

void ruiServerCallback(RemoteUIServerCallBackArg arg){
    switch (arg.action) {
        case CLIENT_DID_SET_PRESET:
        case CLIENT_UPDATED_PARAM:
            bRecalcNow = true;
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::setup(){
    ofLogToFile("log.txt", true);
    //ofSetLogLevel(OF_LOG_VERBOSE);

    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofEnableAlphaBlending();

    TIME_SAMPLE_SET_FRAMERATE(60);
    // TIME_SAMPLE_ENABLE();

    // setup ofxRemoteUI
    RUI_SETUP();
    RUI_SET_CALLBACK(ruiServerCallback);
    // TODO: setup params here
    RUI_NEW_GROUP("App");
    RUI_DEFINE_VAR_WV(bool, "app-fullscreen", false);
    RUI_DEFINE_VAR_WV(int, "width", 320, 10, 1000);
    RUI_DEFINE_VAR_WV(int, "height", 240, 10, 1000);
    RUI_DEFINE_VAR_WV(int, "margin", 10, 0, 500);
    RUI_DEFINE_VAR_WV(int, "fps", 1, 0, 60);
    RUI_DEFINE_VAR_WV(float, "rotateSpeed", 0, 0, 1);
    RUI_DEFINE_VAR_WV(float, "rotateStrength", 0, 0, 360);
    RUI_DEFINE_VAR_WV(int, "opacity", 255, 0, 255);
    
    
    //build a string list for the UI to show
    string blendmodeOptions[] = {"Disabled", "Alpha", "Add", "Subtract", "Multiply", "Screen"};
    chosenBlendmode = OF_BLENDMODE_ALPHA;
    //privide the enum param, loweset enum, highest enum, and the Enum string list
    RUI_SHARE_ENUM_PARAM(chosenBlendmode, OF_BLENDMODE_DISABLED, OF_BLENDMODE_SCREEN, blendmodeOptions);
    RUI_LOAD_FROM_XML();

    bRecalcNow = true;
    time=0.f;
    timeLastFrame=0.f;
}

//--------------------------------------------------------------
void ofApp::update(){
    float dt = (1.f/60.f); // elapsed time (60 fps)
    time += dt;

    if(bRecalcNow){
        bRecalcNow = false;

        // TODO: perform post-param-change updates
        ofSetFullscreen(RUI_VAR(bool, "app-fullscreen"));
        
        ofVec2f newSize(RUI_VAR(int, "width"), RUI_VAR(int, "height"));
        if(newSize != camSize){
            camSize.set(RUI_VAR(int, "width"), RUI_VAR(int, "height"));
            ofSetWindowShape(camSize.x, camSize.y);
            fbo.allocate(camSize.x, camSize.y);
            fbo2.allocate(camSize.x, camSize.y);
            grabber.initGrabber(camSize.x, camSize.y);
            center = camSize/2;
        }

        //if(ofGetFrameRate() != RUI_VAR(int, "fps")){
        //    ofSetFrameRate(RUI_VAR(int, "fps"));
        //}
        frameTime = 1.0/RUI_VAR(int, "fps");
        
        ofEnableBlendMode(chosenBlendmode);
    }

    // update video grabber (get the latest frame)
    grabber.update();

    if (grabber.isFrameNew() && (time-timeLastFrame) > frameTime) {
        timeLastFrame = time;

        // draw camera frame (bottom layer) to fbo
        fbo.begin();
        grabber.draw(0,0);
        
        // draw fbo2 (previous frame) on top of camera layer,
        // apply margin, rotation and opacity
        int m = RUI_VAR(int, "margin");
        float t = ofGetElapsedTimef();
        ofPushMatrix();
        ofTranslate(center);
        ofRotateZ(RUI_VAR(float, "rotateStrength") * sin(t * RUI_VAR(float, "rotateSpeed")));
        ofTranslate(-center);
        ofSetColor(RUI_VAR(int, "opacity"));
        fbo2.draw(m,m, camSize.x-2*m, camSize.y-2*m);
        
        ofPopMatrix();
        fbo.end();
        fbo2.begin();
        fbo.draw(0,0);
        fbo2.end();
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    fbo2.draw(0,0);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch(key){
        case 'f':
            OFX_REMOTEUI_SERVER_SET_VAR(bool, "app-fullscreen", !RUI_VAR(bool, "app-fullscreen"));
            bRecalcNow = true;
            break;
    }
}

//--------------------------------------------------------------
void ofApp::exit(){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    center.set(x,y);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    center.set(x,y);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
	
}
