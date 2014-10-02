//
//  pod.cpp
//  atlantis
//
//  Created by Clay Ewing on 4/7/14.
//
//

#include "pod.h"
void Pod::setupCustom(int id) {
    setData(new CustomData());
    CustomData * theData = (CustomData *)getData();
    theData->type = TYPE_POD;
    theData->remove = false;
    theData->id = id;
    color = ofColor(ofRandom(100, 200), 0, 0, 200);
}

void Pod::display() {
    float width = getWidth();
    float height = getHeight();
    ofPushMatrix();
    ofSetRectMode(OF_RECTMODE_CENTER);
    ofTranslate(getPosition());
    ofRotateZ(getRotation());
    ofSetColor(color);
    ofFill();
    //    image->draw(0,0, width, height);
    ofRect(0, 0, width, height);
    ofSetRectMode(OF_RECTMODE_CORNER);
    ofPopMatrix();
}

void Pod::init() {
    
}