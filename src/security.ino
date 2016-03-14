#include <IRremote.h>
#include "printf.h"
uint8_t MAX_FAILS = 3;
uint8_t fails = 0;
uint8_t index_good = 0;
uint8_t index_bad = 0;
uint8_t ARRAY_SIZE = 10;
uint16_t good[10] = {0,0,0,0,0,0,0,0,0,0};
uint16_t bad[10] = {0,0,0,0,0,0,0,0,0,0};
int fire_DO = 5;
int IRpin = 6;
int open_door_ pin = 7;
int close_door_pin = 8;
IRrecv irrecv(IRpin);
decode_results results;
/**
* Code run on startup
*/
void setup(){
    pinMode(fire_DO, INPUT);

    pinMode(open_door_pin, OUTPUT);
    pinMode(close_door_pin, OUTPUT);

    Serial.begin(115200);
    printf_begin();
    initNRF();
    irrecv.enableIRIn(); // Start the IRreceiver
}

/**
* Code run continuously
*/
void loop(){
  //testRadio();
  if(digitalRead(fire_DO) == LOW){
    openDoor();
  }

  if (irrecv.decode(&results))
    {
      if(results.value != 4294967295){
        Serial.println(results.value, DEC);
        openDoor(results.value);
      }
      irrecv.resume(); // Receive the next value
    }
}

void openDoor(uint16_t code){
  if(preCheck(code)){
    return; // door is open | alarm is raised;
  }
  // send code to server to see if user can open this door.
  sendCode(code);
}



/**
* inital check for valit code and cache
* return true if preCheck was succesful, meaning no communication with the server is needed.
**/
bool preCheck(uint16_t code){
  // TODO build cache. Is there a timeout for a cached value?
  for(uint8_t x = 0; x < ARRAY_SIZE;x++){
    //  printf("%i\n", x);
      if(good[x] == code){
        openDoor();
        return true;
      }
      if(bad[x] == code){
        handleFail();
        return true;
      }
  }
  return false;

}


// add to good cache
void addToGoodCache(uint16_t value){
  if(index_good == ARRAY_SIZE){
      index_good = 0;
  }
  good[index_good] = value;
  index_good++;
}
// add to bad cache
void addToBadCache(uint16_t value){
  if(index_bad == ARRAY_SIZE){
      index_bad = 0;
  }
  bad[index_bad] = value;
  index_bad++;
}

// handle a response from NRF24, result + code entert
void handleResponse(uint16_t result, uint16_t code){
  if(result == 1){ // succes add code to cache
    addToGoodCache(code);
    openDoor();
    return;
  } // fail add fail to bad code cache
  addToBadCache(code);
  handleFail();

}

void handleFail(){
  fails++;
  if(fails >= MAX_FAILS){
    // TODO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! alarm
  }
}

/** Open the door code accepted.
*/
void openDoor(){
  fails = 0; // reset fails.
  // TODO open the door
}
