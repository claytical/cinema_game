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
    void create(float x, float y, int numberOfLinks, ofxBox2d *b2d);
    ofColor color;
    ofImage *image;
    ofxBox2d *b2d;
    ofPoint position;
    ofPoint velocity;
    vector		<ofPtr<Pod> >      pods;		  //	default box2d circles
	vector		<ofPtr<ofxBox2dJoint> >		joints;			  //	joints

    //players assigned part of chain
    //need vector of int for control look up. chainMap[playerID] = chainID
};
