#ifndef _BOX_W_HANDLES
#define _BOX_W_HANDLES

#include "ofMain.h"
#include "ofxXmlSettings.h"


#define	HANDLE_SIZE			10

class boxWithHandles {

	public:

		boxWithHandles();
		boxWithHandles(int w, int h);

		int width, height;

		ofPoint srcPoints[4];
		ofPoint dstPoints[4];

		//--------------------------------------------------------
		void setup(float x, float y, float w, float h);
		void reset(float minX, float minY, float maxX, float maxY);

		float 		xCalib, yCalib, wCalib, hCalib;
		int 		pointBeingCalibrated;
		void 		mousePressed(float x, float y);
		bool 		mouseDragged(float x, float y);
		void 		mouseReleased();
		//--------------------------------------------------------

		void loadSettings(string filename_);
		void saveSettings(string filename_);

		int px, py;

		void draw();



};

#endif
