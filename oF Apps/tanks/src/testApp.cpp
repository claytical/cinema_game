#include "testApp.h"

int testApp::removeExistingPlayer(string ip) {
    for (int i = 0; i < players.size(); i++) {
        if(players[i].playerIp == ip) {
            cout << "Found Existing Player";
            players.erase(players.begin()+i);
            break;
        }
    }
}

void testApp::setPlayerAlive(string ip) {
    for (int i = 0; i < players.size(); i++) {
        if(players[i].playerIp == ip) {
            cout << "Found Existing Player";
            players[i].connected = true;
            break;
        }
    }
    
}
static bool removeDeadPlayer(Player p) {
    if (!p.connected) {
        cout << "found unconnected player" << endl;
        return true;
    }
    return false;
}
static bool removeShape(ofPtr<ofxBox2dBaseShape> shape) {
    CustomData *custom = (CustomData *)shape->getData();
    return custom->remove;
}

static bool removeTank(ofPtr<Tank> tank) {
    return (tank->armor <= 0);
}

//--------------------------------------------------------------
void testApp::setup(){
    ofSetFrameRate(30);
    ofEnableAlphaBlending();
    ofEnableSmoothing();
    ofBackground(10);
    collectFx.loadSound("collect.wav");
    backgroundTrack.loadSound("background.wav");
    backgroundTrack.setLoop(true);
    backgroundTrack.play();
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
    box2d.createBounds();

    //game setup
    
    ofDirectory dir("tanks");
    dir.allowExt("png");
    dir.sort();
    dir.listDir();
    playerImages.clear();
    cout << "cleared current images" << endl;
    imageCounter = 0;

    for (int i = 0; i < dir.numFiles(); i++) {
        ofImage img;
        img.loadImage(dir.getFile(i));
        playerImages.push_back(img);
        cout << "creating new image with " << i << endl;
    }
    debugging = true;
    gameStarted = false;
    gameState = GAME_STATE_WAITING;
    gameTimer = 999999999;
    timeUntilNextGame = 5000;
    timerText.loadFont("joystix.ttf", 48);
    }

