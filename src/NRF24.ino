

#include <SPI.h>
#include "RF24.h"
#include "printf.h"

// The various roles supported by this sketch
typedef enum { role_sender = 1, role_receiver } role_e;

// The debug-friendly names of those roles debug things
const char* role_friendly_name[] = { "invalid", "Sender", "Receiver"};
role_e role;
//const short role_pin = 3;

//
// settings
//
RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };   // Radio pipe addresses for the 2 nodes to communicate.
uint8_t MAX_ATTEMPTS = 5;
uint16_t TIMEOUT_MS = 500;


// setup radio
void initNRF(){
  // pinMode(role_pin, INPUT);
  // digitalWrite(role_pin,HIGH);

  radio.begin();                           // Setup and configure rf radio
  radio.setRetries(15,15);
  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(8);

  // read the address pin, establish our role
  // if (digitalRead(role_pin) ) {
  //   role = role_sender;
  // }  else {
  //   role = role_receiver;
  // }
  // set pipes
  role = role_sender;
  if ( role == role_sender )
  {
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(0,pipes[1]);
  } else {
    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(0,pipes[0]);
  }

  printf("ROLE: %s\n\r",role_friendly_name[role]);
  radio.startListening();                 // Start listening
  radio.printDetails();
  radio.powerUp();
}

// keep sending a test code
void testRadio(){
  if ( role == role_sender ) {
    unsigned long x = 551489775;
    sendCode(x);
    delay(5000);
  } else {
    readCode();
  }
}


/**
* Send code to server
**/
void sendCode(unsigned long code) {

  bool timeout = true;
  uint8_t attempt = 1;
  // run once, more if server does not send a response
  do {
    // First, stop listening so we can talk.
    radio.stopListening();
    printf("Now sending %lu...",code);

    bool ok = radio.write( &code, sizeof( unsigned long) );
    if(ok){
      printf("ok...");
    } else { // only is true if there is a response within 60ms.
      printf("failed....");
    }

    // give server some time to respond
    timeout = waitForResponse();
    // Describe the results
    if (!timeout )
    {
      // Grab the response, compare, and send to debugging spew
      unsigned long got_result;
      radio.read( &got_result, sizeof( unsigned long) );
      // Spew it
      printf("Got response %li\n\r",got_result);
      handleResponse(got_result, code);
      // we are done
      return;
    }
    printf("Failed, response timed out.\n\r");
    attempt++;
    delay(1000);
  } while(timeout && attempt <= MAX_ATTEMPTS);

  // we tried X times but no response;
  handleFail();
}


// wait for the server to send a response.
// this functions block for max @TIMEOUT_MS
bool waitForResponse(){

  // Now, continue listening
  radio.startListening();
  delay(10); // a little time to start
  // Wait here until we get a response, or timeout (250ms)
  unsigned long started_waiting_at = millis();
  bool timeout = false;
  while ( ! radio.available() && ! timeout ) {
    if (millis() - started_waiting_at > TIMEOUT_MS ){
      timeout = true;
    }
  }
  return timeout;

}

// some code for making a receiver arduino
void readCode() {
  // if there is data ready
  if (radio.available())
  {
    printf("read\n");
    // Dump the payloads until we've gotten everything
    unsigned long got_code;
    bool done = false;
    while (!done)
    {
      // Fetch the payload, and see if this was the last one.
      done = radio.read( &got_code, sizeof( unsigned long) );

      // Spew it
      printf("Got payload %lu...",got_code);

      // Delay just a little bit to let the other unit
      // make the transition to receiver
      delay(20);
    }

    // First, stop listening so we can talk
    radio.stopListening();
     unsigned long x;
    // Send the final one back.
    if(got_code == 551489775){
       x = 1;
    } else {
       x = 0;
    }
    radio.write( &x, sizeof( unsigned long) );
    printf("Sent response.\n\r");
    // Now, resume listening so we catch the next packets.
    radio.startListening();
  }
}
