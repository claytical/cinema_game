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
    offset = int(ofRandom(200));
}

void Humanoid::display() {
    float width = getWidth();
    float height = getHeight();
    ofPushMatrix();
    ofSetRectMode(OF_RECTMODE_CENTER);
    ofTranslate(getPosition());
    ofRotateZ(getRotation());
    float alpha = ofMap(sin((ofGetFrameNum()+offset) * .1), -1, 1, 50, 200);
    ofSetColor(255, alpha);
    glow->draw(0, 0, 50, 50);
    ofSetColor(color);
    ofFill();
    image->draw(0,0, width, height);
    ofSetRectMode(OF_RECTMODE_CORNER);
    ofPopMatrix();
}