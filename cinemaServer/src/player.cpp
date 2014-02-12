//
//  player.cpp
//  example-Simple
//
//  Created by Clay Ewing on 2/8/14.
//
//

#include "player.h"

void Player::display() {
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