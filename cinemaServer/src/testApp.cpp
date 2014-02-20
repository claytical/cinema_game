#include "testApp.h"

#define NUMBER_OF_PLAYERS 100
#define NUMBER_OF_ROWS    10
#define PLAYER_SIZE       35
#define STARTING_PORT     9000
#define AMOUNT_OF_FOOD    50

#define WAITING_FOR_PLAYERS 0
#define PLAYING_GAME_1      1
#define SHOWING_WINNER      2

static bool removeShape(ofPtr<ofxBox2dBaseShape> shape) {
    CustomData *custom = (CustomData *)shape->getData();
    return custom->remove;
}

//--------------------------------------------------------------
void testApp::setup() {
	ofSetVerticalSync(true);
    ofSetFrameRate(30);
	ofSetLogLevel(OF_LOG_VERBOSE);
    ofEnableAlphaBlending();
    ofEnableSmoothing();
    ofBackground(13, 45, 152);
    
    bonjour = new ofxBonjourIp();
    bonjour->addEventListeners(this); // optional
    
    bonjour->startService();
    
    bonjour->discoverService();
    largeGameText.loadFont("joystix.ttf", 100);
    gameText.loadFont("joystix.ttf", 50);
	
    //Server Setup
    serverRecvPort = STARTING_PORT;
    serverReceiver.setup(serverRecvPort);

	box2d.init();
	box2d.setGravity(0, 0);
    box2d.enableEvents();
	box2d.createBounds();
	box2d.setFPS(30.0);
	for (int i = 0; i < 10; i++) {
        playerImages[i].loadImage(ofToString(i) + ".png");
    }

    //CREATE food
    
    for (int i = 0; i < AMOUNT_OF_FOOD; i++) {
        newFood();
    }
    gameState = WAITING_FOR_PLAYERS;
    logo.loadImage("nerdlab.png");
    
    mainMenu = new ofxUICanvas();
    mainMenu->addImageButton("Sea Nibbles", "sea-nibbles.png", false, 200, 16);
    gameoverMenu = new ofxUICanvas();
    
    gameoverMenu->addButton("main menu", false);
    gameoverMenu->addButton("replay", false);
    ofAddListener(mainMenu->newGUIEvent,this,&testApp::guiEvent);
    mainMenu->setPosition(logo.width, logo.height + 50);
    gameoverMenu->setPosition(ofGetWidth() - gameoverMenu->getRect()->getWidth(), ofGetHeight() - gameoverMenu->getRect()->getHeight());
    gameoverMenu->setVisible(false);
    newPlayer("tical");
}

void testApp::guiEvent(ofxUIEventArgs &e)
{
	string name = e.widget->getName();
	int kind = e.widget->getKind();
    if (kind == OFX_UI_WIDGET_IMAGEBUTTON) {
        ofxUIImageButton *button = (ofxUIImageButton *) e.widget;
        if (name == "Sea Nibbles" && button->getValue()) {
            broadcastFeedback("state", GAME_STATE_PLAYING);
            gameState = PLAYING_GAME_1;
            ofAddListener(box2d.contactStartEvents, this, &testApp::contactStart);
            ofAddListener(box2d.contactEndEvents, this, &testApp::contactEnd);
            gameTimer = ofGetElapsedTimef() + 30;
            mainMenu->setVisible(false);
            ofRemoveListener(mainMenu->newGUIEvent, this, &testApp::guiEvent);
        }
        
    }
    if(kind == OFX_UI_WIDGET_BUTTON)
    {
        ofxUIButton *button = (ofxUIButton *) e.widget;
        
        if (name == "replay" && button->getValue()) {
            ofRemoveListener(gameoverMenu->newGUIEvent, this, &testApp::guiEvent);
            gameoverMenu->setVisible(false);
            gameTimer = ofGetElapsedTimef() + 30;
            gameState = PLAYING_GAME_1;
        }
        
        if (name == "main menu" && button->getValue()) {
            ofRemoveListener(gameoverMenu->newGUIEvent, this, &testApp::guiEvent);
            ofAddListener(mainMenu->newGUIEvent, this, &testApp::guiEvent);
            gameoverMenu->setVisible(false);
            mainMenu->setVisible(true);
            gameState = GAME_STATE_WAITING;
            
        }
    }
}

