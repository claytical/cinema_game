//
//  pod.cpp
//  atlantis
//
//  Created by Clay Ewing on 4/7/14.
//
//

#include "pod.h"
void Pod::setupCustom() {
    setData(new CustomData());
    CustomData * theData = (CustomData *)getData();
    theData->type = TYPE_POD;
    theData->remove = false;
    color = ofColor(ofRandom(100, 200), 0, 0, 200);
}

void Pod::display() {
    float width = getWidth();
    float height = getHeight();
    ofPushMatrix();
    ofSetRectMode(OF_RECTMODE_CENTER);
    ofTranslate(getPosition());
    ofRotateZ(getRotation());
//    ofSetColor(color);
    ofFill();
//    passenger->draw(0, 0);
    image->draw(0,0);
    ofSetRectMode(OF_RECTMODE_CORNER);
    ofPopMatrix();
}

void Pod::init() {
    setVelocity(0, -1);

}