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
    TIME_SAMPLE_DISABLE();

    // setup ofxRemoteUI
    RUI_SETUP();
    RUI_SET_CALLBACK(ruiServerCallback);
    // TODO: setup params here
    RUI_NEW_GROUP("App");
    RUI_DEFINE_VAR_WV(bool, "app-fullscreen", false);
    RUI_DEFINE_VAR_WV(bool, "time-samples", false);
    RUI_DEFINE_VAR_WV(int, "width", 320, 10, 1000);
    RUI_DEFINE_VAR_WV(int, "height", 240, 10, 1000);
    RUI_DEFINE_VAR_WV(int, "margin", 10, 0, 500);
    RUI_DEFINE_VAR_WV(int, "fps", 1, 0, 60);
    RUI_DEFINE_VAR_WV(float, "offsetSpeed", 0.f, 0.f, 1.f);
    RUI_DEFINE_VAR_WV(float, "xOffset", 0.f, 0.f, 1000.f);
    RUI_DEFINE_VAR_WV(float, "yOffset", 0.f, 0.f, 1000.f);
    RUI_DEFINE_VAR_WV(float, "rotateSpeed", 0, 0, 1);
    RUI_DEFINE_VAR_WV(float, "rotateStrength", 0, 0, 360);
    RUI_DEFINE_VAR_WV(float, "tiltStrength", 0, 0, 360);
    RUI_DEFINE_VAR_WV(int, "opacity", 255, 0, 255);
    //build a string list for the UI to show
    string blendmodeOptions[] = {"Disabled", "Alpha", "Add", "Subtract", "Multiply", "Screen"};
    chosenBlendmode = OF_BLENDMODE_ALPHA;
    //privide the enum param, loweset enum, highest enum, and the Enum string list
    RUI_SHARE_ENUM_PARAM(chosenBlendmode, OF_BLENDMODE_DISABLED, OF_BLENDMODE_SCREEN, blendmodeOptions);
    RUI_DEFINE_VAR_WV(string, "vignette-shader", "vignetteMask.jpg");
    RUI_LOAD_FROM_XML();

    bRecalcNow = true;
    time=0.f;
    timeLastFrame=0.f;
    
    #ifdef TARGET_OPENGLES
        vignetteShader.load("shadersES2/vignetteShader");
    #else
        if(ofIsGLProgrammableRenderer()){
            vignetteShader.load("shadersGL3/vignetteShader");
        }else{
            vignetteShader.load("shadersGL2/vignetteShader");
        }
    #endif
    
    offset.set(0.,0.);
    source = &grabber;
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
        
        if(RUI_VAR(string, "vignette-shader") != ""){
            vignetteMaskImage.loadImage("vignetteMask.jpg");
        } else {
            vignetteMaskImage.clear();
        }
        
        if(RUI_VAR(bool, "time-samples") != TIME_SAMPLE_GET_ENABLED())
            TIME_SAMPLE_SET_ENABLED(RUI_VAR(bool, "time-samples"));
    }

    // update video grabber (get the latest frame)
    // grabber.update();
    source->update();

    if (source->isFrameNew() && (time-timeLastFrame) > frameTime) {
        timeLastFrame = time;

        // draw camera frame (bottom layer) to fbo
        fbo.begin();
        // if(mirrorHorizontal)
        source->draw(source->getWidth(),0, source->getWidth()*-1, source->getHeight());

        // draw otherfbo (previous frame) on top of camera layer,
        // apply margin, rotation and opacity
        int m = RUI_VAR(int, "margin");
        float t = ofGetElapsedTimef();
        ofPushMatrix();
        ofTranslate(center);
        ofRotateZ(RUI_VAR(float, "rotateStrength") * sin(t * RUI_VAR(float, "rotateSpeed")));
        ofRotateX(RUI_VAR(float, "tiltStrength") * sin(t * RUI_VAR(float, "rotateSpeed")));
        ofTranslate(-center);
        
        offset.set(RUI_VAR(float, "xOffset") * sin(t*RUI_VAR(float, "offsetSpeed")), RUI_VAR(float, "yOffset") * sin(t*RUI_VAR(float, "offsetSpeed")));
        ofTranslate(offset);
        ofSetColor(RUI_VAR(int, "opacity"));

        bool useVignetteShader = vignetteMaskImage.isAllocated();

        if(useVignetteShader){
            vignetteShader.begin();
            vignetteShader.setUniformTexture("imageMask", vignetteMaskImage.getTextureReference(), 1);
        }

        fbo2.draw(m,m, camSize.x-2*m, camSize.y-2*m);

        if(useVignetteShader){
            vignetteShader.end();
        }

        ofPopMatrix();
        fbo.end();
        
        // copy fbo to fbo2
        ofPixels pixs;
        fbo.getTextureReference().readToPixels(pixs);
        fbo2.getTextureReference().loadData(pixs);
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
    if(player.loadMovie(dragInfo.files[0])){
        source = &player;
        player.setLoopState(OF_LOOP_NORMAL);
        player.play();
        grabber.close();
    } else {
        source = &grabber;
        grabber.initGrabber(camSize.x, camSize.y);
    }
}
