//
//  shot.cpp
//  atlantis
//
//  Created by Clay Ewing on 3/30/14.
//
//

#include "shot.h"

void Shot::setupCustom(int id) {
    setData(new CustomData());
    CustomData * theData = (CustomData *)getData();
    theData->type = TYPE_FOOD;
    theData->remove = false;
    theData->id = id;
    color = ofColor(ofRandom(100, 200), 200);
}
void Shot::display() {
    float width = getRadius();
    ofPushMatrix();
    ofTranslate(getPosition());
    ofRotateZ(getRotation());
    ofSetColor(color);
    ofFill();
//    image->draw(0,0, width, width);
    ofCircle(0, 0, width);
    ofPopMatrix();
}