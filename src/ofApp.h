#pragma once

#include "ofMain.h"
#include "ofxNI2.h"
#include "ofxGui.h"
#include "ofxOpenCv.h"
#include <OpenNI.h>

/*
 DO NOT FORGET TO PASTE THE FOLLOWING CODES TO SCRIPTS IN PROJECT SETTINGS
 
 
 cp -R ../../../addons/ofxNI2/libs/OpenNI2/lib/osx/ "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/MacOS/";
 cp -R ../../../addons/ofxNI2/libs/NiTE2/lib/osx/ "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/MacOS/";
 
 */


#define TEXTURE_SIZE    512
#define MIN_NUM_CHUNKS(data_size, chunk_size)    ((((data_size)-1) / (chunk_size) + 1))
#define MIN_CHUNKS_SIZE(data_size, chunk_size)    (MIN_NUM_CHUNKS(data_size, chunk_size) * (chunk_size))

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void exit();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
    
    ofxNI2::Device device;
    ofxNI2::IrStream ir;
    ofxNI2::Grayscale gray;
    //ofxNI2::ColorStream colorStream;
    ofxNI2::DepthStream depth;
    
    ofFbo depthFbo;
    
    int w,h;
    
    // Opencv
    ofxCvColorImage colorImg;
    
    ofxCvGrayscaleImage grayImage; // grayscale depth image
    ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
    ofxCvGrayscaleImage grayThreshFar; // the far thresholded image
    
    ofxCvContourFinder contourFinder;
    
    // used for viewing the point cloud
    ofEasyCam easyCam;
    
    // GUI
    ofxPanel gui;
    ofParameter<bool> bThreshWithOpenCV;
    ofParameter<bool> bDrawPointCloud;
    
    ofParameter<int> nearThreshold;
    ofParameter<int> farThreshold;
    
    ofParameter<int> nearThresHoldDepth;
    ofParameter<int> farThresHoldDepth;
    ofParameter<int> pointSize;
    
    ofxLabel fps;
    
    bool isDebug;
    
    // utils
    void drawStroke(int x, int y, int w, int h, ofColor color = ofColor(255));
    
    // draw pointcloud
    void drawPointCloud();

};
