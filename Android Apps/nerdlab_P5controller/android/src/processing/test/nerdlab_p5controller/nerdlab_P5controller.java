package processing.test.nerdlab_p5controller;

import processing.core.*; 
import processing.data.*; 
import processing.event.*; 
import processing.opengl.*; 

import apwidgets.*; 
import oscP5.*; 
import netP5.*; 
import android.view.inputmethod.EditorInfo; 
import android.view.inputmethod.InputMethodManager; 
import android.text.InputType; 
import android.content.Context; 
import android.view.MotionEvent; 

import java.util.HashMap; 
import java.util.ArrayList; 
import java.io.File; 
import java.io.BufferedReader; 
import java.io.PrintWriter; 
import java.io.InputStream; 
import java.io.OutputStream; 
import java.io.IOException; 

public class nerdlab_P5controller extends PApplet {










APWidgetContainer widgetContainer; 
APEditText hostTextField;
APEditText nameTextField;
APButton playButton;
PImage logo;
OscP5 oscP5;
NetAddress myRemoteLocation;
String host;
String chosenGame;
int playerNumber;
int tapCount;
int playerTeam;
int playerSubteam;
int playerAvatar;
int playerIndex;
int gameState;
int gameControl;
int score;
String instructions;
String playerName;
String chosenGame;
int playerColor;
PVector velocity;
PVector position;
PVector smooth;
float holdingTime;
float rotation;
boolean holding;
boolean playing;
//PImage playerImage[];
ArrayList<PImage> playerImage;
PImage bgImage;
PFont joystix;
int playerColor;

final int PORT = 9000;
final float ACCELEROMETER_PADDING = .15f;
final int GAME_STATE_NO_SERVER_CONNECTION = -1;
final int GAME_STATE_WAITING = 0;
final int GAME_STATE_JOIN = 999;
final int GAME_STATE_PAUSED = 1;
final int GAME_STATE_PLAYING = 2;
final int GAME_CONTROL_MOVE = 0;
final int GAME_CONTROL_AUDIO = 1;
final int GAME_CONTROL_ACCEL = 2;
final int GAME_CONTROL_TAP = 3;
final int GAME_CONTROL_ROTATE = 4;

final int IMAGE_SET_SQUARE  =  0;
final int IMAGE_SET_ABSTRACT=  1;
final int IMAGE_SET_HUMANS  =  2;
final int IMAGE_SET_TANKS   =  3;

public void setup() {
 
  tapCount = 0;
  orientation(PORTRAIT);
  logo = loadImage("bg-nerd_lab_logo.png");
  bgImage = loadImage("bg-1.png");
  playerImage = new PImage[10];
  for (int i = 0; i < 10; i++) {
     playerImage[i] = loadImage(i + ".png"); 
  }
  position = new PVector(width/2,height/2,0);
  velocity = new PVector(width/2,height/2,0);
  rotation = 0;
  joystix = createFont("joystix.ttf", 48);
  textFont(joystix, 48);
  gameState = GAME_STATE_NO_SERVER_CONNECTION;
  widgetContainer = new APWidgetContainer(this); //create new container for widgets
  nameTextField = new APEditText(10, logo.height + 32 + 70 + 20, width-20, 70);
  hostTextField = new APEditText(10, logo.height + 32 + 70 + 15 + 100, width-20, 70); //create a textfield from x- and y-pos., width and height
  playButton = new APButton(10, height/2 - 10, width-20, 70, "play"); //create new button from x- and y-pos. and label. size determined by text content
  nameTextField.setInputType(InputType.TYPE_CLASS_TEXT);
  nameTextField.setImeOptions(EditorInfo.IME_ACTION_NEXT);
  hostTextField.setInputType(InputType.TYPE_CLASS_PHONE); 
  hostTextField.setImeOptions(EditorInfo.IME_ACTION_DONE); //Enables a Done button
  hostTextField.setCloseImeOnDone(true); //close the IME when done is pressed
  widgetContainer.addWidget(nameTextField);
  widgetContainer.addWidget(hostTextField); //place textField in container
  widgetContainer.addWidget(playButton); //place button in container
  oscP5 = new OscP5(this,9001);
  playing = false;
  chosenGame = "no game chosen";
  playerImages = new ArrayList<PImage>();
}

public void draw() {
  fill(255);
  switch (gameState) {
  case GAME_STATE_NO_SERVER_CONNECTION:
    background(0);
    image(logo, 0, 0, displayWidth, logo.height);
    textFont(joystix, 12);
    text("Name", 10, logo.height + 32 + 70 + 15);
    text("Host or IP Address", 10, logo.height + 32 + 70 + 20 + 70 + 20);
    
    break;
  case GAME_STATE_JOIN:
    myRemoteLocation = new NetAddress(host, PORT);
    joinGame();
    gameState = GAME_STATE_WAITING;
    break;
  case GAME_STATE_WAITING:
    background(0);
    textFont(joystix, 48);
    text("Waiting for\nPlayers", 10, height/2);
    textFont(joystix, 24);
    text("Double Tap\nTo Quit", 10, height/2 + 100);
    break;
  case GAME_STATE_PAUSED:
    break;
  case GAME_STATE_PLAYING:
    background(0);
    if (playing) {
        tint(playerColor);
        image(playerImage[playerAvatar], position.x, position.y);
    }
  
    break;
  default:
    break;
  }
}

//onClickWidget is called when a widget is clicked/touched
public void onClickWidget(APWidget widget){
  
  if(widget == playButton){ 
    host = hostTextField.getText();
    playerName = nameTextField.getText();
    widgetContainer.hide();
    hideVirtualKeyboard();
    gameState = GAME_STATE_JOIN;
    println("Joining game on " + host);
  }
}

public void showVirtualKeyboard() {
  InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
  imm.toggleSoftInput(InputMethodManager.SHOW_FORCED,0);
}

public void hideVirtualKeyboard() {
  InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
  imm.toggleSoftInput(InputMethodManager.HIDE_IMPLICIT_ONLY, 0);
}
public void loadImageSet(int set) {
  playerImages.clear();
  String directoryPath;
  switch(set) {
     case IMAGE_SET_ABSTRACT:
      directoryPath = "abstract"; 
      break;
    case IMAGE_SET_HUMANS:
      directoryPath = "humans";
      break;
    case IMAGE_SET_SQUARE:
      directoryPath = "square";
      break;
    case IMAGE_SET_TANKS:
      directoryPath = "tanks";
      break; 
  }
  
  java.io.File folder = new java.io.File(dataPath(directoryPath));
  java.io.FilenameFilter pngFilter = new java.io.FilenameFilter() {
    boolean accept(File dir, String name) {
      return name.toLowerCase().endsWith(".png");
    }
  };
    
  String[] filenames = folder.list(pngFilter);
  for (int i = 0; i < filenames.length; i++) {
    PImage newImage;
    newImage = loadImage(filenames[i]);
    playerImages.add(newImage);
  }  
    
}


public void joinGame() {
  OscBundle bndl = new OscBundle();
  OscMessage joinMessage = new OscMessage("/join");
  joinMessage.add(playerName); /* add an int to the osc message */
  bndl.add(joinMessage);
  oscP5.send(bndl, myRemoteLocation);  
}

public void move(float x, float y) {
  OscBundle bndl = new OscBundle();
  OscMessage moveMessage = new OscMessage("/move");
  moveMessage.add(playerNumber);
  moveMessage.add(x); /* add an int to the osc message */
  moveMessage.add(y);
  bndl.add(moveMessage);
  oscP5.send(bndl, myRemoteLocation);  
  
}

public void tap() {
  OscBundle bndl = new OscBundle();
  OscMessage tapMessage = new OscMessage("/tap");
  tapMessage.add(playerNumber);
  bndl.add(tapMessage);
  oscP5.send(bndl, myRemoteLocation);  
}

public void rotatePlayer(float angle) {
  OscBundle bndl = new OscBundle();
  OscMessage rotateMessage = new OscMessage("/rotate");
  rotateMessage.add(playerNumber);
  rotateMessage.add(angle); /* add an int to the osc message */
  bndl.add(rotateMessage);
  oscP5.send(bndl, myRemoteLocation);  
 
}

public void oscEvent(OscMessage theOscMessage) {
  if (theOscMessage.checkAddrPattern("/quit")) {
    //no heartbeat, doesn't matter?      
  }

  if (theOscMessage.checkAddrPattern("/reset")) {
      OscBundle bndl = new OscBundle();
      OscMessage resetMessage = new OscMessage("/alive");
      bndl.add(resetMessage);
      oscP5.send(bndl, myRemoteLocation);  
  }

  if (theOscMessage.checkAddrPattern("/feedback")) {
    if (theOscMessage.get(0).stringValue().equals("control")) {

      gameControl = theOscMessage.get(1).intValue();
    }
    if (theOscMessage.get(0).stringValue().equals("instructions")) {
      instructions = theOscMessage.get(1).stringValue();
    }
    if (theOscMessage.get(0).stringValue().equals("score")) {
      score = theOscMessage.get(1).intValue();
    }
  }
  
  if (theOscMessage.checkAddrPattern("set")) {
    if (theOscMessage.get(0).stringValue().equals("state")) {
      gameState = theOscMessage.get(1).intValue();
    }
    if (theOscMessage.get(0).stringValue().equals("playing")) {
      playing = theOscMessage.get(1).intValue();
      gameState = GAME_STATE_PLAYING;  
    }
    if (theOscMessage.get(0).stringValue().equals("control")) {
      gameControl = theOscMessage.get(1).intValue();
    }
    if (theOscMessage.get(0).stringValue().equals("color")) {
      playerColor = color(theOscMessage.get(1).intValue(), theOscMessage.get(2).intValue(), theOscMessage.get(3).intValue());
    }
    if (theOscMessage.get(0).stringValue().equals("image")) {
      playerAvatar = theOscMessage.get(1).intValue();
    }

    if (theOscMessage.get(0).stringValue().equals("id")) {
      playerNumber = theOscMessage.get(1).intValue();
    }

    if (theOscMessage.get(0).stringValue().equals("images")) {
      loadImageSet(theOscMessage.get(1).intValue());
    }

    if (theOscMessage.get(0).stringValue().equals("index")) {
      playerIndex = theOscMessage.get(1).intValue();
    }

    if (theOscMessage.get(0).stringValue().equals("game")) {
      chosenGame = theOscMessage.get(1).stringValue();
    }

    if (theOscMessage.get(0).stringValue().equals("instructions")) {
      instructions = theOscMessage.get(1).stringValue();
    }

    if (theOscMessage.get(0).stringValue().equals("score")) {
      score = theOscMessage.get(1).intValue();
    }
  }

  print("### received an osc message.");
  print(" addrpattern: "+theOscMessage.addrPattern());
  println(" typetag: "+theOscMessage.typetag());
}

public void mousePressed() {
  println("tap!");
    if (playing) {
     velocity.x = mouseX;
     velocity.y = mouseY;     
     if (gameControl == GAME_CONTROL_TAP) {
          tap();
      }
    holding = true;
  }
  if (gameState == GAME_STATE_WAITING) {
    tapCount++;
    if (tapCount > 2) {
        gameState = GAME_STATE_NO_SERVER_CONNECTION;
        widgetContainer.show();
        tapCount = 0;
    }
  }
}

public void mouseDragged() {
  if (gameControl == GAME_CONTROL_MOVE && playing) {   
        println("moving!");
          position.x = mouseX;
          position.y = mouseY;  
          move(velocity.x - position.x, velocity.y - position.y);            
   }
   
   if (gameControl == GAME_CONTROL_ROTATE && playing) {
       position.x = mouseX;
       position.y = mouseY;
       rotation = velocity.x - position.x;
       rotatePlayer(rotation);
       println("rotating!");
        
   }
}

  public int sketchWidth() { return displayWidth; }
  public int sketchHeight() { return displayHeight; }
}
