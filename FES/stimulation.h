// Stimulation headers
#include "smpt_ll_client.h"
#include "smpt_client.h"
#include "smpt_ml_client.h"
#include "smpt_messages.h"
#include "smpt_packet_number_generator.h"
#include "smpt_ll_packet_validity.h"

// Standard headers 
#include <stdio.h>
using namespace std;
#include <unistd.h>
using namespace std;


class stimulation{

public:

    /* Stimulator variables */
    const char *port_name= "/dev/ttyUSB0";
    Smpt_device device= {0};

    /* Mid Level */
    Smpt_ml_init ml_init = {0};  /* Struct for ml_init command */
    Smpt_ml_update ml_update = {0};   /* Struct for ml_update command */
    Smpt_ml_get_current_data ml_get_current_data = {0}; 

    /* Low Level */
    uint8_t packet_number = 0;  /* The packet_number can be used for debugging purposes */
    Smpt_ll_init ll_init = {0};       /* Struct for ll_init command */
    Smpt_ll_channel_config ll_channel_config = {0};   /* Struct for ll_channel_config command */
    Smpt_ll_channel_config channels[4];

    /*Train of pulses parameters*/
    int Number_of_points = 0; 
    int Ramp = 0; 
    int Period = 0; 
    int stim_current = 0;
    int stim_PW = 0;
    int Freq_stim = 40; 
    // int T_stim = 1/Freq_stim*1000;
    int Number_of_channels = 0; 

    /* Stimulator functions */ 
    void initialize_ml_stimulation();
    void initialize_ll_stimulation();
    void ml_stimulate();
    void ll_stimulate();
    void set_stimulation_variables();

    /* Other variables */
    bool check_sent = 0; 
    char mode[1];

};