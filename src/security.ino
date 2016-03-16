#include <IRremote.h>
#include "printf.h"
uint8_t MAX_FAILS = 3;
uint8_t fails = 0;
uint8_t index_good = 0;
uint8_t ARRAY_SIZE = 10;
unsigned long good[10] = {0,0,0,0,0,0,0,0,0,0};
int fire_DO = 5;
int IRpin = 6;
int door_ pin = 7;
IRrecv irrecv(IRpin);
decode_results results;
bool code_loaded_from_cache = false;
/**
* Code run on startup
*/
void setup(){
    pinMode(fire_DO, INPUT);
    pinMode(door_pin, OUTPUT);

    Serial.begin(115200);
    printf_begin();
    initNRF();
    irrecv.enableIRIn(); // Start the IRreceiver

    delay(10); // so the door closes properly
    closeDoor();
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

void openDoor(unsigned long code){
  code_loaded_from_cache = preCheck(code);
  if(code_loaded_from_cache{
    openDoor(); // open door so user dont have to wait for a response from the server.
  }
  // send code to server to see if user can open this door.
  sendCode(code);
}



/**
* Check cache for code.
* if code exists in cache this function returns true
**/
bool preCheck(unsigned long code){
  for(uint8_t x = 0; x < ARRAY_SIZE;x++){
      if(good[x] == code){
        return true;
      }
  }
  return false;

}


// add a code to the cache.
// a maximun of 10 codes is stored in the chace
void addToGoodCache(unsigned long value){
  if(index_good == ARRAY_SIZE){
      index_good = 0;
  }
  good[index_good] = value;
  index_good++;
}


// handle a response from NRF24, result + code entert
void handleResponse(unsigned long result, unsigned long code){
  if(result == 1){ // succes add code to cache

    // check if door already has been opend.
    if(!code_loaded_from_cache){
      openDoor();
    } else {
      addToGoodCache(code);
    }
    return;
  } // fail add fail to bad code cache

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
  digitalWrite(door_pin, HIGH);
}


void closeDoor(){
  digitalWrite(door_pin, LOW);
}
