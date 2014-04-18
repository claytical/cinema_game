#pragma once

#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxOsc.h"
#include "ofxBonjourIp.h"
#include "player.h"
#include "ofxVoiceSynthesizer.h"
#include "ofxCenteredTrueTypeFont.h"
#include "food.h"
#include "humanoid.h"

#define SERVER_PORT 9000
#define CLIENT_PORT 9001

#define GAME_STATE_WAITING          0
#define GAME_STATE_PAUSED           1
#define GAME_STATE_PLAYING          2
#define GAME_STATE_SHOW_SCORE       3

#define GAME_CONTROL_MOVE           0
#define GAME_CONTROL_AUDIO          1
#define GAME_CONTROL_ACCEL          2
#define GAME_CONTROL_TAP            3

#define AMOUNT_OF_FOOD              50

#define IMAGE_SET_SQUARE    0
#define IMAGE_SET_ABSTRACT  1
#define IMAGE_SET_HUMANS    2


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

    //FOR BOX2d
    
    
        void contactStart(ofxBox2dContactArgs &e);
        void contactEnd(ofxBox2dContactArgs &e);

    //ADDED
        //testing
        bool debugging;
    
    //networking and communication
        ofxOscReceiver oscReceiver;
        ofxOscSender oscSender;
        ofxBonjourIp *bonjour;
        vector<string> playerIPs;
    
        void sendState(string ip, int state);
        void sendControl(string ip, int control);
        void sendScore(string ip, int score);
        void broadcastState(int state);
        void broadcastControl(int control);
        void broadcastScores();
        int removeExistingPlayer(string ip);
        void setPlayerAlive(string ip);
    
    //game specific
        vector<Player> players;
        bool gameStarted;
        int gameState;
        int winnerID;
        float gameTimer;
        float timeUntilNextGame;
        ofxVoiceSynthesizer narrator;
        ofxBox2d    box2d;			  //	the box2d world
        ofxCenteredTrueTypeFont timerText;
        vector<ofImage> playerImages;
        ofImage plankton;
        int imageCounter;
        ofSoundPlayer backgroundTrack;
        ofSoundPlayer collectFx;
    //game #1
        vector <ofPtr<Food> > food;
        vector <ofPtr<Humanoid> > humanoids;
        void newFood();
        void newHumanoid(string ip);

    
    //game functions
        int newPlayer(string player, string ip);
        void joinGame(string ip);
        void startGame();
        void resetConnections();
        int whoWon();
		
};
