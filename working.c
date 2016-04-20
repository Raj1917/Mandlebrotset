/*
 *  bmpServer.c
 *  1917 serve that 3x3 bmp from lab3 Image activity
 *
 *  Created by Tim Lambert on 02/04/12.
 *  Containing code created by Richard Buckland on 28/01/11.
 *  Copyright 2012 Licensed under Creative Commons SA-BY-NC 3.0. 
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <assert.h>
#include <complex.h>
#include <math.h>
#include <unistd.h>

int waitForConnection (int serverSocket);
int makeServerSocket (int portno);
void serveBMP (int socket);
int escapeSteps(double x, double y);
void zoomRegion(double x, double y, int levels);
void createHeader (int socket);


#define SIMPLE_SERVER_VERSION 1.0
#define REQUEST_BUFFER_SIZE 1000
#define DEFAULT_PORT 8777         //The port for connection and URL
#define NUMBER_OF_PAGES_TO_SERVE 10
// after serving this many pages the server will halt

#define MAGICHEADER 0x4d42
#define BYTES_PER_PIXEL 3
#define OFFSET 54
#define NO_COMPRESSION 0
#define IMAGESIZE 512
#define DIB_HEADER_SIZE 40
#define PIX_PER_METRE 2835
#define NUMBER_PLANES 1
#define NUM_COLORS 0
//Maximum number of steps correlate to the number of colours
#define MAX_STEPS 256
//Store the coordinates in this struct
typedef struct _coordinate {
  double x;
  double y;
} Coordinate;
typedef unsigned char oneByte;
typedef unsigned short twoBytes;
typedef unsigned int fourBytes;

int main (int argc, char *argv[]) {
      
   printf ("************************************\n");
   printf ("Starting simple server %f\n", SIMPLE_SERVER_VERSION);
   printf ("Serving bmps since 2012\n");   

   int serverSocket = makeServerSocket (DEFAULT_PORT);   
   printf ("Access this server at http://localhost:%d/\n", DEFAULT_PORT);
   printf ("************************************\n");

   char request[REQUEST_BUFFER_SIZE];

   int numberServed = 0;
   while (numberServed < NUMBER_OF_PAGES_TO_SERVE) {
      
      printf ("*** So far served %d pages ***\n", numberServed);

      int connectionSocket = waitForConnection (serverSocket);
      // wait for a request to be sent from a web browser, open a new
      // connection for this conversation

      // read the first line of the request sent by the browser  
      int bytesRead;
      bytesRead = read (connectionSocket, request, (sizeof request)-1);
      assert (bytesRead >= 0); 
      // were we able to read any data from the connection?
            
      // print entire request to the console 
      printf (" *** Received http request ***\n %s\n", request);

      //send the browser a simple html page using http

      double x = 0, y = 0;
      int levels, z;
      //==========================================================================
      //==========================================================================

      //==========================================================================

      levels = sscanf (request, "GET /tile_x%lf_y%lf_z%d.bmp", &x, &y, &z);
      printf("The levels of zoom: %d\n", z);
      printf("x value is %lf, y value is %lf\n", x, y);
      printf("===========================================\n");
      //==========================================================================
      //==========================================================================

      //==========================================================================


      serveBMP(connectionSocket);
      
      // close the connection after sending the page- keep aust beautiful
      close(connectionSocket);
      
      numberServed++;
   } 

   
   // close the server connection after we are done- keep aust beautiful
   printf ("** shutting down the server **\n");
   close (serverSocket);
   
   return EXIT_SUCCESS; 
}

int escapeSteps (double x, double y) {
    Coordinate coords;
    coords.x = x;
    coords.y = y;
    double zx = 0.0;
    double zy = 0.0;
    int steps = 0;
    while ( zx*zx + zy*zy <= 4 && steps < MAX_STEPS) {
        double zxcoord = zx*zx - zy*zy + coords.x;
        double zycoord = 2*zy*zx + coords.y;

        zx = zxcoord;
        zy = zycoord;

        steps++;
    }
    return steps;
}

void serveBMP (int socket) {
   char* message;
   
   // first send the http response header
   
   // (if you write stings one after another like this on separate
   // lines the c compiler kindly joins them togther for you into
   // one long string)
   message = "HTTP/1.0 200 OK\r\n"
                "Content-Type: image/bmp\r\n"
                "\r\n";
   printf ("about to send=> %s\n", message);
   write (socket, message, strlen (message));
   createHeader (socket);
   
   unsigned char RGB = 0;
   unsigned char white[] = {255, 255, 255};
   unsigned char black[] = {0, 0, 0};
   unsigned char greyscale[3];

  int zoom = 8;
  double enhance = pow(2, -zoom);
  double x, y;
  double xstart = x - 0.5*IMAGESIZE*enhance;
  double ystart = y - 0.5*IMAGESIZE*enhance;
  double xend = x + 0.5*IMAGESIZE*enhance;
  double yend = y + 0.5*IMAGESIZE*enhance;

  for ( y = ystart; y < yend; y+=enhance ) {
    for ( x = xstart; x < xend; x+=enhance) {
      int steps = escapeSteps(x, y);
      if ( steps < 256) {
        greyscale[0] = steps;
        greyscale[1] = steps;
        greyscale[2] = steps;
        write (socket, &greyscale, sizeof greyscale);
      }
      else {
        write (socket, &black, sizeof black);
      }
    }
  } 
}


/*
void serveBMP (int socket) {
   char* message;
   
   // first send the http response header
   
   // (if you write stings one after another like this on separate
   // lines the c compiler kindly joins them togther for you into
   // one long string)
   message = "HTTP/1.0 200 OK\r\n"
                "Content-Type: image/bmp\r\n"
                "\r\n";
   printf ("about to send=> %s\n", message);
   write (socket, message, strlen (message));
   
   // now send the BMP
   unsigned char bmp[] = {
     0x42,0x4d,0x5a,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x36,0x00,0x00,0x00,0x28,0x00,
     0x00,0x00,0x03,0x00,0x00,0x00,0x03,0x00,
     0x00,0x00,0x01,0x00,0x18,0x00,0x00,0x00,
     0x00,0x00,0x24,0x00,0x00,0x00,0x13,0x0b,
     0x00,0x00,0x13,0x0b,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x07,
     0xff,0x07,0x07,0x07,0x07,0x07,0xff,0x00,
     0x00,0x0e,0x07,0x07,0x07,0x66,0x07,0x07,
     0x07,0x07,0x07,0x00,0x00,0x0d,0x07,0x07,
     0x07,0x07,0x07,0x07,0xff,0xff,0xff,0x00,
     0x00,0x0d};


   write (socket, bmp, sizeof(bmp));
}*/


