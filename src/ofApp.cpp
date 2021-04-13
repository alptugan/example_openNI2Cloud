#include "ofApp.h"

    //--------------------------------------------------------------
void ofApp::setup(){
    // enable Logging
    ofLogToConsole();
    
    w = 320;
    h = 240;
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    
    ofSetWindowShape(1080, 1080);
    
    ofBackground(0);
    
    // setup device
    device.setup();
    
    // Allocate buffers
    /*depthFbo.allocate(w, h, GL_RGBA, 4);
    depthFbo.begin();
    ofClear(0, 0, 0, 0);
    depthFbo.end();
    */

    
    if (depth.setup(device))
    {
        depth.setSize(w, h);
        depth.setFps(30);
        depth.start();
        
        gray.setup(depth);
    }
    
    if (ir.setup(device)) // only for xtion device (OpenNI2-FreenectDriver issue)
    {
        ir.setSize(w, h);
        ir.setFps(30);
        ir.start();
    }
    
    // openCV
    colorImg.allocate(w, h);
    grayImage.allocate(w, h);
    grayThreshNear.allocate(w, h);
    grayThreshFar.allocate(w, h);
    
    // GUI
    isDebug = true;
    string path = "settings-xtion.xml";
    
    gui.setDefaultWidth(300);
    gui.setup("parameters", path);
    gui.add(nearThreshold.set("Near Threshold", 230, 0, 255));
    gui.add(farThreshold.set("Far Threshold", 70, 0, 255));
    gui.add(bThreshWithOpenCV.set("Enable OpenCV Threshold", true));
    gui.add(bDrawPointCloud.set("Draw Point Cloud", false));
    gui.add(stepSize.set("Point Cloud Resolution", 1, 1, 10));
    gui.add(pointSize.set("Set Point Size", 3, 0, 30));
    gui.add(distThres.set("Line Threshold", glm::vec2(1550, 1551), glm::vec2(0), glm::vec2(2000)));
    gui.add(nearThresHoldDepth.set("Near Cloud Threshold", 0, 0, 1000));
    gui.add(farThresHoldDepth.set("Far Cloud Threshold", 0, 0, 20000));
    gui.add(fps.setup("FPS",""));
    
    gui.loadFromFile(path);
    
    /*if (colorStream.setup(device)) // only for kinect device
     {
        colorStream.setSize(w, h);
        colorStream.setFps(60);
        colorStream.start();
     }*/
    
    
    // Lines
    lines.setMode(ofPrimitiveMode::OF_PRIMITIVE_LINES);
}


    //--------------------------------------------------------------
void ofApp::update(){
    device.update();
    
    fps.setup("FPS", ofToString(ofGetFrameRate(),0));
    
   
    // load grayscale depth image from the kinect source
     grayImage.setFromPixels(depth.getPixelsRef());
    
    // we do two thresholds - one for the far plane and one for the near plane
    // we then do a cvAnd to get the pixels which are a union of the two thresholds
    if(bThreshWithOpenCV) {
        grayThreshNear = grayImage;
        grayThreshFar = grayImage;
        grayThreshNear.threshold(nearThreshold, true);
        grayThreshFar.threshold(farThreshold);
        cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
    } else {
        
        // or we do it ourselves - show people how they can work with the pixels
        ofPixels & pix = grayImage.getPixels();
        
        unsigned long int numPixels = pix.size();
        for(int i = 0; i < numPixels; i++) {
            if(pix[i] < nearThreshold && pix[i] > farThreshold) {
                pix[i] = 255;
            } else {
                pix[i] = 0;
            }
        }
    }
    
    // update the cv images
    grayImage.flagImageChanged();
    
    // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
    // also, find holes is set to true so we will get interior contours as well....
    contourFinder.findContours(grayImage, 10, (depth.getWidth() * depth.getHeight())/2, 20, false);
}

    //--------------------------------------------------------------
