#pragma once
#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxOsc.h"
#include "ofxBonjourIp.h"
#include "player.h"
#include "food.h"

/***************************************************************
 CINEMA GAME Display Server
 
 created by the NERDLab at University of Miami
 ***************************************************************/


// -------------------------------------------------

class testApp : public ofBaseApp {
	
public:
	
	void setup();
	void update();
	void draw();
	
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void resized(int w, int h);

	void contactStart(ofxBox2dContactArgs &e);
	void contactEnd(ofxBox2dContactArgs &e);

    //Distribute a received message among the known hosts
    void broadcastReceivedMessage(string chatmessage);
    
    // Parse an OscMessage into a string for easy logging
    string getOscMsgAsString(ofxOscMessage m);
    
    
    void newPlayer(string pname);
    void newFood();
    
    
    //----------------------------------------
    // Client for sending messages to players
    
    vector<string>knownClients; //collected IP's of players
    ofxOscSender serverSender;
    
    ofxOscReceiver clientReceiver; // OSC receiver
    int clientRecvPort; // port where we listen for messages
    
    //----------------------------------------
    // Server for receiving messages from players
    
    ofxOscReceiver serverReceiver; // OSC receiver
    int serverRecvPort; // port we're listening on: must match port from sender!
    
    
    
    
	ofxBox2d                            box2d;			  //	the box2d world
    vector <ofPtr<Player> > players;
    vector <ofPtr<Food> > food;
    ofTrueTypeFont largeGameText;
    ofImage playerImages[5];
    
};

