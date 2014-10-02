#pragma once
#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxOsc.h"
#include "ofxBonjourIp.h"
#include "ofxUI.h"
#include "player.h"
#include "invader.h"
#include "food.h"

/***************************************************************
 CINEMA GAME Display Server
 
 created by the NERDLab at University of Miami
 ***************************************************************/


// -------------------------------------------------

#define NUMBER_OF_TEAMS     5
#define PLAYERS_PER_TEAM    10

#define GAME_STATE_WAITING  0
#define GAME_STATE_PAUSED   1
#define GAME_STATE_PLAYING  2
#define GAME_CONTROL_MOVE   0
#define GAME_CONTROL_AUDIO  1
#define GAME_CONTROL_ACCEL  2
#define GAME_CONTROL_TAP    3

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
    void broadcastFeedback(string control, int param);
    
    // Parse an OscMessage into a string for easy logging
    string getOscMsgAsString(ofxOscMessage m);
    
    void onPublishedService(const void* sender, string &serviceIp);
    void onDiscoveredService(const void* sender, string &serviceIp);
    void onRemovedService(const void* sender, string &serviceIp);
    void gotMessage(ofMessage msg);
    void guiEvent(ofxUIEventArgs &e);
    void exit();
    
    void newPlayer(string pname);
    void newFood();
    void newInvader();
    int whoWon();
    
    ofxUICanvas *mainMenu;
    ofxUICanvas *gameoverMenu;
        
    ofImage logo;
    ofImage plankton;
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
    
    ofxBonjourIp* bonjour;
    
    
	ofxBox2d                            box2d;			  //	the box2d world
    vector <ofPtr<Player> > players;
    vector <ofPtr<Food> > food;
    vector <ofPtr<Invader> > invaders;
    ofTrueTypeFont largeGameText;
    ofTrueTypeFont gameText;
    float gameTimer;
    int playersLeft;
    ofImage playerImages[10];
    int gameState;
    bool gamePaused;
    int winner;

    
};

