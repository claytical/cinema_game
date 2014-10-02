#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0);
    
	introText.loadFont("joystix.ttf", 30);
    largeGameText.loadFont("joystix.ttf", 100);
    
    appstore.loadImage("appstore.png");
    android.loadImage("android.png");
    play.loadImage("play.png");
    wifi.loadImage("wifi.png");
    select.loadImage("select.png");
    
    
    slideNum = 0;
    nextSlide = ofGetElapsedTimef()+7;

}

//--------------------------------------------------------------
void ofApp::update(){
    if(nextSlide <= ofGetElapsedTimef()){
        slideNum++;
        nextSlide=ofGetElapsedTimef()+7;
    }
    if(slideNum>4){
        slideNum=0;
    }

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetColor(225);
    introText.drawString(ofToString(nextSlide), 50, 50);
    
    switch (slideNum) {
        case 0:
            largeGameText.drawString("Atlantis", ofGetWidth()/2-400, ofGetHeight()/2);
            break;
        case 1:
            introText.drawString("1. Download NERDlab controller!", ofGetWidth()/2-500, ofGetHeight()/2);
            appstore.draw((ofGetWidth()/2)-250, (ofGetHeight()/2)+50);
            android.draw(ofGetWidth()/2, (ofGetHeight()/2)+55);
            break;
        case 2:
            introText.drawString("2. Join NERDlab wifi network!", ofGetWidth()/2-500, ofGetHeight()/2);
            wifi.draw(ofGetWidth()/2-200, (ofGetHeight()/2)+50);
            break;
        case 3:
            introText.drawString("3. Press play on controller!", ofGetWidth()/2-500, ofGetHeight()/2);
            play.draw(ofGetWidth()/2-200, (ofGetHeight()/2)+50);
            break;
        case 4:
            introText.drawString("4. Select NERDlab server!", ofGetWidth()/2-500, ofGetHeight()/2);
            select.draw(ofGetWidth()/2-200, (ofGetHeight()/2)+50);
            break;
        default:
            largeGameText.drawString("Atlantis", ofGetWidth()/2-400, ofGetHeight()/2);
            break;
    }
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

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
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}