

import processing.net.*;
int port = 12001;
String host = "166.78.61.139";
Client c;
String data;
  int val = 0;
  int tilt = 0;

void setup() 
{
  size(200, 200);
  // I know that the first port in the serial list on my mac
  // is always my  FTDI adaptor, so I open Serial.list()[0].
  // On Windows machines, this generally opens COM1.
  // Open whatever port is the one you're using.
  c = new Client(this, host, port);


}

void draw() {
  background(255);
  if (keyPressed) {
    if (key == 'l' || key == 'L') { 
      fill(204);  
      if (val < 90){      
      val = val +10;
      c.write("MOVE PAN <"+val+">;"); 
      // c.write("\r\n");
      println("left");
      }
    } 
    else if (key == 'r' || key == 'R') {                     
      fill(0);
      if (val > - 90){
      val = val - 10;
      c.write("MOVE PAN <"+val+">;");  
     // c.write("\r\n");
      println("right");
      }
    }

    else if (key == 'u' || key == 'U') {                   

    if (tilt > - 90){
      tilt = tilt - 10;
      c.write("MOVE TILT <"+tilt+">;");  
      // c.write("\r\n");
      println("up");
    }
    }


    else if (key == 'd' || key == 'D') {                   

      if (tilt < 90){
      tilt = tilt + 10;
      c.write("MOVE TILT <"+tilt+">;"); 
    //   c.write("\r\n");
      println("down");
    }
    }

    else if (key == 'c' || key == 'C') {                   
      fill(0);                     
      c.write("MOVE SIDE [CYCLE];"); 
     // c.write("\r\n");
      println("cycle");
    }


    else if (key == 's' || key == 'S') {                   
      fill(0);                     
      c.write("MOVE STOP;"); 
     // c.write("\r\n");
      println("stop");
    }


    else if (key == 'q' || key == 'Q') {       
      //byte out[] = new byte[2];
      //out[0] = byte(map(mouseX, 0, screenWidth, 0, 63));
      c.write("SOUND REPEAT <63>;");
     //  c.write("\r\n");
      println("shoudler");
    }          


    else if (key == 'w' || key == 'W') {       
      //byte out[] = new byte[2];
      //out[0] = byte(map(mouseX, 0, screenWidth, 0, 63));
      c.write("SOUND PLAY <0>;");
     // c.write("\r\n");
      println("shoudler");
    }          

    rect(50, 50, 100, 100);         // Draw a square
  }
}