//--------------------------------------------------------------
void testApp::update(){
    for (int i = 0; i < tanks.size(); i++) {
        ofRemove(tanks[i]->shots, removeShape);
    }
    ofRemove(tanks, removeTank);
    //create a container for whatever message we get
    box2d.update();
    while (oscReceiver.hasWaitingMessages()) {
        ofxOscMessage msg;
        oscReceiver.getNextMessage(&msg);
        
        //check for
        if (msg.getAddress() == "/alive") {
            setPlayerAlive(msg.getRemoteIp());
        }
        if (msg.getAddress() == "/join") {
            string incomingPlayerIP = msg.getRemoteIp();
            string playerName = msg.getArgAsString(0);
            //look for existing players with the same IP
            if (std::find(playerIPs.begin(), playerIPs.end(), incomingPlayerIP) == playerIPs.end()) {

                //no player with that IP address, create a new one
                int playerID = newPlayer(playerName, incomingPlayerIP);
                //join the game
                joinGame(incomingPlayerIP);
                //newHumanoid(incomingPlayerIP);
            }
            else {
                //wants to rejoin

                int removedPlayerID = removeExistingPlayer(incomingPlayerIP);
                //create a new player
                int playerID = newPlayer(playerName, incomingPlayerIP);
                joinGame(incomingPlayerIP);
            }
        }
        
        if (msg.getAddress() == "/quit") {
            removeExistingPlayer(msg.getRemoteIp());
        }
        
        if (msg.getAddress() == "/rotate") {
            if (msg.getNumArgs() == 2) {
                int tankNumber = msg.getArgAsInt32(0);
                if (tanks.size() > tankNumber) {
                    float rotation = msg.getArgAsFloat(1);
                    tanks[tankNumber].get()->setRotation(rotation);
                }
            }
        }
        
        if (msg.getAddress() == "/move") {
            if (msg.getNumArgs() == 3) {
                int tankNumber = msg.getArgAsInt32(0);
                if (tanks.size() > tankNumber) {
                    float xSpeed = msg.getArgAsFloat(1) * -.01;
                    float ySpeed = msg.getArgAsFloat(2) * -.01;
                    tanks[tankNumber].get()->setVelocity(xSpeed, ySpeed);
                }
            }
        }
        if (msg.getAddress() == "/tap") {
            int tankNumber = msg.getArgAsInt32(0);
            if (tanks.size() > tankNumber) {
                tanks[tankNumber].get()->shoot();
            }
        }
        
    }

    if (tanks.size() <= 1 && gameStarted) {
        //last one standing
        winningTeam = whoWon();
        broadcastScores();
        broadcastState(GAME_STATE_WAITING);
        timeUntilNextGame = ofGetElapsedTimef() + 30;
        gameStarted = false;
        ofRemoveListener(box2d.contactStartEvents, this, &testApp::contactStart);
        ofRemoveListener(box2d.contactEndEvents, this, &testApp::contactEnd);

    }

    //restart game automatically
    if (!gameStarted && ofGetElapsedTimef() > timeUntilNextGame) {
        startGame();

    }
}
//--------------------------------------------------------------
void testApp::draw(){
    for (int i = 0; i < tanks.size(); i++) {
        tanks[i].get()->display();
        for (int j = 0; j < tanks[i]->shots.size(); j++) {
            tanks[i]->shots[j]->display();
        }
    }
    
    // draw the ground
    string timeLeft = ofToString(abs(int(ofGetElapsedTimef() - gameTimer)));
    string timeTil = ofToString(abs(int(ofGetElapsedTimef() - timeUntilNextGame)));
    if (gameStarted) {
        if (ofGetElapsedTimef() < gameTimer - 28) {
            ofSetColor(0);
            timerText.drawStringCentered("destroy!", ofGetWidth()/2-2, ofGetHeight()/2-2);
            ofSetColor(255);
            timerText.drawStringCentered("destroy!", ofGetWidth()/2, ofGetHeight()/2);
            
        }
//        ofSetColor(0);
//        timerText.drawString(timeLeft, ofGetWidth() - timerText.stringWidth(timeLeft) - 18, timerText.getLineHeight() + 18);
//        ofSetColor(255);
//        timerText.drawString(timeLeft, ofGetWidth() - timerText.stringWidth(timeLeft) - 20, timerText.getLineHeight() + 20);

    }
    else {
        //team winner? display all team member names?
        if (!winningTeam.empty()) {
            ofSetColor(0);
            timerText.drawStringCentered(winningTeam, ofGetWidth()/2 - 2, ofGetHeight()/3 - 2);
            timerText.drawStringCentered("Next Game: " + timeTil + " seconds", ofGetWidth()/2 - 2, ofGetHeight()/1.5 - 2);
            timerText.drawStringCentered(bonjour->getDeviceIp(), ofGetWidth()/2 -2, ofGetHeight()/1.1 -2);

            ofSetColor(255);
            timerText.drawStringCentered(winningTeam, ofGetWidth()/2, ofGetHeight()/3);
            timerText.drawStringCentered("Next Game: " + timeTil + " seconds", ofGetWidth()/2, ofGetHeight()/1.5);
            timerText.drawStringCentered(bonjour->getDeviceIp(), ofGetWidth()/2, ofGetHeight()/1.1);

        }
        else {
            ofSetColor(0);
            timerText.drawStringCentered("Press Space to Start", ofGetWidth()/2 - 2, ofGetHeight()/2 - 2);
            timerText.drawStringCentered(bonjour->getDeviceIp(), ofGetWidth()/2 -2, ofGetHeight()/1.5 -2);
            ofSetColor(255);
            timerText.drawStringCentered("Press Space to Start", ofGetWidth()/2, ofGetHeight()/2);
            timerText.drawStringCentered(bonjour->getDeviceIp(), ofGetWidth()/2, ofGetHeight()/1.5);
            
        }
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
    m.clear();
    m.setAddress("set");
    m.addStringArg("state");
    m.addIntArg(state);
    oscSender.sendMessage(m);
    m.clear();

    //set the playing variable as well
    if (state == GAME_STATE_PLAYING) {
        m.setAddress("set");
        m.addStringArg("playing");
        m.addIntArg(1);
        oscSender.sendMessage(m);
    }
    
}

void testApp::broadcastScores() {
    broadcastState(GAME_STATE_SHOW_SCORE);
    for (int i = 0; i < players.size(); i++) {
        sendScore(players[i].playerIp, players[i].score);
    }

}

void testApp::sendInstructions(string ip, string text) {
    ofxOscMessage m;
    m.setAddress("set");
    m.addStringArg("instructions");
    m.addStringArg(text);
    oscSender.setup(ip, CLIENT_PORT);
    m.setRemoteEndpoint(ip, CLIENT_PORT);
    oscSender.sendMessage(m);
}
void testApp::sendScore(string ip, int score) {
    ofxOscMessage m;
    m.setAddress("set");
    m.addStringArg("score");
    m.addIntArg(score);
    oscSender.setup(ip, CLIENT_PORT);
    m.setRemoteEndpoint(ip, CLIENT_PORT);
    oscSender.sendMessage(m);
}

//--------------------------------------------------------------
void testApp::sendControl(string ip, int control) {
    ofxOscMessage m;
    m.setAddress("set");
    m.addStringArg("control");
    m.addIntArg(control);

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
        sendState(playerIPs[i], GAME_STATE_PLAYING);
        sendControl(playerIPs[i], control);
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
void testApp::resetConnections() {
    for (int i = 0; i < players.size(); i++) {
        players[i].connected = false;
        ofxOscMessage m;
        m.setAddress("/reset");
        m.addIntArg(0);
        oscSender.setup(players[i].playerIp, CLIENT_PORT);
        m.setRemoteEndpoint(players[i].playerIp, CLIENT_PORT);
        oscSender.sendMessage(m);
    }
}

//--------------------------------------------------------------
void testApp::joinGame(string ip) {
    ofxOscMessage msg;

    //set player ID
    msg.setAddress("set");
    msg.addStringArg("id");
    msg.addIntArg(players.size()-1);
    oscSender.setup(ip, CLIENT_PORT);
    msg.setRemoteEndpoint(ip, CLIENT_PORT);
    oscSender.sendMessage(msg);
    msg.clear();
    //what image set to use
    msg.setAddress("set");
    msg.addStringArg("images");
    msg.addIntArg(IMAGE_SET_TANKS);
    oscSender.sendMessage(msg);
    msg.clear();
    //what game we're playing
    msg.setAddress("set");
    msg.addStringArg("game");
    msg.addStringArg("tanks");
    oscSender.sendMessage(msg);
    msg.clear();
/*
    //give some instructions while they wait
    msg.setAddress("set");
    msg.addStringArg("instructions");
    msg.addStringArg("when the game starts\ndrag your avatar around to\nfeed on the plankton");
    oscSender.sendMessage(msg);
    msg.clear();

*/
    //now wait until we're ready to play
    sendState(ip, GAME_STATE_WAITING);
 }

//--------------------------------------------------------------
void testApp::startGame() {
    //create all players

    for (int i = 0; i < tanks.size(); i++) {
        tanks[i]->destroy();
    }

    tanks.clear();
    imageCounter = 0;
    cout << "I've got " << players.size() << " players" << endl;
    for (int i = 0; i < players.size(); i+=3) {
        cout << "I'm in a loop!" << endl;
        //0, 3, 6,
        //0,1,2
        //3,4,5
        //6,7,8
        vector<string> ips;
        ips.push_back(players[i].playerIp);
        if (i + 1 < players.size()) {
            cout << "second player control" << endl;
            ips.push_back(players[i+1].playerIp);
        }
        if (i + 2 < players.size()) {
            cout << "third player control" << endl;
            ips.push_back(players[i+2].playerIp);
        }
        newTank(ips);
    }
    
    gameStarted = true;
    
    broadcastState(GAME_STATE_PLAYING);
    gameTimer = ofGetElapsedTimef() + 30;
    for (int i = 0; i < players.size(); i++) {
        players[i].resetScore();
    }
    ofAddListener(box2d.contactStartEvents, this, &testApp::contactStart);
    ofAddListener(box2d.contactEndEvents, this, &testApp::contactEnd);

}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    
    if (key == ' ') {
        startGame();
    }
    if (key == 'r') {
        resetConnections();
        imageCounter = 0;
    }
    if (key == 'k') {
        cout << "removing dead players" << endl;
        ofRemove(players,removeDeadPlayer);
    }
    
    if (key == 'n') {
        newPlayer("zombie" + ofToString(imageCounter), ofToString(ofRandom(5)));
    }
    if (key == 'f') {
        ofToggleFullscreen();
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


void testApp::newTank(vector<string> ips) {
    ofPtr<Tank> t = ofPtr<Tank>(new Tank);
    t.get()->setPhysics(2, .1, 1);
    
    t.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), playerImages[imageCounter %playerImages.size()].width/2, playerImages[imageCounter %playerImages.size()].height/2);
    
    t.get()->setVelocity(0,0);
    t.get()->image = &playerImages[imageCounter % playerImages.size()];
    t.get()->setData(new CustomData());
    t.get()->setupCustom(tanks.size());
    int r = ofRandom(30, 255);
    int g = ofRandom(30, 255);
    int b = ofRandom(30, 255);

    for (int i = 0; i < ips.size(); i++) {
        cout << "assigning player " << i << " to tank " << tanks.size() << endl;
        t.get()->ips[i] = ips[i];
        ofxOscMessage msg;
        oscSender.setup(ips[i], CLIENT_PORT);
        msg.setRemoteEndpoint(ips[i], CLIENT_PORT);
        
        
        //what image set to use
        msg.setAddress("set");
        msg.addStringArg("images");
        msg.addIntArg(IMAGE_SET_TANKS);
        oscSender.sendMessage(msg);
        msg.clear();
        
        //set player color
        msg.setAddress("set");
        msg.addStringArg("color");
        t.get()->color = ofColor(r,g,b);
        msg.addIntArg(r);
        msg.addIntArg(g);
        msg.addIntArg(b);
        oscSender.sendMessage(msg);
        msg.clear();
        //set player image id
        msg.setAddress("set");
        msg.addStringArg("image");
        //only one tank
        msg.addIntArg(0);
        oscSender.sendMessage(msg);
        msg.clear();

        switch (i) {
            case 0:
                cout << "Player Assigned Movement" << endl;
                msg.setAddress("set");
                msg.addStringArg("game");
                msg.addStringArg("Driver / Drag to Move");
                oscSender.sendMessage(msg);
                msg.clear();

                msg.setAddress("/feedback");
                msg.addStringArg("instructions");
                msg.addStringArg("|||||||||| ARMOR  ");
                oscSender.sendMessage(msg);
                msg.clear();
                //set player control
                msg.setAddress("set");
                msg.addStringArg("control");
                msg.addIntArg(GAME_CONTROL_MOVE);
                oscSender.sendMessage(msg);
                msg.clear();

                break;
                
            case 1:
                cout << "player assigned rotation" << endl;
                msg.setAddress("set");
                msg.addStringArg("game");
                msg.addStringArg("tanks");
                oscSender.sendMessage(msg);
                msg.clear();

                
                msg.setAddress("/feedback");
                msg.addStringArg("instructions");
                msg.addStringArg("|||||||||| ARMOR  ");
                oscSender.sendMessage(msg);
                msg.clear();

                //set player control
                msg.setAddress("set");
                msg.addStringArg("control");
                msg.addIntArg(GAME_CONTROL_ROTATE);
                oscSender.sendMessage(msg);
                msg.clear();

                break;
                
            case 2:
                cout << "player assigned tapping" << endl;
                msg.setAddress("/feedback");
                msg.addStringArg("instructions");
                msg.addStringArg("|||||||||| ARMOR  ");
                oscSender.sendMessage(msg);
                msg.clear();
                
                //set player control
                msg.setAddress("set");
                msg.addStringArg("control");
                
                msg.addIntArg(GAME_CONTROL_TAP);
                oscSender.sendMessage(msg);
                msg.clear();

                break;
            default:
                break;
        }

        //set tank number
        msg.setAddress("set");
        msg.addStringArg("id");
        msg.addIntArg(tanks.size());
        oscSender.sendMessage(msg);
        msg.clear();

    }
    
    imageCounter++;


    tanks.push_back(t);
    
}

//--------------------------------------------------------------
void testApp::contactStart(ofxBox2dContactArgs &e) {
    
    if(e.a != NULL && e.b != NULL) {
        CustomData *data1 = (CustomData *)e.a->GetBody()->GetUserData();
        CustomData *data2 = (CustomData *)e.b->GetBody()->GetUserData();
        if (data1 != NULL && data2 != NULL) {
            if (data1->type == data2->type) {
                //colliding against each other
                if (data1->type == TYPE_BULLET) {
                    //BULLETS COLLIDING WITH EACH OTHER
                }
                if (data1->type == TYPE_TANK) {
                    //TANK ON TANK COLLISION
                }
            }
            else {
                collectFx.play();
                int tankId;
                //two different types colliding, lets figure it out
                //if more than 2 types, needs more logic
                if (data1->type == TYPE_BULLET) {
                    //data2 is tank
                    //remove tank armor
                    tanks[data2->id].get()->armor--;
                    tankId = data2->id;
                    data1->remove = true;
                    // newFood();
                }
                else if (data2->type == TYPE_BULLET){
                    tanks[data1->id].get()->armor--;
                    tankId = data1->id;
                    data2->remove = true;
                }
                
                for (int i = 0; i < 3; i++) {
                    //send updated armor to tank
                    string armorLeft;
                    for (int s = 0; s < tanks[tankId].get()->armor; s++) {
                        armorLeft = armorLeft + "|";
                    }
                    sendInstructions(tanks[tankId].get()->ips[i], armorLeft + " ARMOR  ");
                }
            }
        }
        else if (data2 != NULL) {
            data2->remove = true;
        }

        else if (data1 != NULL) {
            data1->remove = true;
        }
    }
    else {
    }
}

//--------------------------------------------------------------
void testApp::contactEnd(ofxBox2dContactArgs &e) {
    if (e.a != NULL && e.b != NULL) {
    }
    
}

string testApp::whoWon() {
    string winners = "winning team:\n\n";
    for (int p = 0; p < 3; p++) {
        for (int i = 0; i < players.size(); i++) {
            if(players[i].playerIp == tanks[0]->ips[p]) {
                winners += players[i].name  + "\n";
                break;
            }
        }
        
    }
    return winners;
}

