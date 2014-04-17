//
//  chain.cpp
//  atlantis
//
//  Created by Clay Ewing on 3/30/14.
//
//

#include "chain.h"

void Chain::create(float x, float y, int numLinks, ofxBox2d *b2w) {
    //10 = players per team
    position.set(x,y);
    b2d = b2w;

    for (int i = 0; i < numLinks -1; i++) {
        ofPtr<Pod> pod = ofPtr<Pod>(new Pod);
        pod.get()->setPhysics(1, .1, .1);
        pod.get()->setVelocity(0, -3);
        pod.get()->setup(b2d->world, x, y + (i*40), 10, 10);
        pod.get()->init();
        //       a.get()->setData(new CustomData());
        //       a.get()->setupCustom(attackers.size());

		pods.push_back(pod);
	}
	
	// now connect each circle with a joint
	for (int i=1; i<pods.size(); i++) {
		
		ofPtr<ofxBox2dJoint> joint = ofPtr<ofxBox2dJoint>(new ofxBox2dJoint);
        joint.get()->setup(b2d->world, pods[i-1].get()->body, pods[i].get()->body);
		joint.get()->setLength(40);
		joints.push_back(joint);
	}

}

void Chain::display() {
    
    ofSetColor(255, 0, 0, 255);
    for (int i = 0; i < pods.size(); i++) {
        pods[i].get()->setVelocity(0, -1);
        ofPushMatrix();
        ofSetRectMode(OF_RECTMODE_CENTER);
        ofTranslate(pods[i].get()->getPosition());
        ofRotateZ(pods[i].get()->getRotation());
        ofSetColor(color);
        //    image->draw(0,0, width, height);
//        ofCircle(0, 0, 10);
        ofRect(0, 0, 10, 10);
        ofSetRectMode(OF_RECTMODE_CORNER);
        ofPopMatrix();
    }

}