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

static bool removeFood(ofPtr<ofxBox2dBaseShape> shape) {
    CustomData *custom = (CustomData *)shape->getData();
    return custom->remove;
}

//--------------------------------------------------------------
void testApp::setup(){
    ofSetFrameRate(30);
    ofEnableAlphaBlending();
    ofEnableSmoothing();
    ofBackground(10);
    collectFx.loadSound("collect.wav");
    collectFx.setVolume(.4);
    collectIndex = 0;
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

    //game setup
    glowImage.loadImage("glow.png");
    debugging = true;
    gameStarted = false;
    gameState = GAME_STATE_WAITING;
    gameTimer = 999999999;
    timeUntilNextGame = 5000;
    timerText.loadFont("joystix.ttf", 48);
    smallerText.loadFont("joystix.ttf", 32);

    selectedGame = GAME_NONE;
    explosionIndex = 0;
    shotIndex = 0;
    for (int i = 0; i < 10; i++) {
        shotFx[i].loadSound("shoot.wav");
        explosionFx[i].loadSound("explode.wav");
        shotFx[i].setLoop(false);
        explosionFx[i].setLoop(false);
    }
    waitingScreen = CONNECT_INSTRUCTIONS;
    foodMultiplier = 1.5;
    instructionLine1.push_back("DOWNLOAD THE NERDLAB CONTROLLER\nON THE APP STORE OR GOOGLE PLAY" );
    instructionLine1.push_back("IP ADDRESS:\n" + bonjour->getDeviceIp());
    instructionLine1.push_back("JOIN THE WIFI NETWORK\nSSID: CANESFILM\nPASS: CANESFILM");
    instructionLine1.push_back("DRAG YOUR FINGER TO MOVE");
    instructionLine1.push_back("GET POINTS BY EATING\nTHE GRAY PLANTKON");
    instructionLine1.push_back("THE AVATAR ON YOUR\nPHONE MATCHES THE\nONE ON THIS SCREEN");

    instructionIndex = 0;
    instructionTimer = ofGetElapsedTimef();
    backgroundImages[0].loadImage("water/wave0.png");
    backgroundImages[1].loadImage("water/wave1.png");
    backgroundImages[2].loadImage("water/wave2.png");

}

void testApp::loadImageSet(string set) {
    ofDirectory dir(set);
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

}

//--------------------------------------------------------------
void testApp::update(){
    if (ofGetElapsedTimef() > instructionTimer) {
        instructionIndex++;
        cout << "instructions advanced" << endl;
        if (instructionLine1.size() <= instructionIndex) {
            instructionIndex = 0;
        }
        instructionTimer = ofGetElapsedTimef() + 8;
        cout << instructionIndex << endl;
    }
    switch (selectedGame) {
        case GAME_SEA_NIBBLES:
            ofRemove(food, removeFood);
            
            if (food.size() < players.size() * foodMultiplier) {
                for (int i = food.size(); i < players.size() * foodMultiplier; i++) {
                    newFood();
                }
                
            }
            
            if (ofGetElapsedTimef() > gameTimer && gameStarted) {
                ofRemoveListener(box2d.contactStartEvents, this, &testApp::contactNibbles);
                gameStarted = false;
                winnerID = whoWon();
                broadcastScores();
                timeUntilNextGame = ofGetElapsedTimef() + 30;
                
            }
            break;
        case GAME_TANKS:
            for (int i = 0; i < tanks.size(); i++) {
                ofRemove(tanks[i]->shots, removeShape);
            }
            ofRemove(tanks, removeTank);
            
            if (tanks.size() <= 1 && gameStarted) {
                //last one standing
                winningTeam = whichTeamWon();
                broadcastScores();
                broadcastState(GAME_STATE_WAITING);
                timeUntilNextGame = ofGetElapsedTimef() + 30;
                gameStarted = false;
                ofRemoveListener(box2d.contactStartEvents, this, &testApp::contactTank);
            }

            break;
    }
    
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
            cout << playerName << " joined the game" << endl;
            //look for existing players with the same IP
            if (std::find(playerIPs.begin(), playerIPs.end(), incomingPlayerIP) == playerIPs.end()) {

                //no player with that IP address, create a new one
                int playerID = newPlayer(playerName, incomingPlayerIP);
                //join the game
                joinGame(incomingPlayerIP);
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
                int objNum = msg.getArgAsInt32(0);

                float xSpeed = msg.getArgAsFloat(1) * -.1;
                float ySpeed = msg.getArgAsFloat(2) * -.1;

                if (selectedGame == GAME_SEA_NIBBLES && humanoids.size() > objNum) {
                    humanoids[objNum].get()->setVelocity(xSpeed, ySpeed);
                    
                }
                if (selectedGame == GAME_TANKS && tanks.size() > objNum) {
                    float xSpeed = msg.getArgAsFloat(1) * -.01;
                    float ySpeed = msg.getArgAsFloat(2) * -.01;
                    tanks[objNum].get()->setVelocity(xSpeed, ySpeed);
                    
                }
            }
        }
        if (msg.getAddress() == "/tap") {
            int tankNumber = msg.getArgAsInt32(0);
            if (tanks.size() > tankNumber) {
                tanks[tankNumber].get()->shoot();
                cout << "Shooting?" << endl;
                shotFx[shotIndex].play();
                shotIndex++;
                if (shotIndex >= 10) {
                    shotIndex = 0;
                }
            }
        }
        
    }
    //restart game automatically
    if (!gameStarted && ofGetElapsedTimef() > timeUntilNextGame) {
        cout << "starting again" << endl;
        startGame(selectedGame);
        
    }

}