void testApp::exit() {
    delete mainMenu;
    delete gameoverMenu;
}
void testApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void testApp::update() {
    //always let players join and move around for now
    box2d.update();
    //Check for incoming player messages
    while (serverReceiver.hasWaitingMessages()) {
        ofxOscMessage msg;
        serverReceiver.getNextMessage(&msg);
        ofLogVerbose("Server Received Message: " + getOscMsgAsString(msg) + " from " + msg.getRemoteIp());
        //JOIN GAME
        if (msg.getAddress() == "/join") {
            //PLAYER JOINING
            string incomingPlayer = msg.getRemoteIp();
            //take in their name
            string playerName = msg.getArgAsString(0);
            //is this a new player?
            if (std::find(knownClients.begin(), knownClients.end(), incomingPlayer) == knownClients.end()) {
                knownClients.push_back(incomingPlayer);
                //send feedback to player
                serverSender.setup(incomingPlayer, serverRecvPort + 1);
                //create player
                ofPtr<Player> p = ofPtr<Player>(new Player);
                //remember their name
                p.get()->name = playerName;
                //delegate their number based on the size of known clients minus 1 (counting from zero)
                p.get()->playerId = knownClients.size() - 1;
                p.get()->teamId = p.get()->playerId % PLAYERS_PER_TEAM;
                p.get()->subTeamId = p.get()->playerId % NUMBER_OF_PLAYERS;

                p.get()->setPhysics(1.0, .5, 0.3);
                p.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), PLAYER_SIZE, PLAYER_SIZE);

                switch (p.get()->teamId) {
                    case 0:
                        p.get()->color = ofColor::springGreen;
                        break;
                    case 1:
                        p.get()->color = ofColor::orchid;
                        break;
                    case 2:
                        p.get()->color = ofColor::blueViolet;
                        break;
                    case 3:
                        p.get()->color = ofColor::turquoise;
                        break;
                    case 4:
                        p.get()->color = ofColor::whiteSmoke;
                        break;
                    case 5:
                        p.get()->color = ofColor::tomato;
                        break;
                    case 6:
                        p.get()->color = ofColor::violet;
                        break;
                    case 7:
                        p.get()->color = ofColor::mintCream;
                        break;
                    case 8:
                        p.get()->color = ofColor::darkorange;
                        break;
                    case 9:
                        p.get()->color = ofColor::darkMagenta;
                        break;
                    default:
                        p.get()->color = ofColor::darkGrey;
                        break;
                }
                p.get()->image = &playerImages[p.get()->subTeamId];
                p.get()->setupCustom(knownClients.size() -1);
               
                ofxOscMessage msgToPlayer;
                //client will look for joined message, the argument is their player number
                msgToPlayer.setAddress("/joined");
                msgToPlayer.addIntArg(p.get()->playerId);
                msgToPlayer.addIntArg(p.get()->teamId);
                msgToPlayer.addIntArg(p.get()->subTeamId);
                msgToPlayer.addIntArg(GAME_CONTROL_MOVE);
                msgToPlayer.addIntArg(GAME_STATE_WAITING);
                msgToPlayer.setRemoteEndpoint(incomingPlayer, serverRecvPort + 1);
                serverSender.sendMessage(msgToPlayer);
                
                players.push_back(p);
                
                ofLogVerbose("Sent message " + msgToPlayer.getArgAsString(0) + " to " + msgToPlayer.getRemoteIp()
                             + ":" + ofToString(msgToPlayer.getRemotePort()));
                
            }
            
        }
        //PLAYER MOVEMENT
        if (msg.getAddress() == "/move") {
            
            if (msg.getNumArgs() == 3) {
                int playerNumber = msg.getArgAsInt32(0);
                if (players.size() > playerNumber) {
                    float xSpeed = msg.getArgAsFloat(1) * -.1;
                    float ySpeed = msg.getArgAsFloat(2) * -.1;
                    players[playerNumber].get()->setVelocity(xSpeed, ySpeed);
                }
            }
        }
        
    }

    
    
    switch (gameState) {
        case WAITING_FOR_PLAYERS:
            break;
        case PLAYING_GAME_1:
            ofRemove(food, removeShape);
            if (food.size() < AMOUNT_OF_FOOD) {
                for (int i = food.size(); i < AMOUNT_OF_FOOD; i++) {
                    newFood();
                }
            }

            if (ofGetElapsedTimef() > gameTimer) {
                winner = whoWon();
                gameState = SHOWING_WINNER;
                broadcastFeedback("state", GAME_STATE_WAITING);
                ofAddListener(gameoverMenu->newGUIEvent,this,&testApp::guiEvent);
                gameoverMenu->setVisible(true);
                ofRemoveListener(box2d.contactStartEvents, this, &testApp::contactStart);
                ofRemoveListener(box2d.contactEndEvents, this, &testApp::contactEnd);

            }

            
            break;
        case SHOWING_WINNER:
            //
            break;
            
        default:
            break;
    }
}


