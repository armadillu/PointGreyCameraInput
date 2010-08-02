#include "testApp.h"

void testApp::exit(){

	delete cam;
}

//--------------------------------------------------------------
void testApp::setup(){

	ofBackground(0,0,0);
	ofEnableAlphaBlending();
	ofEnableSmoothing();
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
															
	cam = new CameraInput(320, 240, 
							 VID_FORMAT_GREYSCALE, 
							 70, 
							 true,		//discard frames
							 false,		//use texture
							 true		//mode7	(needed to get 320 x 240 from the firefly MV)
							 );	
	
}

//--------------------------------------------------------------
void testApp::update(){
	cam->update();
}


//--------------------------------------------------------------
void testApp::draw(){
	
	ofSetColor(0xffffff);
	if ( cam->getWarpedCamImage() != NULL )
		cam->getWarpedCamImage()->draw(0, 0, ofGetWidth(), ofGetHeight());	
	cam->draw();	//UI and suff
}


//--------------------------------------------------------------
void testApp::keyPressed(int key){
}


//--------------------------------------------------------------
void testApp::keyReleased(int key){}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){}
