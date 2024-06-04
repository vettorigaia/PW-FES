#ifndef __LOGGER_H

  #define __LOGGER_H

  #include <fstream>
  #include <iostream>
  #include <sstream>
  #include <time.h>
  #include <mutex>
  #include <vector>
  #include <tuple>

  #include "XsDataIMU6.h"

  #define DEBUG true

  class MTwLogger {

    public:
      std::string fname = "";
      std::vector<std::tuple<std::string, uint32_t>> MTw_sensor_ID;
      std::ofstream mtw_file;
      uint16_t updateRate = 0;
      std::string user = "";
      std::uint32_t UserID = 0;
      std::string trial = "";
      std::string activity = "";
    private:
      time_t t;
      struct tm* now;
      uint8_t countMTw = 0;
      bool _isInit = false;
      uint32_t globalPacketCounter = 0;

    public:
      MTwLogger() 
      {
      }

      MTwLogger(bool debug) {

          if( debug ) {
            fname = "MTw_LogFile_debug.csv";
            std::cout << "\033[1;35m [WARNING]\033[0m Debug mode is active!" << std::endl;
          }
          else {
            fname = "MTw_LogFile_" + getFileNameTemplate() + ".csv";
          }

      }

      void init(XsDeviceIdArray& mtwids, size_t c) {

          // Attach the MTw Sensors ID
          for( uint8_t i = 0; i < c; i++ ) 
              MTw_sensor_ID.push_back(std::make_tuple(mtwids[i].toString().toStdString(), mtwids[i].toInt()));
          // Set the number of connected MTw's
          countMTw = c;

          // Init complete
          _isInit = true;

          // Open the log file
          openLogFile();

      }

      void openLogFile(void) {

          if( !_isInit ) {
              std::cout << " [WARNING] MTwLogger() not initialized properly." << std::endl;
          }

          // Open the log file
          mtw_file.open(fname);
          //countMTw = MTw_sensor_ID.size();

          // Throw runtime_error if the file is not open
          if( !mtw_file.is_open() ) {
              std::ostringstream error;
              error << "\033[1;31m [ERROR]\033[0m Unable to open the log file.\n [HINT] Are you running this from ./bin/" << std::endl;
              throw std::runtime_error(error.str());
          }
          std::cout << " [INFO] Opened log file\n";

          // Init the log file with some info...
          mtw_file << "/// MTw SDK version " << MTW_VERSION << std::endl;
          mtw_file << "/// Number of connected MTw sensors: " << std::to_string(countMTw) << std::endl;
          mtw_file << "/// MTw Sensor ID list: ";
          for ( uint8_t i = 0; i < countMTw-1; i++ ) {
              mtw_file << std::get<0>(MTw_sensor_ID[i]) << ", ";
          }
          mtw_file << std::get<0>(MTw_sensor_ID[countMTw-1]) << std::endl;
          mtw_file << "/// Update rate: " << std::to_string(updateRate) << " Hz" << std::endl;
          // User info
          #if DEBUG == false
          mtw_file << "/// User ID:  " << user << std::endl;
          mtw_file << "/// Trial n:  " << trial << std::endl;
          mtw_file << "/// Activity: " << activity << std::endl;
          #endif
          // File header
          mtw_file << "globalPacketCounter,";
          for ( uint8_t i = 0; i < countMTw-1; i++ ) {
              mtw_file << "packetCounter_" << std::get<0>(MTw_sensor_ID[i]).substr(6) // take only last two chars with substr()
                      << ",Axel_X_" << std::get<0>(MTw_sensor_ID[i]).substr(6) 
                      << ",Axel_Y_" << std::get<0>(MTw_sensor_ID[i]).substr(6)
                      << ",Axel_Z_" << std::get<0>(MTw_sensor_ID[i]).substr(6)
                      << ",Gyro_X_" << std::get<0>(MTw_sensor_ID[i]).substr(6)
                      << ",Gyro_Y_" << std::get<0>(MTw_sensor_ID[i]).substr(6)
                      << ",Gyro_Z_" << std::get<0>(MTw_sensor_ID[i]).substr(6) << ",";
          }
          mtw_file << "packetCounter_" << std::get<0>(MTw_sensor_ID[countMTw-1]).substr(6)
                      << ",Axel_X_" << std::get<0>(MTw_sensor_ID[countMTw-1]).substr(6)
                      << ",Axel_Y_" << std::get<0>(MTw_sensor_ID[countMTw-1]).substr(6)
                      << ",Axel_Z_" << std::get<0>(MTw_sensor_ID[countMTw-1]).substr(6)
                      << ",Gyro_X_" << std::get<0>(MTw_sensor_ID[countMTw-1]).substr(6)
                      << ",Gyro_Y_" << std::get<0>(MTw_sensor_ID[countMTw-1]).substr(6)
                      << ",Gyro_Z_" << std::get<0>(MTw_sensor_ID[countMTw-1]).substr(6)
                      << std::endl;

      }

      void writePacket(std::vector<XsDataIMU6>& data) {

          // Log data to the log file
          mtw_file << globalPacketCounter << ",";
          for( uint8_t i = 0; i < countMTw-1; i++ ) {
              mtw_file << data[i].counter << "," << data[i].printAxel() << "," << data[i].printGyro() << ",";
          }
          mtw_file << data[countMTw-1].counter << "," << data[countMTw-1].printAxel() << "," << data[countMTw-1].printGyro() << "\n";

          globalPacketCounter++;

      }

      uint8_t getBufferWriteThreshold(void) {

          uint8_t tmp = 0;
          for( uint8_t i = countMTw; i > 0; i-- ) tmp += i;
          return tmp;

      }

    private:
      std::string getFileNameTemplate(void) {
          t = time(0);
          now = localtime(&t);
          std::string fname_template = std::to_string(now->tm_year + 1900) + "_" +
                                          std::to_string(now->tm_mon +1) + "_" +
                                          std::to_string(now->tm_mday) + "_";
          char tmp[15];
          strftime(tmp, 15, "%H_%M_%S", now);
          fname_template += tmp;
          return fname_template;
      }

    };

#endif