//--------------------------------------------------------------
void testApp::draw() {
    //DEBUG INFO
	string info = "";
    for (int i = 0; i < players.size(); i++) {
        players[i].get()->display();
    }
    
    float fontWidth = largeGameText.stringWidth(ofToString(int(gameTimer - ofGetElapsedTimef()))) + 20;

    switch (gameState) {
        case WAITING_FOR_PLAYERS:
            info += "Players: " + ofToString(knownClients.size()) + "\n";
//            info += "FPS: "+ofToString(ofGetFrameRate(), 1)+"\n";
            info += "Host: " + bonjour->getDeviceHostName() + "\n";
            info += "IP: " + bonjour->getDeviceIp()+ "\n";

            for (int i = 0; i < players.size(); i++) {
                info+= players[i].get()->name + " has joined the game\n";
            }
            ofSetColor(255,255,255,200);
            logo.draw(ofGetWidth()/2-logo.width/2, 20);
            
            break;
        case PLAYING_GAME_1:

            for (int i = 0; i < food.size(); i++) {
                food[i].get()->display();
            }
            // draw the ground
            box2d.drawGround();
            
            ofSetColor(0);
            largeGameText.drawString(ofToString( int(gameTimer - ofGetElapsedTimef())), ofGetWidth() - fontWidth, 115);
            ofSetColor(255, 255, 255);
            largeGameText.drawString(ofToString( int(gameTimer - ofGetElapsedTimef())), ofGetWidth() - fontWidth, 120);
            

            break;
        case SHOWING_WINNER:
            ofSetColor(0);
            largeGameText.drawString("Game Over", 20, ofGetHeight()/3 );
            ofSetColor(255,255,255);
            largeGameText.drawString("Game Over", 25, ofGetHeight()/3 + 5);
            
            if (players.size() > 0) {
                ofSetColor(0);
                gameText.drawString(players[winner].get()->name + " wins!", 20, ofGetHeight()/3 + 100);
                ofSetColor(255, 255, 255);
                gameText.drawString(players[winner].get()->name + " wins!", 25, ofGetHeight()/3 + 105);
            }
            break;
        default:
            break;
    }
	ofSetHexColor(0xffffff);
	ofDrawBitmapString(info, 20, ofGetHeight()-100);
    
}

void testApp::newPlayer(string pname) {
    ofPtr<Player> p = ofPtr<Player>(new Player);
    //remember their name
    p.get()->name = pname;
    //delegate their number based on the size of known clients minus 1 (counting from zero)
    p.get()->playerId = players.size();
    p.get()->setPhysics(1.0, .5, 0.3);
    p.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), PLAYER_SIZE, PLAYER_SIZE);
    p.get()->color = ofColor(ofRandom(255), ofRandom(255), ofRandom(255));
    p.get()->image = &playerImages[players.size()%10];
    p.get()->setupCustom(players.size());
    
    players.push_back(p);

}
//--------------------------------------------------------------
void testApp::keyPressed(int key) {
	if (key == OF_KEY_DOWN) {
        players[0].get()->addForce(ofVec2f(0,4), 2);
    }
    if (key == OF_KEY_UP) {
        players[0].get()->addForce(ofVec2f(0,-4), 2);
        
    }
	if (key == OF_KEY_LEFT) {
        players[0].get()->addForce(ofVec2f(-4,0), 2);
    }
    if (key == OF_KEY_RIGHT) {
        players[0].get()->addForce(ofVec2f(4,0), 2);
        
    }
    
	if(key == 't') ofToggleFullscreen();

    if(key == ' ') {
/*
        if (gameState == WAITING_FOR_PLAYERS) {
            broadcastFeedback("state", GAME_STATE_PLAYING);
            gameState = PLAYING_GAME_1;
            ofAddListener(box2d.contactStartEvents, this, &testApp::contactStart);
            ofAddListener(box2d.contactEndEvents, this, &testApp::contactEnd);
            gameTimer = ofGetElapsedTimef() + 30;

        }
 */
        if (gameState == PLAYING_GAME_1) {
            gameState = SHOWING_WINNER;
            broadcastFeedback("state", GAME_STATE_WAITING);

        }
        else if (gameState == SHOWING_WINNER) {
            broadcastFeedback("state", GAME_STATE_WAITING);
            gameState = WAITING_FOR_PLAYERS;
            for (int i = 0; i < players.size(); i++) {
                players[i].get()->score = 0;
            }
        }
    }
}

