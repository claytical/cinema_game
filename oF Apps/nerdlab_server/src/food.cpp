//
//  food.cpp
//  atlantis
//
//  Created by Clay Ewing on 3/30/14.
//
//

#include "food.h"

void Food::setupCustom(int id) {
    setData(new CustomData());
    CustomData * theData = (CustomData *)getData();
    theData->type = TYPE_FOOD;
    theData->remove = false;
    theData->id = id;
    color = ofColor(ofRandom(100, 200), 200);
}
void Food::display() {
    float width = getWidth();
    float height = getHeight();
    ofPushMatrix();
    ofSetRectMode(OF_RECTMODE_CENTER);
    ofTranslate(getPosition());
    ofRotateZ(getRotation());
    ofSetColor(0, 200);
    image->draw(0, 0, width*1.1, height*1.1);
    ofSetColor(color);
    image->draw(0,0, width, height);
    ofSetRectMode(OF_RECTMODE_CORNER);
    ofPopMatrix();
}