//
//  humanoid.cpp
//  atlantis server
//
//  Created by Clay Ewing on 3/28/14.
//
//

#include "humanoid.h"

void Humanoid::setupCustom() {
    setData(new CustomData());
    CustomData * theData = (CustomData *)getData();
    theData->type = PLAYER_TYPE;
    theData->remove = false;
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
    image->draw(0, 0, width, height);
    ofSetRectMode(OF_RECTMODE_CORNER);
    ofPopMatrix();
    
}
