#pragma once
#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxOsc.h"
#include "ofxBonjourIp.h"
#include "player.h"

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
    //----------------------------------------
    // Client side:
    
    ofxOscSender clientSender; // all-important ofxOscSender object
    string clientDestination; // IP address we're sending to
    int clientSendPort; // port we're sending to
    string clientTyping; // what we're going to send: some stuff you typed
    
    ofxOscReceiver clientReceiver; // OSC receiver
    int clientRecvPort; // port where we listen for messages
    string clientMessages; // string containing the received messages for display
    
    //----------------------------------------
    // Server side:
    ofxOscReceiver serverReceiver; // OSC receiver
    int serverRecvPort; // port we're listening on: must match port from sender!
    string serverTyping; //messages you've received from the clientes
    
    // Message display variables
    vector<string>serverMessages; //vector containing the received messages for display
    unsigned int maxServerMessages; //nr of messages fitting on the screen
    
    vector<string>knownClients; //collected IP's of chat participants
    ofxOscSender serverSender;
    
    //Distribute a received message among the known hosts
    void broadcastReceivedMessage(string chatmessage);
    
    // Parse an OscMessage into a string for easy logging
    string getOscMsgAsString(ofxOscMessage m);
    
	ofxBox2d                            box2d;			  //	the box2d world
//	vector    <ofPtr<ofxBox2dCircle> >	circles;		  //	default box2d circles
//	vector	  <ofPtr<ofxBox2dRect> >	boxes;			  //	defalut box2d rects

	ofxBonjourIp* bonjour;
    vector <ofPtr<Player> > players;
    ofTrueTypeFont largeGameText;
    ofImage playerImages[5];
    
};

