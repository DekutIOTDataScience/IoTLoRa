#include "mbed.h"
#include "lorawan_network.h"
#include "CayenneLPP.h"
#include "mbed_events.h"
#include "mbed_trace.h"
#include "lora_radio_helper.h"
#include "SX1276_LoRaRadio.h"
#include "LoRaWANInterface.h"
#include "standby.h"

#define STANDBY_TIME_S                   0.5 * 60

extern EventQueue ev_queue;

static uint32_t DEV_ADDR   =      0x2601135A;
static uint8_t NWK_S_KEY[] =      { 0x6C, 0xCB, 0x86, 0xD9, 0xE5, 0x3E, 0xD6, 0xE7, 0x06, 0x7D, 0x49, 0x5F, 0x1B, 0xC0, 0xD4, 0x2F };
static uint8_t APP_S_KEY[] =      { 0xC5, 0xC1, 0xFF, 0xF5, 0xF7, 0x28, 0xE9, 0xAA, 0x04, 0xB7, 0xC1, 0x8B, 0x59, 0x3F, 0x62, 0x3D };
AnalogIn moist(A2);

  //global variable
float moisture; 
bool moist_updated = false;

void moist_measure(){
    moisture=moist.read();
    moist_updated = true;
    }
    
void check_for_updated_moist() {
    if (moist_updated){
        moist_updated = false ;
        printf("Measure Value = %f ",moisture);
        
        CayenneLPP payload(50);
        static float moisture_value =moisture;
        printf("Moisture Value=%f\n", moisture_value);
        payload.addAnalogInput(4, static_cast<float>(moisture_value));
        
        if (!lorawan_send(&payload)){
           
            standby(STANDBY_TIME_S);
            }
        }
    }

static void lora_event_handler(lorawan_event_t event) {
    switch (event) {
        case CONNECTED:
            printf("[LNWK][INFO] Connection - Successful\n");
            break;
        case DISCONNECTED:
            ev_queue.break_dispatch();
            printf("[LNWK][INFO] Disconnected Successfully\n");
            break;
        case TX_DONE:
            printf("[LNWK][INFO] Message Sent to Network Server\n");

         //   delete distance;
            standby(STANDBY_TIME_S);
            break;
        case TX_TIMEOUT:
        case TX_ERROR:
        case TX_CRYPTO_ERROR:
        case TX_SCHEDULING_ERROR:
            printf("[LNWK][INFO] Transmission Error - EventCode = %d\n", event);

            //delete distance;
            standby(STANDBY_TIME_S);
            break;
        case RX_DONE:
            printf("[LNWK][INFO] Received message from Network Server\n");
            receive_message();
            break;
        case RX_TIMEOUT:
        case RX_ERROR:
            printf("[LNWK][INFO] Error in reception - Code = %d\n", event);
            break;
        case JOIN_FAILURE:
            printf("[LNWK][INFO] OTAA Failed - Check Keys\n");
            break;
        default:
            MBED_ASSERT("Unknown Event");
    }
}


int main() {
    set_time(0);
    
    printf("=========================================\n");
    printf("      DeKUT Innovator IoT- LoRa System        \n");
    printf("=========================================\n");

    lorawan_setup(DEV_ADDR, NWK_S_KEY, APP_S_KEY, lora_event_handler);

    printf("Measuring Moisture Value...\n");
    
  //immediately measure the sensor value
     wait_ms(100);
     moisture= moist.read();
    printf("Measuring Moisture =%f...\n",moisture);
      moist_measure();
     ev_queue.call_every(3000, &check_for_updated_moist);   

    ev_queue.dispatch_forever();
}



