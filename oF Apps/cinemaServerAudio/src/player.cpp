//
//  player.cpp
//  example-Simple
//
//  Created by Clay Ewing on 2/8/14.
//
//

#include "player.h"

void Player::setupCustom(int id) {
    setData(new CustomData());
    CustomData * theData = (CustomData *)getData();
    theData->type = PLAYER_TYPE;
    theData->remove = false;
    theData->id = id;
    score = 0;
    hit = false;
}
void Player::display() {
    float width = getWidth();
    float height = getHeight();
    ofPushMatrix();
    ofSetRectMode(OF_RECTMODE_CENTER);
    ofTranslate(getPosition());
    ofRotateZ(getRotation());
    if (!hit) {
        ofSetColor(color, 255);
    }
    else {
        ofSetColor(10, 10, 10);
    }
    ofFill();
    image->draw(0, 0, width, height);
    ofSetRectMode(OF_RECTMODE_CORNER);
    ofPopMatrix();
    
}