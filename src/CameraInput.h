/*
 *  CameraInput.h
 *  emptyExample
 *
 *  Created by Oriol Ferrer Mesià on 24/04/10.
 *  Copyright 2010 uri.cat. All rights reserved.
 *
 */

#pragma once

#include "ofTypes.h"
#include "ofTexture.h"
#include "ofxVideoGrabber.h"
#include "boxWithHandles.h"
#include "ofxOpenCv.h"


#ifndef ABS
	#define ABS(x) (((x) < 0) ? -(x) : (x))
#endif


#define		UI_THRESHOLD_ENABLE_ID		1000
#define		UI_THRESHOLD_ID				1001

#define		UI_BG_ENABLE				1002
#define		UI_BG_ADAPT					1003
//#define		UI_BG_CUTTOFF				1004

#define		UI_INVERT_ENABLE			1005

#define		UI_ERODE					1006
#define		UI_DILATE					1007

#define		UI_WARP_ENABLE				1008
class CameraInput {

	public:

		enum UI_State{
			HIDDEN = 0,
			SET_CAM_SETTINGS,
			SET_PROCESSING_SETTINGS,
			SET_IMAGE_WARP
		};
	
		CameraInput(int w, int h, VID_FORMATS format, int fps, bool discardFrames = true, bool useTexture = true, bool mode7 = false, uint deviceID = 0, bool verbose = true );
		~CameraInput();
		bool update();	//returns yes if frame is new
		void draw();

		void mousePressed(ofMouseEventArgs & args);
		void mouseDragged(ofMouseEventArgs & args);
		void mouseReleased(ofMouseEventArgs & args);
		void keyDown(ofKeyEventArgs & args);
	
		ofxCvImage*	getCamImage(){ return camImage; }
		ofxCvGrayscaleImage*	getGrayCamImage(){ return &grayImage; }
		ofxCvGrayscaleImage*	getWarpedCamImage(){ return &grayWarpImage; }
		ofxCvGrayscaleImage*	getThresholdedImage(){ return &binaryImage; }
	
		void calculateWarpedImage(bool doIt){ warp = doIt; }
		
	
	private:

//		void enableThresholdProcessing(){ threshold = true;}
//		void disableThresholdProcessing(){ threshold = false;}

		void setupGUIEvents();
		void setupUI();

		void editCamSettings();
		void editImageWarp();	
		void editProcessing();	
		void hideUI();
	
		void writeSettings();
		void readSettings();

		void calculateCaptureFramerate();

		void parameterCallback(float param1, float param2, int param_mode, int callback_id);
        ParameterCallback<CameraInput> callback;
	
		UI_State					uiState;
	
		ofxVideoGrabber				vidGrabber;
		Libdc1394Grabber *			sdk;
		boxWithHandles				warper;
			
		int							camWidth;
		int							camHeight;
		VID_FORMATS					camFormat;

		ofxControlPanel				processingPanel;
		ofxControlPanel				warpingPanel;
	
		ofxCvImage *				camImage;
		
		//those are internal
		ofxCvColorImage				rgbImage;

		ofxCvGrayscaleImage			grayImage;
		ofxCvGrayscaleImage			grayWarpImage;
		
		ofxCvGrayscaleImage			binaryImage;
		ofxCvFloatImage				accImage;
		ofxCvGrayscaleImage			bgImage;
		ofxCvGrayscaleImage			diffImage;
		
	
		//processing
		bool						threshold;
		uint						thresholdVal;

		bool						bgSubtraction;
		double						bgAccumRate;
		//int							bgSubtractionCutOff;
	
		bool						invert;
		bool						warp;
	
		int							numErosions;
		int							numDilations;

		//framerate display
		char						buf[255];
		char						buf2[255];
		float						mytimeNow, mytimeThen, myframeRate;
		float						myfps;
		float						myframes;
	
		void absDiffWithCutoffInverted( ofxCvGrayscaleImage *backImage, ofxCvGrayscaleImage *image,  ofxCvGrayscaleImage *diffImage, uchar upperCut );
};
