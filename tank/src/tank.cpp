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
void Tank::shoot(ofxBox2d *b2d) {
    ofPtr<Shot> shot = ofPtr<Shot>(new Shot);

    shot.get()->setup(b2d->getWorld(), getPosition().x + ofRandom(-30, 30), getPosition().y + ofRandom(-30, 30), 5);
    shot.get()->setPhysics(2, .1, 1);
    shot.get()->velocity.set(cos(-1), sin(-1));
    shot.get()->setVelocity(cos(-1), sin(-1));
    float radians = getRotation() * PI /2;
    shot.get()->setData(new CustomData());
    shot.get()->setupCustom(shots.size());

    shot.get()->color = ofColor(255,0,0);
 //   cout << "X Velocity: " << shot.get()->getVelocity().x << endl;
 //   cout << "Y Velocity: " << shot.get()->getVelocity().y << endl;

    shots.push_back(shot);
    //    shot.setVelocity(get, <#float y#>)

//    t.get()->setVelocity(0,0);
    
//    t.get()->setData(new CustomData());
//    t.get()->setupCustom(tanks.size());

    
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
//    cout << shots.size() << " shots taken" << endl;
    
}