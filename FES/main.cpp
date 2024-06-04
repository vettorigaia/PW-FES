#include "stimulation.h"
#include "tcp_socket.h"
#include <stdio.h>
using namespace std;
#include <unistd.h>
#include <thread>
#include <string.h>

using namespace std;
char stim_mode = 0; 

// TensorFlow socket buffer
const uint32_t tf_buf_size = 4;
uint8_t tf_buf[tf_buf_size];

void f_tf_socket_read(void);
int tf_socket;
volatile bool tf_data_ready = false;
// Prediction from TF model (Python)
int32_t task_pred = 0;

int main(int argc, char** argv) {
 
   for( size_t i = 1; i < argc; i++ ) {
      if( strcmp(argv[i], "--level") == 0 || strcmp(argv[i], "-l") == 0 ) {
         stim_mode = char(*argv[i+1]);
      }
   }

   stimulation stim;

   // Open serial port - Rename serial port with your port name (i.e. /dev/ttyUSB0) in stimulation class 
   smpt_open_serial_port(&stim.device, stim.port_name);

   // Set stimulation parameters    
   stim.set_stimulation_variables();
   cout << "Parameters setting done" << endl; 
   
   if( stim_mode == 0 ) {
      cout << "Select the Stimulation Mode of Operation (LowLevel/MidLevel) [L/M]: " ;
      cin >> stim_mode; 
   }
   
   // Connect with the TensorFlow model over TCP
   tf_socket = connect_socket(host, port);
   // Launch a new thread that reads incoming data from the TF model
   std::thread t_tf_read(f_tf_socket_read);

   if((stim_mode == 'm') || (stim_mode == 'M')){

      stim.initialize_ml_stimulation();
      cout << "Mid Level Stimulation Initialization done" << endl;

   }
   else if((stim_mode=='l') || (stim_mode=='L')){

      stim.initialize_ll_stimulation();
      cout << "Low Level Stimulation Initialization done" << endl;

   }
   else {

      cout << "Invalid selection.\nExiting...\n";
      exit(1);

   }

   while(1) {

      if((stim_mode == 'm') || (stim_mode == 'M')){

         // Check for new data from TF socket
         if( tf_data_ready ) {
            tf_data_ready = false;
            // Copy the bytes into our variable
            memcpy(&task_pred, tf_buf, 4);
            cout << "[tf_socket] received: " << std::to_string(task_pred) << endl;
         }
         /* Stimulate the corresponding muscles*/
         /*With ML mode you have to update the pkg at least every 2 seconds to "wake up" stimulation*/
         if(task_pred==1){
            stim.ml_stimulate();
            cout << "check_sent" << stim.check_sent << endl;
         }

         sleep(1);
      
      }
      else if((stim_mode=='l') || (stim_mode=='L')){

         /*With LL mode you have to a new pulse at the Stimulation Frequency*/
         stim.ll_stimulate();
         cout << "check_sent" << stim.check_sent << endl;
         usleep(stim.Period);
      
      }
   
   }

   t_tf_read.join();

   smpt_close_serial_port(&stim.device);

}

void f_tf_socket_read() {

   while(1) {

      // recv() is blocking so we run this in a separate thread to avoid blocking the stimulation control
      ssize_t n_bytes_read = recv(tf_socket, tf_buf, 4, 0);
      // Signal the main thread when we have new data
      if( n_bytes_read > 0 ) tf_data_ready = true;

   }

}
