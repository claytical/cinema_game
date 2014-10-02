//
//  invader.cpp
//  cinemaServerAudio
//
//  Created by Clay Ewing on 3/4/14.
//
//

#include "invader.h"

void Invader::setupCustom(int id) {
    setData(new CustomData());
    CustomData * theData = (CustomData *)getData();
    theData->type = INVADER_TYPE;
    theData->remove = false;
    theData->id = id;
}
void Invader::display() {
    //    float radius = getRadius();
    float width = getWidth();
    float height = getHeight();
    ofPushMatrix();
    ofSetRectMode(OF_RECTMODE_CENTER);
    ofTranslate(getPosition());
    ofRotateZ(getRotation());
    ofSetColor(0,255,0);
    ofFill();
//    image->draw(0,0, width, height);
    ofRect(0, 0, width, height);
    ofSetRectMode(OF_RECTMODE_CENTER);
    //    ofCircle(0,0,radius);
    ofPopMatrix();
}