int testApp::whoWon() {
    int highScore = -999;
    int winner = -1;
    for (int i = 0; i < players.size(); i++) {
        if (players[i].get()->score>highScore) {
            winner = i;
            highScore = players[i].get()->score;
        }
    }
    return winner;
}

//--------------------------------------------------------------
void testApp::keyReleased(int key) {
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ) {
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button) {
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button) {
	
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button) {
}

//--------------------------------------------------------------
void testApp::resized(int w, int h){
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

//--------------------------------------------------------------
void testApp::contactStart(ofxBox2dContactArgs &e) {
    cout  << "made contact" << endl;
    
    if(e.a != NULL && e.b != NULL) {
        CustomData *data1 = (CustomData *)e.a->GetBody()->GetUserData();
        CustomData *data2 = (CustomData *)e.b->GetBody()->GetUserData();
        if (data1 != NULL && data2 != NULL) {
        if (data1->type == data2->type) {
            //colliding against each other
            if (data1->type == PLAYER_TYPE) {
                //PLAYERS COLLIDING WITH EACH OTHER
            }
            if (data1->type == FOOD_TYPE) {
                //FOOD ON FOOD COLLISION
            }
        }
        else {
            //two different types colliding, lets figure it out
            //if more than 2 types, needs more logic
            if (data1->type == FOOD_TYPE) {
                //food is data1, player is data2
                data1->remove = true;
                players[data2->id].get()->score+= food[data1->id].get()->getRadius();
               // newFood();
            }
            else {
                //food is data2, player is data1
                data2->remove = true;
                players[data1->id].get()->score+= food[data2->id].get()->getRadius();
               // newFood();
            }

        }
        }
    }
}
void testApp::newFood() {
    ofPtr<Food> f = ofPtr<Food>(new Food);
    f.get()->setPhysics(.5, 1, 1);

    f.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), ofRandom(10,20));
    f.get()->setVelocity(ofRandom(-2,2), ofRandom(-2,2));

    f.get()->setData(new CustomData());
    f.get()->setupCustom(food.size());
    food.push_back(f);

}
//--------------------------------------------------------------
void testApp::contactEnd(ofxBox2dContactArgs &e) {
    cout << "contact ended" << endl;
    if (e.a != NULL && e.b != NULL) {
//        Player *player = (Player*)e.a->GetBody()->GetUserData();
    }

}


/* TWO LAST FUNCTIONS ARE FROM CHAT SYSTEM EXAMPLE in oF Addons*/
//--------------------------------------------------------------
string testApp::getOscMsgAsString(ofxOscMessage m){
	string msg_string;
	msg_string = m.getAddress();
	msg_string += ":";
	for(int i = 0; i < m.getNumArgs(); i++){
		// get the argument type
		msg_string += " " + m.getArgTypeName(i);
		msg_string += ":";
		// display the argument - make sure we get the right type
		if(m.getArgType(i) == OFXOSC_TYPE_INT32){
			msg_string += ofToString(m.getArgAsInt32(i));
		}
		else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
			msg_string += ofToString(m.getArgAsFloat(i));
		}
		else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
			msg_string += m.getArgAsString(i);
		}
		else{
			msg_string += "unknown";
		}
	}
	return msg_string;
}
void testApp::broadcastFeedback(string control, int param) {
	//create a new OSC message
	ofxOscMessage m;
	m.setAddress("/feedback");
	m.addStringArg(control);
    m.addIntArg(param);
    
	//Send message to all known hosts
	// use another port to avoid a localhost loop
	for(unsigned int i = 0; i < knownClients.size(); i++){
		serverSender.setup(knownClients[i], serverRecvPort + 1);
		m.setRemoteEndpoint(knownClients[i], serverRecvPort + 1);
		serverSender.sendMessage(m);
		ofLogVerbose("Server broadcast message " + m.getArgAsString(0) + " to " + m.getRemoteIp()
					 + ":" + ofToString(m.getRemotePort()));
	}
    
}


void testApp::broadcastReceivedMessage(string message){
    
	//create a new OSC message
	ofxOscMessage m;
	m.setAddress("/feedback");
	m.addStringArg(message);
    
	//Send message to all known hosts
	// use another port to avoid a localhost loop
	for(unsigned int i = 0; i < knownClients.size(); i++){
		serverSender.setup(knownClients[i], serverRecvPort + 1);
		m.setRemoteEndpoint(knownClients[i], serverRecvPort + 1);
		serverSender.sendMessage(m);
		ofLogVerbose("Server broadcast message " + m.getArgAsString(0) + " to " + m.getRemoteIp()
					 + ":" + ofToString(m.getRemotePort()));
	}
}

