#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    //standard oF setup
    ofSetFrameRate(30);
    ofEnableAlphaBlending();
    ofEnableSmoothing();
    
    //networking setup
    //bonjour auto discovery stuff
    bonjour = new ofxBonjourIp();
    bonjour->addEventListeners(this);
    bonjour->startService();
    bonjour->discoverService();
    //osc setup
    oscReceiver.setup(SERVER_PORT);
    
    //narrator setup
    narrator.setup("com.apple.speech.synthesis.voice.Fred");
    
    debugging = true;
    gameStarted = false;
    gameState = GAME_STATE_WAITING;
}

//--------------------------------------------------------------
void testApp::update(){
    //create a container for whatever message we get
 
    while (oscReceiver.hasWaitingMessages()) {
        ofxOscMessage msg;
        oscReceiver.getNextMessage(&msg);
        
        //check for 
        if (msg.getAddress() == "/join" && !gameStarted) {
            string incomingPlayerIP = msg.getRemoteIp();
            string playerName = msg.getArgAsString(0);
            //look for existing players with the same IP
            if (std::find(playerIPs.begin(), playerIPs.end(), incomingPlayerIP) == playerIPs.end()) {
                //no player with that IP address, create a new one
                    int playerID = newPlayer(playerName, incomingPlayerIP);
                
                //join the game
                    joinGame(playerID);
                //wait for the go signal
                //    sendState(incomingPlayerIP, GAME_STATE_WAITING);
                
            }
        }
        
    }
}

//--------------------------------------------------------------
void testApp::draw(){
    
}

//--------------------------------------------------------------
void testApp::sendState(string ip, int state) {
    ofxOscMessage m;
    m.setAddress("/feedback");
    m.addStringArg("state");
    m.addIntArg(state);
    oscSender.setup(ip, CLIENT_PORT);
    m.setRemoteEndpoint(ip, CLIENT_PORT);
    oscSender.sendMessage(m);
}

//--------------------------------------------------------------
void testApp::sendControl(string ip, int control, int playerId) {
    //bug in iOS software, need to revise. currently using JOINED address to change the controls
    ofxOscMessage m;
    m.setAddress("/joined");
    m.addIntArg(playerId);
//we're making everyone on the same team and subteam for now
    m.addIntArg(1);
    m.addIntArg(1);
    
    //this sets the control scheme
    m.addIntArg(control);
    //set them to playing, should be redundant
    m.addIntArg(GAME_STATE_PLAYING);

    oscSender.setup(ip, CLIENT_PORT);
    m.setRemoteEndpoint(ip, CLIENT_PORT);
    oscSender.sendMessage(m);
}


//--------------------------------------------------------------
void testApp::broadcastState(int state) {
    for (int i = 0; i < playerIPs.size(); i++) {
        sendState(playerIPs[i], state);
    }
}

//--------------------------------------------------------------
void testApp::broadcastControl(int control) {
    for (int i = 0; i < playerIPs.size(); i++) {
        sendControl(playerIPs[i], control, players[i].playerId);
        sendState(playerIPs[i], GAME_STATE_PLAYING);
        cout << "Sending " << control << " to " << playerIPs[i] << endl;
    }
    
}


//--------------------------------------------------------------
int testApp::newPlayer(string player, string ipaddress) {
    playerIPs.push_back(ipaddress);
    Player p;
    p.create(player, ipaddress);
    players.push_back(p);
    return (players.size() - 1);
}

//--------------------------------------------------------------
void testApp::joinGame(int playerId) {
    ofxOscMessage msg;
    msg.setAddress("/joined");
    msg.addIntArg(playerId);
    //keep team, subteam and control at
    msg.addIntArg(1);
    msg.addIntArg(1);
    //default state is dragging
    msg.addIntArg(GAME_CONTROL_MOVE);
    msg.addIntArg(GAME_STATE_WAITING);
    oscSender.setup(players[playerId].playerIp, CLIENT_PORT);
    msg.setRemoteEndpoint(players[playerId].playerIp, CLIENT_PORT);
    oscSender.sendMessage(msg);
    cout << "Sending Join Message to " << players[playerId].playerIp << endl;
}

//--------------------------------------------------------------
void testApp::changeGameScene() {
    cout << "Changing Game Scene" << endl;
    switch (gameState) {
        case GAME_STATE_GAME_1_INTRO:
            cout << "I'm playing the intro" << endl;
        
            narrator.speak("36,000 feet below the Earth's surface lay a civilization, removed from the barbaric life of the Ocean [[slnc 1000]] and the even more barbaric life of the human world. [[slnc 2000]] For more than 5 millennia, the people of Atlantis have existed in relative peace and prosperity. [[slnc 3000]] Until today. [[slnc 10000]]");
            broadcastState(GAME_STATE_WAITING);
            break;
        case GAME_STATE_GAME_1:
            cout << "Starting game #1, everyone should have drag controls" << endl;
            broadcastControl(GAME_CONTROL_MOVE);
            break;
        case GAME_STATE_GAME_2_INTRO:
            cout << "Intro to game #2" << endl;
            broadcastState(GAME_STATE_WAITING);
            break;
        case GAME_STATE_GAME_2:
            cout << "Starting game #2, everyone should have tilt controls" << endl;
            broadcastControl(GAME_CONTROL_ACCEL);
            break;
        case GAME_STATE_GAME_3_INTRO:
            cout << "Intro to game #3" << endl;
            broadcastState(GAME_STATE_WAITING);
            break;
        case GAME_STATE_GAME_3:
            cout << "Starting game #3, everyone should have audio controls" << endl;
            broadcastControl(GAME_CONTROL_AUDIO);
            break;
        case GAME_STATE_GAME_4_INTRO:
            cout << "Intro to game #4" << endl;
            broadcastState(GAME_STATE_WAITING);
            break;
        case GAME_STATE_GAME_4:
            cout << "Starting game #4, controls varied. using tap for testing." << endl;
            broadcastControl(GAME_CONTROL_TAP);
            break;
        case GAME_STATE_GAME_5_INTRO:
            cout << "Intro to game #5" <<endl;
            broadcastState(GAME_STATE_WAITING);
            break;
        case GAME_STATE_GAME_5:
            cout << "Starting game #5, everyone has drag controls, need random zombie" << endl;
            broadcastControl(GAME_CONTROL_MOVE);
            break;
        case GAME_STATE_GAME_6_INTRO:
            cout << "Intro to game #6" << endl;
            broadcastState(GAME_STATE_WAITING);
            break;
        case GAME_STATE_GAME_6:
            cout << "Starting game #6, slowly get audio controls" << endl;
            broadcastControl(GAME_CONTROL_AUDIO);
            break;
        case GAME_STATE_GAME_OVER:
            cout << "Game is now over" << endl;
            broadcastState(GAME_STATE_WAITING);
            break;
    }
}


//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if (key == ' ' && !gameStarted) {
        gameStarted = true;
        gameState = GAME_STATE_GAME_1_INTRO;
        changeGameScene();
    }
    else {
        gameState++;
        if (gameState > GAME_STATE_GAME_OVER) {
            gameState = GAME_STATE_WAITING;
            gameStarted = false;
        }
        changeGameScene();
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void testApp::onPublishedService(const void* sender, string &serviceIp) {
    ofLog() << "Received published service event: " << serviceIp;
}

void testApp::onDiscoveredService(const void* sender, string &serviceIp) {
    ofLog() << "Received discovered service event: " << serviceIp;
}

void testApp::onRemovedService(const void* sender, string &serviceIp) {
    ofLog() << "Received removed service event: " << serviceIp;
}
