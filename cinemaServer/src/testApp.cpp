#include "testApp.h"

#define NUMBER_OF_PLAYERS 100
#define NUMBER_OF_ROWS    10
#define PLAYER_SIZE       35

//--------------------------------------------------------------
void testApp::setup() {
	ofSetVerticalSync(true);
    ofSetFrameRate(30);
	ofSetLogLevel(OF_LOG_VERBOSE);
    largeGameText.loadFont("mono.ttf", 60);
	
    //Server Setup
    serverRecvPort = 9000;
    serverReceiver.setup(serverRecvPort);
    maxServerMessages = 100;
    
    //Client Setup
    //SENDING
    clientDestination = "localhost";
    clientSendPort = 9000;
    
    clientSender.setup(clientDestination, clientSendPort);

    //RECEIVING
    clientRecvPort = clientSendPort + 1;
    clientReceiver.setup(clientRecvPort);
    
	box2d.init();
	box2d.setGravity(0, 0);
	box2d.createBounds();
	box2d.setFPS(30.0);
	box2d.registerGrabbing();
	for (int i = 0; i < 5; i++) {
        playerImages[i].loadImage("star.png");
    }
    
    
    int columns = NUMBER_OF_PLAYERS / NUMBER_OF_ROWS;
    int colPadding = (ofGetWidth()  - (columns        * PLAYER_SIZE))/columns + PLAYER_SIZE;
    int rowPadding = (ofGetHeight() - (NUMBER_OF_ROWS * PLAYER_SIZE))/ NUMBER_OF_ROWS + PLAYER_SIZE;

    for (int x = 0; x <= ofGetWidth(); x+= colPadding) {
        for (int y = 0; y <= ofGetHeight(); y+=rowPadding) {
        }
    }
}

//--------------------------------------------------------------
void testApp::update() {
	box2d.update();
    //MESSAGE QUEUE FROM OSC
    while (serverReceiver.hasWaitingMessages()) {
        ofxOscMessage msg;
        serverReceiver.getNextMessage(&msg);
        ofLogVerbose("Server Received Message: " + getOscMsgAsString(msg) + " from " + msg.getRemoteIp());
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
                p.get()->setPhysics(1.0, .5, 0.3);
                p.get()->setup(box2d.getWorld(), ofRandom(ofGetWidth()), ofRandom(ofGetHeight()), PLAYER_SIZE, PLAYER_SIZE);
                p.get()->color = ofColor(ofRandom(255), ofRandom(255), ofRandom(255));
                p.get()->image = &playerImages[0];
                
                ofxOscMessage msgToPlayer;
                //client will look for joined message, the argument is their player number
                msgToPlayer.setAddress("/joined");
                msgToPlayer.addIntArg(p.get()->playerId);
                msgToPlayer.addIntArg(p.get()->teamId);
                msgToPlayer.addIntArg(p.get()->subTeamId);
                //define shape ids
                //            msgToPlayer.addIntArg(p.get()->playerId);
                msgToPlayer.setRemoteEndpoint(incomingPlayer, serverRecvPort + 1);
                serverSender.sendMessage(msgToPlayer);
                players.push_back(p);
                
                ofLogVerbose("Sent message " + msgToPlayer.getArgAsString(0) + " to " + msgToPlayer.getRemoteIp()
                             + ":" + ofToString(msgToPlayer.getRemotePort()));

            }

        }
 
        if (msg.getAddress() == "/move") {
            if (msg.getNumArgs() == 3) {
                int playerNumber = msg.getArgAsInt32(0);
                float xSpeed = msg.getArgAsFloat(1);
                float ySpeed = msg.getArgAsFloat(2);
                players[playerNumber].get()->setVelocity(xSpeed, ySpeed);
                cout << "Changing " << playerNumber << " to " << xSpeed << ", " << ySpeed<< endl;
                
            }
        }
    }
}


//--------------------------------------------------------------
void testApp::draw() {
/*
    
	for(int i=0; i<circles.size(); i++) {
		ofFill();
		ofSetHexColor(0xf6c738);
		circles[i].get()->draw();
	}
	
	for(int i=0; i<boxes.size(); i++) {
		ofFill();
		ofSetHexColor(0xBF2545);
		boxes[i].get()->draw();
	}
*/
    for (int i = 0; i < players.size(); i++) {
        players[i].get()->display();
    }
	// draw the ground
	box2d.drawGround();
	
	
	
	string info = "";
	info += "FPS: "+ofToString(ofGetFrameRate(), 1)+"\n";
	ofSetHexColor(0x444342);
	ofDrawBitmapString(info, 30, 30);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key) {
	
	if(key == 't') ofToggleFullscreen();
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

