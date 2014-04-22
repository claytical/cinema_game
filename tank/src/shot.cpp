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
    theData->type = TYPE_BULLET;
    theData->remove = false;
    theData->id = id;
    color = ofColor(ofRandom(100, 200), 200);
   //    setVelocity(-5, 5);
}
void Shot::display() {
    
    ofPushMatrix();
    ofTranslate(getPosition());
    ofRotateZ(getRotation());
    ofSetColor(color);
    ofFill();
    //    image->draw(0,0, width, height);
//    ofRect(0, 0, width, height);
    ofCircle(0, 0, getRadius());
    ofPopMatrix();
    
}