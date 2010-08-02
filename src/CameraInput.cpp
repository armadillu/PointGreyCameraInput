/*
 *  CameraInput.cpp
 *  emptyExample
 *
 *  Created by Oriol Ferrer Mesià on 24/04/10.
 *  Copyright 2010 uri.cat. All rights reserved.
 *
 */

#include "CameraInput.h"

//--------------------------------------------------------------
CameraInput::CameraInput(int w, int h, VID_FORMATS format, int fps, bool discardFrames, bool useTexture, bool mode7, uint deviceID, bool verbose ){

	camWidth 		= w;	// try to grab at this size.
	camHeight 		= h;

	uiState = HIDDEN;
	camFormat = format;

	mytimeThen		= 0.0f;
	myframeRate     = 0.0f;
	myframes        = 0.0f;

    vidGrabber.setDeviceID(deviceID);

    sdk = new Libdc1394Grabber();

	if (mode7)
		sdk->setFormat7(VID_FORMAT7_1);
	
	sdk->listDevices();
	sdk->setDiscardFrames(discardFrames);
	sdk->set1394bMode(false);
	//sdk->setROI(0,0,320,240);
	//sdk->setDeviceID("b09d01008bc69e:0");

	ofxIIDCSettings *settings = new ofxIIDCSettings();
	char aux[255];
	sprintf(aux,"camSettings%d.xml",deviceID );
	settings->setXMLFilename(aux);

	vidGrabber.setVerbose(verbose);
	
	// GOOD ONE FOR FLYCAP!
	//bool result = vidGrabber.initGrabber( camWidth, camHeight, VID_FORMAT_GREYSCALE, VID_FORMAT_GREYSCALE, 60, true, sdk);
	
	warp = false;
	
	bool result = vidGrabber.initGrabber( camWidth, camHeight, format, format, fps, useTexture, sdk);

	if(result) {
	    ofLog(OF_LOG_VERBOSE,"Camera successfully initialized.");
		
		vidGrabber.update();
		
		if (format == VID_FORMAT_GREYSCALE){
			grayImage.allocate( camWidth, camHeight );
			grayImage.setFromPixels( vidGrabber.getPixels(), camWidth, camHeight );
			grayWarpImage.allocate(camWidth, camHeight);
			
			camImage = &grayImage;
		}
	
		if (format == VID_FORMAT_RGB){
			rgbImage.allocate( camWidth, camHeight );
			rgbImage.setFromPixels( vidGrabber.getPixels(), camWidth, camHeight );

			camImage = &rgbImage;
		}
		
		binaryImage.allocate(camWidth, camHeight);
		accImage.allocate(camWidth, camHeight);
		accImage.setNativeScale(0.0f,255.0f);
		diffImage.allocate(camWidth, camHeight);
		bgImage.allocate(camWidth, camHeight);

		accImage = grayWarpImage;
		
		warper.setup( 0, 0, camWidth, camHeight );
		readSettings();
		
		processingPanel.setXMLFilename("processingSettings.xml");
		warpingPanel.setXMLFilename("warpingSettings.xml");
		setupGUIEvents();
		setupUI();
		processingPanel.reloadSettings();
		warpingPanel.reloadSettings();
		
	} else {
	    ofLog(OF_LOG_FATAL_ERROR,"Camera failed to initialize.");
	}
}


CameraInput::~CameraInput(){

	printf("closing VideoGrabber\n");
	vidGrabber.close();
	delete sdk;
}


//--------------------------------------------------------------
void CameraInput::update(){

	vidGrabber.update();
		
	if (vidGrabber.isFrameNew()){
		
        calculateCaptureFramerate();
		camImage->setFromPixels( vidGrabber.getPixels(), camWidth, camHeight );

		if (camFormat == VID_FORMAT_RGB){
			if (warp)
				grayImage = rgbImage;
			else
				grayWarpImage = rgbImage;
		}
		
		if (camFormat == VID_FORMAT_GREYSCALE){
			if (warp)
				grayWarpImage.warpIntoMe( grayImage, warper.dstPoints , warper.srcPoints);
			else
				grayWarpImage = grayImage;
		}
		
		//now grayWarpImage has the image
		
		if (bgSubtraction){			
			//absDiffWithCutoffInverted( &bgImage, &grayWarpImage, &diffImage, bgSubtractionCutOff );
			cvAbsDiff(bgImage.getCvImage(), grayWarpImage.getCvImage(), diffImage.getCvImage());
			diffImage.flagImageChanged();
			binaryImage = diffImage;
		}else {
			binaryImage = grayWarpImage;
		}

		if (threshold)
			binaryImage.threshold(thresholdVal, false);

		if (invert)
			binaryImage.invert();
		
		if ( numErosions > 0 ){				
			for (int i=0; i<numErosions; i++)
				binaryImage.erode();
		}		

		if ( numDilations > 0 ){				
			for (int i=0; i<numDilations; i++)
				binaryImage.dilate();
		}
			
		if (bgSubtraction){
			cvRunningAvg(grayWarpImage.getCvImage(), accImage.getCvImage(), bgAccumRate);
			cvConvert( accImage.getCvImage(), bgImage.getCvImage() );
			bgImage.flagImageChanged();
		}
	}
	
	if (uiState == SET_PROCESSING_SETTINGS){
		processingPanel.update();
	}
	
	if (uiState == SET_IMAGE_WARP){
		warpingPanel.update();
	}

}


