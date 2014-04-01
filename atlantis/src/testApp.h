#pragma once

#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxOsc.h"
#include "ofxBonjourIp.h"
#include "player.h"
#include "ofxVoiceSynthesizer.h"
#include "food.h"
#include "attacker.h"
#include "zombie.h"
#include "humanoid.h"

#define SERVER_PORT 9000
#define CLIENT_PORT 9001

#define GAME_STATE_WAITING          0
#define GAME_STATE_PAUSED           1
#define GAME_STATE_PLAYING          2

#define GAME_STATE_GAME_1_INTRO     3
#define GAME_STATE_GAME_1           4
#define GAME_STATE_GAME_2_INTRO     5
#define GAME_STATE_GAME_2           6
#define GAME_STATE_GAME_3_INTRO     7
#define GAME_STATE_GAME_3           8
#define GAME_STATE_GAME_4_INTRO     9
#define GAME_STATE_GAME_4           10
#define GAME_STATE_GAME_5_INTRO     11
#define GAME_STATE_GAME_5           12
#define GAME_STATE_GAME_6_INTRO     13
#define GAME_STATE_GAME_6           14
#define GAME_STATE_GAME_OVER        15

#define GAME_CONTROL_MOVE           0
#define GAME_CONTROL_AUDIO          1
#define GAME_CONTROL_ACCEL          2
#define GAME_CONTROL_TAP            3

#define AMOUNT_OF_FOOD              50
#define AMOUNT_OF_ATTACKERS         10
#define AMOUNT_OF_ZOMBIES           200


class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

    
    //OPENFRAMEWORKS EVENTS
    
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    //FOR BONJOUR
    
        void onPublishedService(const void* sender, string &serviceIp);
        void onDiscoveredService(const void* sender, string &serviceIp);
        void onRemovedService(const void* sender, string &serviceIp);

    
    //ADDED
        //testing
        bool debugging;
    
    //networking and communication
        ofxOscReceiver oscReceiver;
        ofxOscSender oscSender;
        ofxBonjourIp *bonjour;
        vector<string> playerIPs;
    
        void sendState(string ip, int state);
        void sendControl(string ip, int control, int playerId);
        void broadcastState(int state);
        void broadcastControl(int control);
    
    //game specific
        vector<Player> players;
        bool gameStarted;
        int gameState;
        ofxVoiceSynthesizer narrator;
        ofxBox2d    box2d;			  //	the box2d world

    
    //game #1
        vector <ofPtr<Food> > food;
        vector <ofPtr<Humanoid> > humanoids;
        void newFood();
        void newHumanoid();

    //game #2
        vector <ofPtr<Attacker> > attackers;
        void newAttacker();

    //game #4
    vector <ofPtr<Zombie> > zombies;
    void newZombie();

    
    //game functions
        int newPlayer(string player, string ip);
        void joinGame(int playerId);
        void changeGameScene();
		
};
