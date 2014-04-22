//
//  attacker.cpp
//  atlantis
//
//  Created by Clay Ewing on 3/30/14.
//
//

#include "attacker.h"

void Attacker::setupCustom(int id) {
    setData(new CustomData());
    CustomData * theData = (CustomData *)getData();
    theData->type = TYPE_ATTACKER;
    theData->remove = false;
    theData->id = id;
    color = ofColor(255, 0, 0, 200);
}

void Attacker::display() {
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