void CameraInput::setupUI(){
	
    processingPanel.setup("Camera Image Processing Settings", camWidth/2, 0, 420, 480);
	processingPanel.loadFont("LiberationMono-Regular.ttf", 10);
    processingPanel.addPanel("Settings", 2, false);
	processingPanel.setWhichPanel("Settings");
	processingPanel.setWhichColumn(0);

    /* set the gui callback */
    callback.SetCallback( this, &CameraInput::parameterCallback);

	processingPanel.addToggle("Bg Subtraction Enable", "bgEnable", false, &callback, UI_BG_ENABLE);
	processingPanel.addButtonSlider("Bg Adapt Rate", "bgAdaptRate" , 0.0001, 0. , 0.001, false, &callback,UI_BG_ADAPT);
	//processingPanel.addButtonSlider("Bg CutOff", "bgCutoff" , 128, 0, 255, true, &callback,UI_BG_CUTTOFF);

	processingPanel.addToggle("Threshold Enable", "thresholdEnable", false, &callback, UI_THRESHOLD_ENABLE_ID);
	processingPanel.addButtonSlider("threshold", "Threshold" , 128, 0, 255, true, &callback,UI_THRESHOLD_ID);

	processingPanel.addToggle("Invert", "invert", false, &callback, UI_INVERT_ENABLE);
	
	processingPanel.addButtonSlider("Erode", "erode" , 0, 0, 8, true, &callback,UI_ERODE);
	processingPanel.addButtonSlider("Dilate", "dilate" , 0, 0, 8, true, &callback,UI_DILATE);
	
	warpingPanel.setup("Image Crop / Warp Settings", camWidth * 1.5f, 0, 350, 200);
	warpingPanel.loadFont("LiberationMono-Regular.ttf", 10);
    warpingPanel.addPanel("Settings", 2, false);
	warpingPanel.setWhichColumn(0);
	warpingPanel.addToggle("Calc Image Warp", "warpEnable", false, &callback, UI_WARP_ENABLE);
}


void CameraInput::absDiffWithCutoffInverted( ofxCvGrayscaleImage *backImage, ofxCvGrayscaleImage *image,  ofxCvGrayscaleImage *diffImage, uchar upperCut ){

	unsigned char val, bg;
	IplImage* img = image->getCvImage();
	IplImage* bgImg = backImage->getCvImage();
	IplImage* diffImg = diffImage->getCvImage();

	int totalPixels = img->height * img->width;

	for (register int i=0; i < totalPixels; i++) {

 		val = img->imageData[i];	//our camera pixel brightness
		bg = bgImg->imageData[i];	//our bg pixel brightness

		if ( val > upperCut) {
			diffImg->imageData[i] = 0;
		} else {
			diffImg->imageData[i] = ABS(val - bg);
		}
	}
}


void CameraInput::draw(){
	
	float tw = camWidth / 2;
	float th = camHeight / 2;
	
	int count = 0;
	
	switch (uiState) {

		case HIDDEN:
			break;

		case SET_CAM_SETTINGS:
			camImage->draw(0,0, ofGetWidth(), ofGetHeight() );
			vidGrabber.drawSettings();
			ofDrawBitmapString(buf2, 20, ofGetHeight() - 20);
			ofDrawBitmapString( ofToString( ofGetFrameRate(), 3), 20, ofGetHeight() - 40);
			break;

		case SET_IMAGE_WARP:
			
			camImage->draw(0,0);
			grayWarpImage.draw(camWidth, 0.f, tw, th );
			
			if (warp)
				warper.draw();
			warpingPanel.draw();

			ofSetColor(0xffffff);
			ofDrawBitmapString( "w: write settings   r: read settings", 20, ofGetHeight() - 20);
			ofDrawBitmapString( ofToString( ofGetFrameRate(), 3), 20, ofGetHeight() - 40);
			break;
			
		case SET_PROCESSING_SETTINGS:
			
			grayWarpImage.draw(0, 0, tw, th); count++;
						
			if (bgSubtraction){
				//accImage.draw(0, th * count, tw, th); count++;
				bgImage.draw(0, th * count, tw, th); count++;
				diffImage.draw(0, th * count, tw, th); count++;
			}

			binaryImage.draw(0, th * count, tw, th); count++;
			processingPanel.draw();
			ofDrawBitmapString( "b: capture Background", 20, ofGetHeight() - 20);
			ofDrawBitmapString( ofToString( ofGetFrameRate(), 3), 20, ofGetHeight() - 40);
			break;

		default: break;
	}
}


