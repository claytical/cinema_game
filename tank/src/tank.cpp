//
//  tank.cpp
//  tanks
//
//  Created by Clay Ewing on 4/18/14.
//
//

#include "tank.h"

void Tank::setupCustom(int id) {
    setData(new CustomData());
    CustomData * theData = (CustomData *)getData();
    theData->type = TYPE_HUMANOID;
    theData->remove = false;
    theData->id = id;
}

void Tank::display() {
    //#1 moves
    
    //#2 rotates
    if (ips[1].empty()) {
        //no player 2
    }

    //#3 shoots

    if (ips[2].empty()) {
        //no player 3
    }

    float width = getWidth();
    float height = getHeight();
    ofPushMatrix();
    ofSetRectMode(OF_RECTMODE_CENTER);
    ofTranslate(getPosition());
    ofRotateZ(getRotation());
    ofSetColor(color);
    ofFill();
    //image->draw(0,0, width, height);
    ofRect(0, 0, width, height);
    ofSetRectMode(OF_RECTMODE_CORNER);
    ofPopMatrix();
}