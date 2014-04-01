//
//  humanoid.cpp
//  atlantis
//
//  Created by Clay Ewing on 3/30/14.
//
//

#include "humanoid.h"

void Humanoid::setupCustom(int id) {
    setData(new CustomData());
    CustomData * theData = (CustomData *)getData();
    theData->type = TYPE_HUMANOID;
    theData->remove = false;
    theData->id = id;
    color = ofColor(127, ofRandom(150, 200), 127, 200);
}

void Humanoid::display() {
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