void createHeader (int socket){

   twoBytes magicNumber = MAGICHEADER;
   write (socket, &magicNumber, sizeof magicNumber);

   fourBytes fileSize = OFFSET + (IMAGESIZE * IMAGESIZE * BYTES_PER_PIXEL);
   write (socket, &fileSize, sizeof fileSize);

   fourBytes reserved = 0;
   write (socket, &reserved, sizeof reserved);

   fourBytes offset = OFFSET;
   write (socket, &offset, sizeof offset);

   fourBytes dibHeaderSize = DIB_HEADER_SIZE;
   write (socket, &dibHeaderSize, sizeof dibHeaderSize);

   fourBytes width = IMAGESIZE;
   write (socket, &width, sizeof width);

   fourBytes height = IMAGESIZE;
   write (socket, &height, sizeof height);

   twoBytes planes = NUMBER_PLANES;
   write (socket, &planes, sizeof planes);

   twoBytes bitsPerPixel = BYTES_PER_PIXEL * 8;
   write (socket, &bitsPerPixel, sizeof bitsPerPixel);

   fourBytes compression = NO_COMPRESSION;
   write (socket, &compression, sizeof compression);

   fourBytes imageSize = (IMAGESIZE * IMAGESIZE * BYTES_PER_PIXEL);
   write (socket, &imageSize, sizeof imageSize);

   fourBytes hResolution = PIX_PER_METRE;
   write (socket, &hResolution, sizeof hResolution);

   fourBytes vResolution = PIX_PER_METRE;
   write (socket, &vResolution, sizeof vResolution);

   fourBytes numColors = NUM_COLORS;
   write (socket, &numColors, sizeof numColors);

   fourBytes importantColors = NUM_COLORS;
   write (socket, &importantColors, sizeof importantColors);
}



// start the server listening on the specified port number
int makeServerSocket (int portNumber) { 
   
   // create socket
   int serverSocket = socket (AF_INET, SOCK_STREAM, 0);
   assert (serverSocket >= 0);   
   // error opening socket
   
   // bind socket to listening port
   struct sockaddr_in serverAddress;
   memset ((char *) &serverAddress, 0,sizeof (serverAddress));
   
   serverAddress.sin_family      = AF_INET;
   serverAddress.sin_addr.s_addr = INADDR_ANY;
   serverAddress.sin_port        = htons (portNumber);
   
   // let the server start immediately after a previous shutdown
   int optionValue = 1;
   setsockopt (
      serverSocket,
      SOL_SOCKET,
      SO_REUSEADDR,
      &optionValue, 
      sizeof(int)
   );

   int bindSuccess = 
      bind (
         serverSocket, 
         (struct sockaddr *) &serverAddress,
         sizeof (serverAddress)
      );
   
   assert (bindSuccess >= 0);
   // if this assert fails wait a short while to let the operating 
   // system clear the port before trying again

   return serverSocket;
}

// wait for a browser to request a connection,
// returns the socket on which the conversation will take place
int waitForConnection (int serverSocket) {
   // listen for a connection
   const int serverMaxBacklog = 10;
   listen (serverSocket, serverMaxBacklog);
   
   // accept the connection
   struct sockaddr_in clientAddress;
   socklen_t clientLen = sizeof (clientAddress);
   int connectionSocket = 
      accept (
         serverSocket, 
         (struct sockaddr *) &clientAddress, 
         &clientLen
      );
   
   assert (connectionSocket >= 0);
   // error on accept
   
   return (connectionSocket);
}