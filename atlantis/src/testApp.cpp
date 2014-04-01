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

    //box2d setup
    box2d.init();
	box2d.setGravity(0, 0);
    box2d.enableEvents();
	box2d.setFPS(30.0);
    
    
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
    
    
    switch(gameState) {
        case GAME_STATE_GAME_1:
            box2d.update();
            if (food.size() < AMOUNT_OF_FOOD) {
                for (int i = food.size(); i < AMOUNT_OF_FOOD; i++) {
                    newFood();
                }
            }
            
            
            break;
        case GAME_STATE_GAME_2:
            box2d.update();
            if (attackers.size() < AMOUNT_OF_ATTACKERS) {
                for (int i = attackers.size(); i < AMOUNT_OF_ATTACKERS; i++) {
                    newAttacker();
                }
            }
            break;
        case GAME_STATE_GAME_4:
            box2d.update();
            if (zombies.size() < AMOUNT_OF_ZOMBIES) {
                for (int i = zombies.size(); i < AMOUNT_OF_ZOMBIES; i++) {
                    newZombie();
                }
            }
            break;
    }
}

//--------------------------------------------------------------
void testApp::draw(){
    switch(gameState) {
        case GAME_STATE_GAME_1:
            ofSetColor(255,255,255);
            
            for (int i = 0; i < humanoids.size(); i++) {
                humanoids[i].get()->display();
            }
            
            for (int i = 0; i < food.size(); i++) {
                food[i].get()->display();
            }

            // draw the ground
            box2d.drawGround();
            break;
        case GAME_STATE_GAME_2:
            ofSetColor(255,255,255);
            
            
            for (int i = 0; i < attackers.size(); i++) {
                attackers[i].get()->display();
            }
            // draw the ground
            box2d.drawGround();
            break;
            
        case GAME_STATE_GAME_4:
            ofSetColor(255,255,255);
            
            
            for (int i = 0; i < zombies.size(); i++) {
                zombies[i].get()->display();
            }
            // draw the ground
            box2d.drawGround();
            break;

    }
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
    m.addIntArg(1); //TEAM
    m.addIntArg(1); //SUBTEAM
    
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
            //
            
            cout << "I'm playing the intro" << endl;
        
            narrator.speak("36,000 feet below the Earth's surface lay a civilization, removed from the barbaric life of the Ocean [[slnc 1000]] and the even more barbaric life of the human world. [[slnc 2000]] For more than 5 millennia, the people of Atlantis have existed in relative peace and prosperity. [[slnc 1000]] Until today. [[slnc 10000]]");
            broadcastState(GAME_STATE_WAITING);
            break;
        case GAME_STATE_GAME_1:
            cout << "Starting game #1, everyone should have drag controls" << endl;
            //create humanoids for each player
            for (int i = 0; i < players.size(); i++) {
                newHumanoid();
            }
            
            broadcastControl(GAME_CONTROL_MOVE);
            break;
        case GAME_STATE_GAME_2_INTRO:
            //clean up game #1
            food.clear();

            narrator.speak("Most of you have made it to your surface vehicles. But a few souls are lost, now as crazed as the attacking sea life. They are bent on destroying their former neighbors and friends. Your vehicle team must work together to navigate through the depths, avoiding contact with these deadly creatures that threaten to destroy your only mode of transportation.");
            cout << "Intro to game #2" << endl;
            broadcastState(GAME_STATE_WAITING);
            break;
        case GAME_STATE_GAME_2:
            
            cout << "Starting game #2, everyone should have tilt controls" << endl;
            broadcastControl(GAME_CONTROL_ACCEL);
            break;
        case GAME_STATE_GAME_3_INTRO:
            attackers.clear();
            narrator.speak("In the distance is a building still standing. Fortified enough to shield your from attack, large enough for everyone to seek shelter. You must propel yourselves to safety. Take turns blowing into your phone, one-at-a-time, in the order you are lined up. ");
            cout << "Intro to game #3" << endl;
            broadcastState(GAME_STATE_WAITING);
            break;
        case GAME_STATE_GAME_3:
            cout << "Starting game #3, everyone should have audio controls" << endl;
            broadcastControl(GAME_CONTROL_AUDIO);
            break;
        case GAME_STATE_GAME_4_INTRO:
            narrator.speak("Inside the warehouse, you find a stockpile of human firearms. Large, powerful guns that need three people to operate them. One to move. One to aim. One to fire.");
            cout << "Intro to game #4" << endl;
            broadcastState(GAME_STATE_WAITING);
            break;
        case GAME_STATE_GAME_4:
            cout << "Starting game #4, controls varied. using tap for testing." << endl;
            broadcastControl(GAME_CONTROL_TAP);
            break;
        case GAME_STATE_GAME_5_INTRO:
            zombies.clear();
            cout << "Intro to game #5" <<endl;
            narrator.speak("Avoid anyone who is red, they are infected. Stay alive, [[slnc 1000]] if you can.");
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
            narrator.speak("Congratulations, you've saved the Atlantian race. Everyone is cured.");
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

//-----------GAME #1 FUNCTIONS

void testApp::newFood() {
    ofPtr<Food> f = ofPtr<Food>(new Food);
    f.get()->setPhysics(.5, 1, 1);
    
    float fSize = ofRandom(.1, .5);
    int planktonNumber = int(ofRandom(5));
    // f.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), plankton[planktonNumber].width* fSize, plankton[planktonNumber].height * fSize);
    f.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), 50 * fSize, 50 * fSize);
    
    f.get()->setVelocity(ofRandom(-2,2), ofRandom(-2,2));
    //f.get()->image = &plankton[planktonNumber];
    
    f.get()->setData(new CustomData());
    f.get()->setupCustom(food.size());
    food.push_back(f);
    
}

void testApp::newHumanoid() {
    ofPtr<Humanoid> h = ofPtr<Humanoid>(new Humanoid);
    h.get()->setPhysics(.5, 1, 1);
    
    // f.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), plankton[planktonNumber].width* fSize, plankton[planktonNumber].height * fSize);
    h.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), 50, 50);
    
    h.get()->setVelocity(0,0);
    //f.get()->image = &plankton[planktonNumber];
    
    h.get()->setData(new CustomData());
    h.get()->setupCustom(humanoids.size());
    humanoids.push_back(h);
    
}


//-----------GAME #2 FUNCTIONS

void testApp::newAttacker() {
    ofPtr<Attacker> a = ofPtr<Attacker>(new Attacker);
    a.get()->setPhysics(.5, 1, 1);
    
    float aSize = ofRandom(.2, .3);
    int planktonNumber = int(ofRandom(5));
    a.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(0, -ofGetHeight()), 50 * aSize, 50 * aSize);
    
    a.get()->setVelocity(0, ofRandom(1,4));
    //f.get()->image = &plankton[planktonNumber];
    
    a.get()->setData(new CustomData());
    a.get()->setupCustom(attackers.size());
    attackers.push_back(a);
    
}


//-----------GAME #4 FUNCTIONS

void testApp::newZombie() {
    ofPtr<Zombie> z = ofPtr<Zombie>(new Zombie);
    z.get()->setPhysics(.5, 1, 1);
    
    float zSize = ofRandom(.2, .3);
    int planktonNumber = int(ofRandom(5));
    z.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(0, -ofGetHeight()), 50 * zSize, 50 * zSize);
    
    z.get()->setVelocity(ofRandom(.1,-.1), ofRandom(0,1));
    //f.get()->image = &plankton[planktonNumber];
    
    z.get()->setData(new CustomData());
    z.get()->setupCustom(attackers.size());
    zombies.push_back(z);
    
}
