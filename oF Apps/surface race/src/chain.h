//
//  chain.h
//  atlantis
//
//  Created by Clay Ewing on 3/30/14.
//
//

#include "ofMain.h"
#include "pod.h"
#include "ofxBox2dJoint.h"
#include "ofxBox2d.h"

class Chain {
    
public:
    void display();
    void create(float x, float y, ofxBox2d *b2d);
    void newPod(string ip);
    ofColor color;
    ofImage *image;
    ofImage *passengerImage;
    ofxBox2d *b2d;
    ofPoint position;
    ofPoint velocity;
    vector      <string>                    playerIps;
    vector		<ofPtr<Pod> >               pods;		  //	default box2d circles
	vector		<ofPtr<ofxBox2dJoint> >		joints;			  //	joints

};