void CameraInput::calculateCaptureFramerate(){
	
    mytimeNow = ofGetElapsedTimef();
    if( (mytimeNow-mytimeThen) > 0.05f || myframes == 0 ) {
        myfps = myframes / (mytimeNow-mytimeThen);
        mytimeThen = mytimeNow;
        myframes = 0;
        myframeRate = 0.9f * myframeRate + 0.1f * myfps;
        sprintf(buf2,"Capture framerate : %f",myframeRate);
    }
    myframes++;
}


void CameraInput::setupGUIEvents(){	
    ofAddListener(ofEvents.mousePressed, this, &CameraInput::mousePressed);
    ofAddListener(ofEvents.mouseDragged, this, &CameraInput::mouseDragged);
    ofAddListener(ofEvents.mouseReleased, this, &CameraInput::mouseReleased);
	ofAddListener(ofEvents.keyPressed, this, &CameraInput::keyDown);
}


void CameraInput::mousePressed(ofMouseEventArgs & args){
	if (uiState == SET_IMAGE_WARP){ 
		warper.mousePressed(args.x, args.y);
		warpingPanel.mousePressed(args.x,args.y,args.button);
	}
    if (uiState == SET_PROCESSING_SETTINGS) processingPanel.mousePressed(args.x,args.y,args.button);
}

void CameraInput::mouseDragged(ofMouseEventArgs & args){
	if (uiState == SET_IMAGE_WARP){ 
		warper.mouseDragged(args.x, args.y);
		warpingPanel.mouseDragged(args.x,args.y,args.button);
	}
	if (uiState == SET_PROCESSING_SETTINGS) processingPanel.mouseDragged(args.x,args.y,args.button);
}


void CameraInput::mouseReleased(ofMouseEventArgs & args){
	if (uiState == SET_IMAGE_WARP){ 
		warper.mouseReleased();
		warpingPanel.mouseReleased();
	}
	if (uiState == SET_PROCESSING_SETTINGS) processingPanel.mouseReleased();
}

void CameraInput::keyDown(ofKeyEventArgs & args){

	int key = args.key;
	
	if (key == '0') hideUI();
	if (key == '1')	editCamSettings();
	if (key == '2')	editImageWarp();
	if (key == '3') editProcessing();
				
	switch (uiState) {
		case HIDDEN:break;
		case SET_CAM_SETTINGS:break;
		case SET_PROCESSING_SETTINGS:
			if (key == 'b'){ 
				accImage = grayWarpImage;
			}
			break;

		case SET_IMAGE_WARP:
			if( key == 'w' ) writeSettings();
			if (key == 'r' ) readSettings();
			break;
	}
}

void CameraInput::parameterCallback(float param1, float param2, int param_mode, int param_id){

	//printf("\ncallbakc: %f %f %d %d\n", param1,  param2, param_mode, param_id);
	
	switch (param_id) {
		case UI_THRESHOLD_ENABLE_ID:
			threshold = (bool)param_mode; break;
			
		case UI_THRESHOLD_ID:
			thresholdVal = (int)param1; break;

		case UI_BG_ENABLE:
			bgSubtraction = (bool)param_mode; break;

		case UI_BG_ADAPT:
			bgAccumRate = param1; break;

//		case UI_BG_CUTTOFF:
//			bgSubtractionCutOff = (int)param1; break;

		case UI_INVERT_ENABLE:
			invert = (bool)param_mode; break;

		case UI_ERODE:
			numErosions = (int)param1; break;

		case UI_DILATE:
			numDilations = (int)param1; break;

		case UI_WARP_ENABLE:
			warp = (bool)param_mode; break;

		default:
			break;
	}
	//printf("th enabled: %d   val: %d\n", threshold, thresholdVal);
}


void CameraInput::writeSettings(){
	warper.saveSettings( ofToDataPath("points.xml") );
}

void CameraInput::readSettings(){
	warper.loadSettings( ofToDataPath("points.xml") );
}

void CameraInput::editCamSettings(){

	vidGrabber.hideUI();
	processingPanel.hide();

	if (uiState != SET_CAM_SETTINGS){
		uiState = SET_CAM_SETTINGS;
		vidGrabber.showUI();
	}else{
		uiState = HIDDEN;
	}
}

void CameraInput::editProcessing(){
	vidGrabber.hideUI();
	processingPanel.hide();
	
	if (uiState != SET_PROCESSING_SETTINGS){
		uiState = SET_PROCESSING_SETTINGS;
		processingPanel.show();
	}else{
		uiState = HIDDEN;
	}
}

void CameraInput::editImageWarp(){
	vidGrabber.hideUI();
	processingPanel.hide();

	if (uiState != SET_IMAGE_WARP)
		uiState = SET_IMAGE_WARP;
	else
		uiState = HIDDEN;

}

void CameraInput::hideUI(){
	vidGrabber.hideUI();
	processingPanel.hide();
	
	uiState = HIDDEN;
}
