#include "boxWithHandles.h"


//---------------------------------------------------------
boxWithHandles::boxWithHandles()
{
	width 	    = 320;
	height 	    = 240;

	reset( 0,0,320,240);
}//---------------------------------------------------------
boxWithHandles::boxWithHandles(int w, int h)
{
	width 	    = w;
	height 	    = h;

	reset( 0,0,w,h);
}
//--------------------------------------------------------
void boxWithHandles::reset(float minX, float minY, float maxX, float maxY){

	dstPoints[0].x = minX;
	dstPoints[0].y = maxY;

	dstPoints[1].x = maxX;
	dstPoints[1].y = maxY;

	dstPoints[2].x = maxX;
	dstPoints[2].y = minY;

	dstPoints[3].x = minX;
	dstPoints[3].y = minY;

	for( int i = 0; i < 4; i++)
	{
        srcPoints[i].set(dstPoints[i].x,dstPoints[i].y);
	}

}
//--------------------------------------------------------
void boxWithHandles::setup(float x, float y, float w, float h){
	xCalib = x;
	yCalib = y;
	wCalib = w;
	hCalib = h;
	pointBeingCalibrated = -1;

	width 	    = w;
	height 	    = h;

	reset(0,0,w,h);

}

//--------------------------------------------------------
void boxWithHandles::mousePressed(float x, float y){

	float scalew = wCalib / (float)width;
	float scaleh = hCalib / (float)height;
	px = x;
	py = y;
	for (int i = 0; i < 4; i++){
		float xdiff = xCalib + dstPoints[i].x*scalew - x;
		float ydiff = yCalib + dstPoints[i].y*scaleh - y;
		float dist = sqrt(xdiff * xdiff + ydiff * ydiff);
		if (dist < HANDLE_SIZE){
			pointBeingCalibrated = i;
		}
	}
}
//--------------------------------------------------------
bool boxWithHandles::mouseDragged(float x, float y){

	float scalew = wCalib / (float)width;
	float scaleh = hCalib / (float)height;

	if (pointBeingCalibrated >= 0 && pointBeingCalibrated <= 3){
		float newx = (x - xCalib) / scalew;
		float newy = (y - yCalib) / scaleh;
		dstPoints[pointBeingCalibrated].x = newx;
		dstPoints[pointBeingCalibrated].y = newy;
		return true;
	}else{

		for (int i = 0; i < 4; i++){
			float newx = (px -x ) / scalew;
			float newy = (py -y ) / scaleh;
			dstPoints[i].x -= newx;
			dstPoints[i].y -= newy;
		}
		px = x;
		py = y;
		
		return false;
	}
}
//--------------------------------------------------------
void boxWithHandles::mouseReleased(){
	pointBeingCalibrated = -1;
}



//---------------------------------------------------------
void boxWithHandles::draw(){

	float scalew = wCalib / (float)width;
	float scaleh = hCalib / (float)height;

	ofSetColor(0xffff00);
	glLineWidth(2.0);
	
	glBegin(GL_LINE_LOOP);
		for (int i = 0; i < 4; i++){
			glVertex2f(xCalib + dstPoints[i].x*scalew, yCalib + dstPoints[i].y*scaleh);
		}
	glEnd();
	
	glBegin(GL_LINES);
		glVertex2f(xCalib + dstPoints[0].x*scalew, yCalib + dstPoints[0].y*scaleh);
		glVertex2f(xCalib + dstPoints[2].x*scalew, yCalib + dstPoints[2].y*scaleh);
		glVertex2f(xCalib + dstPoints[1].x*scalew, yCalib + dstPoints[1].y*scaleh);
		glVertex2f(xCalib + dstPoints[3].x*scalew, yCalib + dstPoints[3].y*scaleh);
	glEnd();

	ofFill();
	for (int i = 0; i < 4; i++){
		if (pointBeingCalibrated == i)  ofSetColor(0xff0000);
		else ofSetColor(0xffff00);		
		ofRect(xCalib + dstPoints[i].x*scalew - HANDLE_SIZE/2, -HANDLE_SIZE/2 + yCalib + dstPoints[i].y*scaleh, HANDLE_SIZE, HANDLE_SIZE);
	} 
}



//---------------------------------------------------------
void boxWithHandles::loadSettings(string filename_){

	ofxXmlSettings  xml;

    if( xml.loadFile(filename_) ){
		printf("Loaded mask settings.\n");
	}else{
		printf("Unable to load tracking settings, check data/ folder\n");
		return;
	}

	char quadStr[255];

	for (int i = 0; i < 4; i++){
			sprintf(quadStr, "cv:warp:point_%i:x", i);
			dstPoints[i].x = xml.getValue(quadStr, 0.0f);
			sprintf(quadStr, "cv:warp:point_%i:y", i);
			dstPoints[i].y = xml.getValue(quadStr, 0.0f);
	}

}
//---------------------------------------------------------
void boxWithHandles::saveSettings(string filename_){

	ofxXmlSettings  xml;
    xml.loadFile(filename_);

	char quadStr [255];

	for (int i = 0; i < 4; i++){
		sprintf(quadStr, "cv:warp:point_%i:x", i);
		xml.setValue(quadStr, (dstPoints[i].x));
		sprintf(quadStr, "cv:warp:point_%i:y", i);
		xml.setValue(quadStr, (dstPoints[i].y));
	}
	xml.saveFile(filename_);


}