void ofApp::draw(){
    /*
    
    ofPixels px = depth.getPixelsRef(ofGetMouseY(), ofGetMouseX());
    depthFbo.readToPixels(px);
    
    depth.draw(w, 0);
    drawStroke(w,0,w, h);*/
    //depthFbo.draw(w, 0);
    
    
    //ofSetColor(255, 255, 255);
    
    if(bDrawPointCloud) {
        easyCam.begin();
        drawPointCloud();
        easyCam.end();
    } else {
        // draw from the live kinect
        ir.draw(10,10);
        //colorStream.draw(10, 10);
        drawStroke(10,10,w, h);
        
        // Draw original depth
        depth.draw(20+w, 10);
        drawStroke(20+w,10, w, h);
        
        // Draw filtered image
        grayImage.draw(10, h+20, w, h);
        drawStroke(10, h+20, w, h);
        
        // Draw contour finder
        contourFinder.draw(20 + w, h+20, w, h);
        drawStroke(20 + w, h+20, w, h);
    }
    
    // draw instructions
    ofSetColor(255, 255, 255);
    stringstream reportStream;
    reportStream << "press p to switch between images and point cloud, rotate the point cloud with the mouse" << endl
    << "using opencv threshold = " << bThreshWithOpenCV <<" (press spacebar)" << endl
    << "set near threshold " << nearThreshold << " (press: + -)" << endl
    << "set far threshold " << farThreshold << " (press: < >) num blobs found " << contourFinder.nBlobs<< endl;
    
    ofDrawBitmapString(reportStream.str(), 20, 652);
    
    if(isDebug)
    {
        gui.draw();
    }
    
}

void ofApp::drawPointCloud() {
    int w = depth.getWidth();
    int h = depth.getHeight();
    ofMesh mesh;
    mesh.setMode(OF_PRIMITIVE_POINTS);
    ofPixels px = depth.getPixelsRef(nearThresHoldDepth, farThresHoldDepth, true);

    for(int y = 0; y < h; y += stepSize) {
        for(int x = 0; x < w; x += stepSize) {
            if(depth.getWorldCoordinateAt(x, y).z > nearThresHoldDepth && depth.getWorldCoordinateAt(x, y).z < farThresHoldDepth) {
                float mono = px.getColor(x, y).getBrightness();
                mesh.addColor(ofColor(mono));
                //ofColorr dd = depth.getPixelsRef().getColor(x, y).getNormalized();
                //ofLog() << dd;
                mesh.addVertex(depth.getWorldCoordinateAt(x, y));
            }
        }
    }
    glPointSize(pointSize);
    ofPushMatrix();
    // the projected points are 'upside down' and 'backwards'
    ofScale(1, 1, -1);
    ofTranslate(0, 0, -1000); // center the points a bit
    ofEnableDepthTest();
    mesh.drawVertices();
    
    
    lines.clear();
    //ofSetLineWidth(2);
    
    
    vector<glm::vec3> & meshVert = mesh.getVertices();
    
    for (int i = 0; i < meshVert.size(); i = i + 10) {
        for (int j = i + 10; j < meshVert.size(); j = j + 10) {
            
            if(j % 50 == 0) {
                float dst = glm::distance(meshVert[i], meshVert[j]);
            
                //if(randDice > 0.1)
                //{
                if(dst > distThres->x && dst <  distThres->y) {
                    //ofDrawLine(particles[i].getPos().x, particles[i].getPos().y, particles[j].getPos().x, particles[j].getPos().y);
                    float alpha = ofMap(dst, distThres->x,  distThres->y, 150, 0, true);
                    lines.addColor(ofColor(255,255, 255, alpha));
                    lines.addVertex(glm::vec3(meshVert[i].x, meshVert[i].y, meshVert[i].z));
                    lines.addColor(ofColor(255,255, 255, alpha));
                    lines.addVertex(glm::vec3(meshVert[j].x, meshVert[j].y, meshVert[j].z));
                }
            }
            //}
        }
    }
    
    lines.draw();
    
    ofDisableDepthTest();
    ofPopMatrix();
}

void ofApp::drawStroke(int x, int y, int w, int h, ofColor color) {
    ofPushStyle();
    ofSetColor(color);
    ofNoFill();
    ofDrawRectangle(x,y,w,h);
    ofPopStyle();
}
    //--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key) {
        case ' ':
            bThreshWithOpenCV = !bThreshWithOpenCV;
            break;
            
        case'p':
            bDrawPointCloud = !bDrawPointCloud;
            break;
            
        case '>':
        case '.':
            farThreshold ++;
            if (farThreshold > 255) farThreshold = 255;
            break;
            
        case '<':
        case ',':
            farThreshold --;
            if (farThreshold < 0) farThreshold = 0;
            break;
            
        case '+':
        case '=':
            nearThreshold ++;
            if (nearThreshold > 255) nearThreshold = 255;
            break;
            
        case '-':
            nearThreshold --;
            if (nearThreshold < 0) nearThreshold = 0;
            break;
    }
}

    //--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

    //--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

    //--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

    //--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

    //--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

    //--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

    //--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
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

void ofApp::exit() {
    // close device before app closing
    device.exit();
}