//--------------------------------------------------------------
void testApp::draw(){
    //calculate time left

    string timeLeft = ofToString(abs(int(ofGetElapsedTimef() - gameTimer)));
    string timeTil = ofToString(abs(int(ofGetElapsedTimef() - timeUntilNextGame)));
    float bgSpacer = ofMap(sin(ofGetElapsedTimef()), -1, 1, -50, 0);
    float bgSpacer2 = ofMap(sin(ofGetElapsedTimef()), -1, 1, -30, 0);
    float bgSpacer3 = ofMap(sin(ofGetElapsedTimef()), -1, 1, -60, 0);
    
    int column = 0;
    switch (selectedGame) {
        case GAME_SEA_NIBBLES:
            ofSetColor(150, 242, 241);
            ofRect(0, 0, ofGetWidth(), ofGetHeight());
            ofSetColor(255);
            for (int i = 0; i < 2; i++) {
                backgroundImages[2].draw(bgSpacer3, ofGetHeight()-backgroundImages[0].height-backgroundImages[1].height+(backgroundImages[2].height/3));
                backgroundImages[2].draw(bgSpacer3 + backgroundImages[2].width, ofGetHeight()-backgroundImages[0].height-backgroundImages[1].height+(backgroundImages[2].height/3));
            }
            
            for (int i = 0; i < 2; i++) {
                backgroundImages[1].draw(bgSpacer2, ofGetHeight()-backgroundImages[0].height-(backgroundImages[1].height/2));
                backgroundImages[1].draw(bgSpacer2 + backgroundImages[1].width, ofGetHeight()-backgroundImages[0].height-(backgroundImages[1].height/2));
            }
            
            for (int i = 0; i < 2; i++) {
                backgroundImages[0].draw(bgSpacer, ofGetHeight()-backgroundImages[0].height);
                backgroundImages[0].draw(bgSpacer + backgroundImages[0].width, ofGetHeight()-backgroundImages[0].height);
            }
            
            
            
            
            for (int i = 0; i < humanoids.size(); i++) {
                humanoids[i].get()->display();
            }
            
            for (int i = 0; i < food.size(); i++) {
                food[i].get()->display();
            }
            break;
            
        case GAME_TANKS:
            for (int i = 0; i < tanks.size(); i++) {
                tanks[i].get()->display();
                for (int j = 0; j < tanks[i]->shots.size(); j++) {
                    tanks[i]->shots[j]->display();
                }
            }

            break;
        default:
            break;
    }
    
    if (gameStarted) {
        if (selectedGame == GAME_SEA_NIBBLES) {
            ofSetColor(0);
            timerText.drawString(timeLeft, ofGetWidth() - timerText.stringWidth(timeLeft) - 18, timerText.getLineHeight() + 18);

            ofSetColor(255);
            timerText.drawString(timeLeft, ofGetWidth() - timerText.stringWidth(timeLeft) - 20, timerText.getLineHeight() + 20);
            
        }
        if (ofGetElapsedTimef() < gameTimer - 28) {
            ofSetColor(0);
            timerText.drawStringCentered(startingInstructions, ofGetWidth()/2-2, ofGetHeight()/2-2);
            ofSetColor(255);
            timerText.drawStringCentered(startingInstructions, ofGetWidth()/2, ofGetHeight()/2);
        }

    }
    else {
        switch (selectedGame) {
            case GAME_SEA_NIBBLES:
                ofSetColor(255);
//                timerText.drawStringCentered("Sea Nibbles", ofGetWidth()/2-2, 50-2);
                smallerText.drawStringCentered(ofToString(players.size()) + " players connected", ofGetWidth()/2 - 2, ofGetHeight()/3  - 2);

                if (abs(int(ofGetElapsedTimef() - timeUntilNextGame)) <= 30) {
                    timerText.drawStringCentered("Next Game: " + timeTil + " seconds", ofGetWidth()/2 - 2, ofGetHeight()/2 - 2);
                }

                //sync instruction vectors
                ofSetColor(0);
                smallerText.drawStringCentered(instructionLine1[instructionIndex], ofGetWidth()/2 -2, ofGetHeight()/1.5 - 2);
//                timerText.drawStringCentered(bonjour->getDeviceIp(), ofGetWidth()/2 -2, ofGetHeight()/1.5 -2);
//                timerText.drawStringCentered("Sea Nibbles", ofGetWidth()/2, 50);
            
                smallerText.drawStringCentered(ofToString(players.size()) + " players connected", ofGetWidth()/2 - 2, ofGetHeight()/3);
                if (abs(int(ofGetElapsedTimef() - timeUntilNextGame)) <= 30) {
                    timerText.drawStringCentered("Next Game: " + timeTil + " seconds", ofGetWidth()/2, ofGetHeight()/2);
                }
                
                ofSetColor(255);
                //sync instruction vectors
                smallerText.drawStringCentered(instructionLine1[instructionIndex], ofGetWidth()/2, ofGetHeight()/1.5);

//                timerText.drawStringCentered(bonjour->getDeviceIp(), ofGetWidth()/2, ofGetHeight()/1.5);

                if (winnerID >= 0 && players.size() > 0 && winnerID < players.size()) {
                    ofSetColor(0);
                    timerText.drawStringCentered(players[winnerID].name + " wins!", ofGetWidth()/2 - 2, ofGetHeight()/4.5 - 2);
                    
                    ofSetColor(255);
                    timerText.drawStringCentered(players[winnerID].name + " wins!", ofGetWidth()/2, ofGetHeight()/4.5);
                    
                }

    
                break;
            case GAME_TANKS:
                ofSetColor(0);
                timerText.drawStringCentered("Tanks", ofGetWidth()/2-2, 50-2);
                timerText.drawStringCentered("Next Game: " + timeTil + " seconds", ofGetWidth()/2 - 2, ofGetHeight()/1.5 - 2);
                timerText.drawStringCentered(bonjour->getDeviceIp(), ofGetWidth()/2 -2, ofGetHeight()/1.1 -2);

                ofSetColor(255);
                timerText.drawStringCentered("Tanks", ofGetWidth()/2, 50);
                timerText.drawStringCentered("Next Game: " + timeTil + " seconds", ofGetWidth()/2, ofGetHeight()/1.5);
                timerText.drawStringCentered(bonjour->getDeviceIp(), ofGetWidth()/2, ofGetHeight()/1.1);

                if (!winningTeam.empty()) {
                    ofSetColor(0);
                    timerText.drawStringCentered(winningTeam, ofGetWidth()/2 - 2, ofGetHeight()/3 - 2);
                    
                    ofSetColor(255);
                    timerText.drawStringCentered(winningTeam, ofGetWidth()/2, ofGetHeight()/3);
                    
                }

                break;
                
            case GAME_NONE:
                if (waitingScreen == DOWNLOAD_INSTRUCTIONS) {
                    ofSetColor(255);
                    
                    for (int i = 0; i < players.size(); i++) {
                        if (i < 22) {
                            column = 0;
                        }
                        if (i == 22) {
                            column = 1;
                        }
                        if (i == 44) {
                            column = 2;
                        }
                        if (i == 66) {
                            column = 3;
                        }
                        if (i == 88) {
                            column = 4;
                        }
                        smallerText.drawString(players[i].name, 20 + (column * 270), i%22*40 + 40);
                    }
                    
                }
                else if (waitingScreen == CONNECT_INSTRUCTIONS) {
                    ofSetColor(0);
                    timerText.drawStringCentered("NERDLAB", ofGetWidth()/2-2, 20-2);
                    
                    timerText.drawStringCentered("Connect to This IP Address", ofGetWidth()/2 - 2, ofGetHeight()/2 - 2);
                    timerText.drawStringCentered(bonjour->getDeviceIp(), ofGetWidth()/2 -2, ofGetHeight()/1.5 -2);
                    
                    smallerText.drawStringCentered(ofToString(players.size()) + " players connected", ofGetWidth()/2 - 2, ofGetHeight() - 150 - 2);
                    
                    ofSetColor(255);
                    timerText.drawStringCentered("NERDLAB", ofGetWidth()/2-2, 160);
                    
                    timerText.drawStringCentered("Connect to This IP Address", ofGetWidth()/2, ofGetHeight()/2);
                    timerText.drawStringCentered(bonjour->getDeviceIp(), ofGetWidth()/2, ofGetHeight()/1.5);
                    
                    smallerText.drawStringCentered(ofToString(players.size()) + " players connected", ofGetWidth()/2 - 2, ofGetHeight() - 150);
                    
                }

            default:
                break;
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

void testApp::broadcastGame(string gameName) {
    for (int i = 0; i < players.size(); i++) {
        sendGame(players[i].playerIp, gameName);
    }
}

void testApp::sendGame(string ip, string gameName) {
    //what game we're playing
    ofxOscMessage msg;
    oscSender.setup(ip, CLIENT_PORT);
    msg.setRemoteEndpoint(ip, CLIENT_PORT);
    msg.setAddress("set");
    msg.addStringArg("game");
    if (selectedGame == GAME_TANKS) {
        msg.addStringArg("tanks");
    }
    if (selectedGame == GAME_SEA_NIBBLES) {
        msg.addStringArg("sea nibbles");
        
    }
    
    oscSender.sendMessage(msg);
    msg.clear();
    
    //what image set to use
    msg.setAddress("set");
    msg.addStringArg("images");
    
    if (selectedGame == GAME_TANKS) {
        msg.addIntArg(IMAGE_SET_TANKS);
    }
    
    if (selectedGame == GAME_SEA_NIBBLES) {
        msg.addIntArg(IMAGE_SET_ABSTRACT);
    }
    oscSender.sendMessage(msg);
    msg.clear();
    
    
    

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
void testApp::startGame(int game) {
    for (int i = 0; i < humanoids.size(); i++) {
        humanoids[i]->destroy();
    }

    
    humanoids.clear();
    
    for (int i = 0; i < tanks.size(); i++) {
        tanks[i]->destroy();
    }
    tanks.clear();
    
    for (int i = 0; i < food.size(); i++) {
        food[i]->destroy();
    }
    
    food.clear();
    
    box2d.createBounds();

    //create all players
    imageCounter = 0;

    ofRemoveListener(box2d.contactStartEvents, this, &testApp::contactNibbles);
    ofRemoveListener(box2d.contactStartEvents, this, &testApp::contactTank);
    switch (game) {
        case GAME_SEA_NIBBLES:
            backgroundTrack.loadSound("seanibbles.wav");
            backgroundTrack.setLoop(true);

            broadcastGame("Sea Nibbles");

            startingInstructions = "FEED!";

            loadImageSet("abstract");
            plankton.loadImage("plankton.png");


            for (int i = 0; i < players.size(); i++) {
                newHumanoid(players[i].playerIp);
            }
            gameTimer = ofGetElapsedTimef() + 30;
            ofAddListener(box2d.contactStartEvents, this, &testApp::contactNibbles);

            
            break;
        case GAME_TANKS:
            broadcastGame("Tanks");
            backgroundTrack.loadSound("tanks.wav");
            backgroundTrack.setLoop(true);

            loadImageSet("tanks");
            startingInstructions = "DESTROY!";
            for (int i = 0; i < players.size(); i+=3) {
                cout << "I'm in a loop!" << endl;
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
            ofAddListener(box2d.contactStartEvents, this, &testApp::contactTank);

            break;
    }

    backgroundTrack.play();

    
    gameStarted = true;
    
    broadcastState(GAME_STATE_PLAYING);
    gameTimer = ofGetElapsedTimef() + 30;
    for (int i = 0; i < players.size(); i++) {
        players[i].resetScore();
    }

}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if (!gameStarted) {
        if (key == '1') {
            selectedGame = GAME_SEA_NIBBLES;
        }
        if (key == '2') {
            selectedGame = GAME_TANKS;
        }
        
        if (key == 'd') {
            waitingScreen = DOWNLOAD_INSTRUCTIONS;
        }
        
        if (key == 'c') {
            waitingScreen = CONNECT_INSTRUCTIONS;
        }
    }
    
    if (key == ' ') {
        startGame(selectedGame);
    }
    
    if (selectedGame == GAME_SEA_NIBBLES) {
        if (key == 'm') {
            foodMultiplier += .1;
        }
        if (key == 'l') {
            foodMultiplier -= .1;
        }
    }
    if (key == 'r') {
        resetConnections();
        backgroundTrack.stop();
        selectedGame = GAME_NONE;
        gameStarted = false;
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

void testApp::newFood() {
    ofPtr<Food> f = ofPtr<Food>(new Food);
    f.get()->setPhysics(.5, 1, 1);
    
    float fSize = ofRandom(.1, .5);
    f.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), 50 * fSize, 50 * fSize);
    
    f.get()->setVelocity(ofRandom(-2,2), ofRandom(-2,2));
    f.get()->image = &plankton;
    f.get()->shadowImage = &plankton;
    
    f.get()->setData(new CustomData());
    f.get()->setupCustom(food.size());
    food.push_back(f);
    
}


void testApp::newHumanoid(string ip) {
    ofxOscMessage msg;
    oscSender.setup(ip, CLIENT_PORT);
    msg.setRemoteEndpoint(ip, CLIENT_PORT);
    
    ofPtr<Humanoid> h = ofPtr<Humanoid>(new Humanoid);
    h.get()->setPhysics(.5, 1, 1);
    
    h.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), playerImages[imageCounter %playerImages.size()].width/2, playerImages[imageCounter %playerImages.size()].height/2);
    
    h.get()->setVelocity(0,0);
    
    h.get()->setData(new CustomData());
    h.get()->setupCustom(humanoids.size());
    
    //set player color
    msg.setAddress("set");
    msg.addStringArg("color");
    int r = ofRandom(0, 255);
    int g = ofRandom(0, 255);
    int b = ofRandom(0, 255);
    h.get()->color = ofColor(r,g,b);
    
    msg.addIntArg(r);
    msg.addIntArg(g);
    msg.addIntArg(b);
    oscSender.sendMessage(msg);
    msg.clear();
    //set player image id
    
    msg.setAddress("set");
    msg.addStringArg("image");
    msg.addIntArg(imageCounter %playerImages.size());
    h.get()->image = &playerImages[imageCounter %playerImages.size()];
    h.get()->glow = &glowImage;
    oscSender.sendMessage(msg);
    msg.clear();
    imageCounter++;
    
    //set player control
    msg.setAddress("set");
    msg.addStringArg("control");
    msg.addIntArg(GAME_CONTROL_MOVE);
    oscSender.sendMessage(msg);
    msg.clear();
    //set player number
    msg.setAddress("set");
    msg.addStringArg("id");
    cout << "Controlling Human " << humanoids.size() << endl;
    msg.addIntArg(humanoids.size());
    oscSender.sendMessage(msg);
    msg.clear();
    
    humanoids.push_back(h);
    cout << "Humanoid Created" << endl;
    
}

//-----------GAME #1 FUNCTIONS


void testApp::newTank(vector<string> ips) {
    ofPtr<Tank> t = ofPtr<Tank>(new Tank);
    t.get()->setPhysics(2, .1, 1);
    
    t.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), playerImages[imageCounter %playerImages.size()].width/2, playerImages[imageCounter %playerImages.size()].height/2);
    
    t.get()->setVelocity(0,0);
    t.get()->image = &playerImages[imageCounter % playerImages.size()];
    t.get()->glow = &glowImage;
    t.get()->setData(new CustomData());
    t.get()->setupCustom(tanks.size());
    int r = ofRandom(0, 255);
    int g = ofRandom(0, 255);
    int b = ofRandom(0, 255);

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
void testApp::contactTank(ofxBox2dContactArgs &e) {
    
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
                explosionFx[explosionIndex].play();
                explosionIndex++;
                if (explosionIndex >= 10) {
                    explosionIndex = 0;
                }
                int tankId;
                bool foundTank = false;
                //two different types colliding, lets figure it out
                //if more than 2 types, needs more logic
                if (data1->type == TYPE_BULLET) {
                    //data2 is tank
                    //remove tank armor
                    foundTank = true;
                    tanks[data2->id].get()->armor--;
                    tankId = data2->id;
                    data1->remove = true;
                }
                else if (data2->type == TYPE_BULLET){
                    foundTank = true;
                    tanks[data1->id].get()->armor--;
                    tankId = data1->id;
                    data2->remove = true;
                }
                if (foundTank) {
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

void testApp::contactNibbles(ofxBox2dContactArgs &e) {
    
    if(e.a != NULL && e.b != NULL) {
        CustomData *data1 = (CustomData *)e.a->GetBody()->GetUserData();
        CustomData *data2 = (CustomData *)e.b->GetBody()->GetUserData();
        if (data1 != NULL && data2 != NULL) {
            if (data1->type == data2->type) {
                //colliding against each other
                if (data1->type == TYPE_HUMANOID) {
                    //PLAYERS COLLIDING WITH EACH OTHER
                }
                if (data1->type == TYPE_FOOD) {
                    //FOOD ON FOOD COLLISION
                }
            }
            else {
                collectFx.play();
                collectIndex++;
                if (collectIndex > 5) {
                    collectIndex = 0;
                }
                //two different types colliding, lets figure it out
                //if more than 2 types, needs more logic
                if (data1->type == TYPE_FOOD) {
                    //food is data1, player is data2
                    players[data2->id].score+= food[data1->id].get()->getWidth();
                    sendScore(players[data2->id].playerIp, players[data2->id].score);
                    data1->remove = true;
                }
                else {
                    //food is data2, player is data1
                    players[data1->id].score+= food[data2->id].get()->getWidth();
                    sendScore(players[data1->id].playerIp, players[data1->id].score);
                    
                    data2->remove = true;
                }
                
            }
        }
    }
}

string testApp::whichTeamWon() {
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

int testApp::whoWon() {
    int highScore = -999;
    int theWinner = -1;
    for (int i = 0; i < players.size(); i++) {
        if (players[i].score > highScore) {
            theWinner = i;
            highScore = players[i].score;
        }
    }
    return theWinner;
}
