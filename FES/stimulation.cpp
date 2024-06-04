#include "stimulation.h"

void stimulation::initialize_ml_stimulation(){
   
    /* Clear ml_init struct and set the data */
    smpt_clear_ml_init(&ml_init);
    ml_init.packet_number = smpt_packet_number_generator_next(&device);

    /* Send the initialized data packet to the device */
    smpt_send_ml_init(&device, &ml_init);

    /* Prepare the update data packet */
    smpt_clear_ml_update(&ml_update);

    /* Enable the communication with the stimulator channels 
    !! You can choose to enable all the channels or only the ones that you are going to use */

    ml_update.enable_channel[Smpt_Channel_Red] = true;
    // ml_update.enable_channel[Smpt_Channel_Blu] = true;
    // ml_update.enable_channel[Smpt_Channel_White] = true;
    // ml_update.enable_channel[Smpt_Channel_Black] = true;

    /* Send the initialized packet to the stimulator*/
    ml_update.packet_number = smpt_packet_number_generator_next(&device);
}

void stimulation::set_stimulation_variables(){

    // Number of active points that describe the single stimulation pulse [0 15]
    Number_of_points = 2; 

    // Default value
    Ramp = 3; 

    // Period between to adjacent pulses [ms]
    Period = 25; 

    // Amplitude of stimulation
    stim_current = 10;

    // Pulsewidth of stimulation
    stim_PW = 300;

    // Active channels of the stimulator [0 3]
    Number_of_channels = 3; 

}

void stimulation::ml_stimulate(){

    /*For each channel you should define all the properties of the stimulation as follow: */ 
    

    for (int i=0; i<3; i++){

        ml_update.channel_config[i].number_of_points = Number_of_points;
        ml_update.channel_config[i].ramp = Ramp;
        ml_update.channel_config[i].period = Period; 

        for (int j=0; j<Number_of_points; j++){

            /* Biphasic waveform */ 
            ml_update.channel_config[i].points[j].current = stim_current;
            ml_update.channel_config[i].points[j].current = 0;
            ml_update.channel_config[i].points[j].current = -stim_current;

            ml_update.channel_config[i].points[j].time = stim_PW;
            ml_update.channel_config[i].points[j].time = 0;
            ml_update.channel_config[i].points[j].time =stim_PW;

        }
    }

    // Send packet
    smpt_send_ml_update(&device, &ml_update);
    Smpt_ml_get_current_data ml_get_current_data = {0};
    ml_get_current_data.packet_number = smpt_packet_number_generator_next(&device);
    ml_get_current_data.data_selection[Smpt_Ml_Data_Stimulation] = true;

    // You can print the output of the function to check if the command has been sent
    check_sent = smpt_send_ml_get_current_data(&device, &ml_get_current_data);

}


void stimulation::initialize_ll_stimulation()
{
    channels[0].enable_stimulation = true;
    channels[0].channel = Smpt_Channel_Red;
    channels[0].number_of_points = Number_of_points;

    channels[1].enable_stimulation = true;
    channels[1].channel = Smpt_Channel_Blue;
    channels[1].number_of_points = Number_of_points;

    channels[2].enable_stimulation = true;
    channels[2].channel = Smpt_Channel_Black;
    channels[2].number_of_points = Number_of_points;

    channels[3].enable_stimulation = true;
    channels[3].channel = Smpt_Channel_White;
    channels[3].number_of_points = Number_of_points;

    Smpt_ll_init ll_init = {0};  /* Struct for ll_init command */
    Smpt_ll_init_ack ll_init_ack = {0};  /* Struct for ll_init_ack response */
    Smpt_ack ack = {0};  /* Struct for general response */

    smpt_clear_ll_init(&ll_init);
    ll_init.packet_number = smpt_packet_number_generator_next(&device);
}

void stimulation::ll_stimulate(){

  
    /* Set the stimulation pulse */
    /* First point, current: 10 mA, positive, pulse width: 300 µs */
    ll_channel_config.points[0].current =  stim_current;
    ll_channel_config.points[0].time    = stim_PW;

    /* Second point, pause 100 µs */
    ll_channel_config.points[1].current =  0;
    ll_channel_config.points[1].time    = stim_PW;
    
    /* Third point, pause 100 µs */
    ll_channel_config.points[2].current =  -stim_current;
    ll_channel_config.points[2].time    = stim_PW;

    /* Send the ll_channel_list command to the stimulation unit */
    check_sent=smpt_send_ll_channel_config(&device, &ll_channel_config);

    packet_number++;

}