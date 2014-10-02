//
//  chain.cpp
//  atlantis
//
//  Created by Clay Ewing on 3/30/14.
//
//

#include "chain.h"

void Chain::create(float x, float y, ofxBox2d *b2w) {
    //10 = players per team
    position.set(x,y);
    b2d = b2w;
    color = ofColor(ofRandom(255), ofRandom(255), ofRandom(255));
}

void Chain::newPod(string ip) {
    //add ip for control
    playerIps.push_back(ip);
    //last pod * radius
    float y = position.y + (pods.size() * 40);
    ofPtr<Pod> pod = ofPtr<Pod>(new Pod);
    pod.get()->setPhysics(.5, 1, .1);
    pod.get()->setup(b2d->world, position.x, y, 10, 10);
    pod.get()->setupCustom();
    pod.get()->init();
    pod.get()->image = image;
//    pod.get()->passenger = passengerImage;

    pods.push_back(pod);

    if (pods.size() > 1) {
        //connect pod with a joint to the last one
		ofPtr<ofxBox2dJoint> joint = ofPtr<ofxBox2dJoint>(new ofxBox2dJoint);
        joint.get()->setup(b2d->world, pods[pods.size() -2].get()->body, pods[pods.size()-1].get()->body);
		joint.get()->setLength(40);
		joints.push_back(joint);
    }
    
}

void Chain::display() {
    
    for (int i = 0; i < pods.size(); i++) {
        ofSetColor(color);
        pods[i].get()->display();
    }

}