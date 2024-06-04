//---------------------------------------------------------------------------------------------------------------------
//
/// @file   mtw_imu_main.cpp
/// @author Mattia Pesenti
/// @date   March 2022
//
//---------------------------------------------------------------------------------------------------------------------
#include "include/project.h"
#include "include/p_gettid.h"
#include <arpa/inet.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define TCP_PORT 50500

// Working modes
#define OFFLINE_DATA_ACQUISITION	0
#define ONLINE_FES_CONTROL			1
// Set default working mode
volatile uint8_t _working_mode = ONLINE_FES_CONTROL;

volatile bool _running = false, _logDataAvailable = false;
static bool _verbose = false;
volatile bool _debug = DEBUG;

//---------------------------------------------------------------------------------------------------------------------
// LoggerFunction for the Logging Thread | Log data to the console
//---------------------------------------------------------------------------------------------------------------------
void LoggerFunction(std::vector<XsDataIMU6>& data, std::vector<MtwCallback*>& mtwc) {

	using namespace std::literals::chrono_literals;

	while( _running && _verbose ) {

		if( _logDataAvailable ) {

			_logDataAvailable = false;
			for( size_t i = 0; i < data.size(); i++ ) {
				std::cout << " [" << i << "] ID: " << mtwc[i]->device().deviceId().toString().toStdString()
						  << ", Axel: " << std::setw(7) << std::fixed << std::setprecision(2) << data[i].printAxel()
						  << "\tGyro: " << std::setw(7) << std::fixed << std::setprecision(2) << data[i].printGyro()
						  << "\n";
			}
			std::cout << std::endl;

			std::this_thread::sleep_for(900ms);

		}

	}

}

//---------------------------------------------------------------------------------------------------------------------
// Main
//---------------------------------------------------------------------------------------------------------------------
int main(int argc, char* argv[]) {

	unsigned int desiredUpdateRate = 100;	// 100 Hz data update rate
	unsigned int desiredRadioChannel = 16;	// Radio channel

	volatile bool high_priority = false;

	// Clear terminal screen
	system("clear");

	/* Parse command-line arguments */
	if( argc > 1 ) {
		for( uint8_t i = 1; i < argc; i++ ) {
			/* Update rate */
			if( strcmp(argv[i], "-fs") == 0 ) {
				desiredUpdateRate = atoi(argv[i+1]);
			}
			/* Radio Channel */
			if( strcmp(argv[i], "-ch") == 0 ) {
				uint8_t ch = atoi(argv[i+1]);
				desiredRadioChannel = (ch >= 11 && ch <= 25) ? ch : desiredRadioChannel;
			}
			/* Display version and exit */
			if( strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0 ) {
				std::cout << "MTw Driver Linux version " << MTW_VERSION << std::endl;
				return 0;
			}
			/* Set higher thread priority */
			if( (strcmp(argv[i], "-hp") == 0) || (strcmp(argv[i], "--high-priority") == 0) ) {
				high_priority = true;
			}
			/* Set working mode */
			if( (strcmp(argv[i], "-w") == 0) || (strcmp(argv[i], "--working-mode") == 0) ) {
				_working_mode = atoi(argv[i+1]);
				std::cout << green_key << " [INFO] " << color_key << "Working mode set to: " << std::to_string(_working_mode) << std::endl;
			}
			/* Enable debug mode*/
			if( strcmp(argv[i], "--debug") == 0 ) {
				_debug = true;
			}
		}
	}

	/// Callback for wireless master
	WirelessMasterCallback wirelessMasterCallback;
	/// Callbacks for mtw devices
	std::vector<MtwCallback*> mtwCallbacks;

	/* Welcome message */
	std::cout << " [INFO] MTw Driver Linux version " << boldgreen_key << MTW_VERSION << color_key << std::endl;

	std::cout << " Constructing XsControl..." << std::endl;
	XsControl* control = XsControl::construct();
	if( control == 0 ) std::cout << " Failed to construct XsControl instance." << std::endl;

	/* Instantiate MTwFileLogger */
	MTwLogger MTwLog = MTwLogger(_debug); 

	try {

		XsPortInfoArray detectedDevices = XsScanner::scanPorts();

		XsPortInfoArray::const_iterator wirelessMasterPort = detectedDevices.begin();
		while( wirelessMasterPort != detectedDevices.end() && !wirelessMasterPort->deviceId().isWirelessMaster() ) {
			++wirelessMasterPort;
		}
		if( wirelessMasterPort == detectedDevices.end() ) {
			throw std::runtime_error(" [ERROR] No wireless masters found");
		}
		std::cout << green_key << " [INFO]" <<color_key << " Wireless master found @ " << *wirelessMasterPort;

		std::cout << ". Opening port..." << std::endl;
		if( !control->openPort(wirelessMasterPort->portName().toStdString(), wirelessMasterPort->baudrate()) ) {
			std::ostringstream error;
			error << " [ERROR] Failed to open port " << *wirelessMasterPort;
			throw std::runtime_error(error.str());
		}

		std::cout << " Getting XsDevice instance for wireless master..." << std::endl;
		XsDevicePtr wirelessMasterDevice = control->device(wirelessMasterPort->deviceId());
		if( wirelessMasterDevice == 0 ) {
			std::ostringstream error;
			error << " [ERROR] Failed to construct XsDevice instance: " << *wirelessMasterPort;
			throw std::runtime_error(error.str());
		}

		std::cout << "Do you want to reset the Wireless Master? [Y/N]\n";
		while(1) {
			if( _kbhit() ) {
				char kbp = (char) _getch();
				if( toupper(kbp) == 'Y' ) {
					std::cout << boldpurple_key << "[INFO] Wireless Master is being reset..." << color_key << std::endl;
					if ( !wirelessMasterDevice->reset() ) {
						std::cout << boldred_key << " [WARNING]" << color_key << " Unable to reset the Wireless Master.\n";
					}
					XsTime::msleep(1000);
				}
				break;
			}
		}

		std::cout << green_key << " [INFO]" << color_key << " XsDevice instance created @ " << *wirelessMasterDevice << std::endl;

		std::cout << " Setting config mode..." << std::endl;
		if( !wirelessMasterDevice->gotoConfig() ) {
			std::ostringstream error;
			error << " [ERROR] Failed to set config mode: " << *wirelessMasterDevice;
			throw std::runtime_error(error.str());
		}

		std::cout << " Attaching callback handler..." << std::endl;
		wirelessMasterDevice->addCallbackHandler(&wirelessMasterCallback);

		const XsIntArray supportedUpdateRates = wirelessMasterDevice->supportedUpdateRates();
		std::cout << " Supported update rates: ";
		for( XsIntArray::const_iterator itUpRate = supportedUpdateRates.begin(); itUpRate != supportedUpdateRates.end(); ++itUpRate ) {
			std::cout << *itUpRate << " ";
		}
		std::cout << std::endl;

		/// Actual update rate (sampling frequency)
		const int newUpdateRate = findClosestUpdateRate(supportedUpdateRates, desiredUpdateRate);
		std::cout << green_key << " [INFO]" << color_key << " Setting update rate to " << newUpdateRate << " Hz" << std::endl;
		if( !wirelessMasterDevice->setUpdateRate(newUpdateRate) ) {
			std::ostringstream error;
			error << " [ERROR] Failed to set update rate: " << *wirelessMasterDevice;
			throw std::runtime_error(error.str());
		}
		MTwLog.updateRate = newUpdateRate;

		/// Get user/trial info
		std::cin.ignore(std::numeric_limits<std::streamsize>::max());
		if( _debug == false ) {
			std::cout << "User number:  ";
			MTwLog.user = std::cin.get();
			std::cout << "Trial number: ";
			MTwLog.trial = std::cin.get();
		}

		std::cout << " Disabling radio channel if previously enabled..." << std::endl;
		if( wirelessMasterDevice->isRadioEnabled() ) {
			if( !wirelessMasterDevice->disableRadio() ) {
				std::ostringstream error;
				error << " [ERROR] Failed to disable radio channel: " << *wirelessMasterDevice;
				throw std::runtime_error(error.str());
			}
		}

		/* If 'dev' branch, warn the user */
		if( std::string(MTW_VERSION).find("dev") != std::string::npos ) {
			std::cout << boldpurple_key << "\n\t [WARNING]" << color_key << " This is the dev branch!\n" << std::endl;
		}

		std::cout << green_key << " [INFO]" << color_key << " Setting radio channel to " << desiredRadioChannel << "\n Enabling radio..." << std::endl;
		if( !wirelessMasterDevice->enableRadio(desiredRadioChannel) ) {
			std::ostringstream error;
			error << " [ERORR] Failed to set radio channel: " << *wirelessMasterDevice;
			throw std::runtime_error(error.str());
		}

		//-------------------------------------------------------------------------------------------------------------
		// Master Configuration Complete
		//-------------------------------------------------------------------------------------------------------------

		std::cout << " Waiting for MTw to wirelessly connect..." << std::endl;
		bool waitForConnections = true;
		bool calibrationFlag = false;
		size_t connectedMTWCount = wirelessMasterCallback.getWirelessMTWs().size();

		do {
			XsTime::msleep(100);

			while( true ) {
				size_t nextCount = wirelessMasterCallback.getWirelessMTWs().size();
				if( nextCount != connectedMTWCount ) {
					std::cout << boldgreen_key << " Number of connected MTWs: " << nextCount << color_key 
							  << " >> Press 'Y' to start measurement or 'Z' to calibrate.\n";
					connectedMTWCount = nextCount;
				}
				else break;
			}
			if( _kbhit() ) {
				char kbp = (char) _getch();
				if( toupper(kbp) == 'Z' ) { 
					calibrationFlag = true; 
					kbp = 'Y'; 
				}
				waitForConnections = ( toupper(kbp) != 'Y' );
			}
		}
		while( waitForConnections );

		std::cout << green_key << "\n [INFO]" << color_key << " Preparing for data acquisition loop..." << std::endl;

		XsResultValue testSynch = control->testSynchronization();
		if( testSynch != XRV_OK ) 
			std::cout << purple_key << " [WARNING]" << color_key << " Test synch not OK\n";

		if( !wirelessMasterDevice->gotoMeasurement() ) {
			std::ostringstream error;
			error << " [ERORR] Failed to enable measurement mode: " << *wirelessMasterDevice;
			throw std::runtime_error(error.str());
		}

		std::cout << "\n Getting XsDevice instances for all MTw's..." << std::endl;
		XsDeviceIdArray allDeviceIds = control->deviceIds();
		XsDeviceIdArray mtwDeviceIds;
		for( XsDeviceIdArray::const_iterator i = allDeviceIds.begin(); i != allDeviceIds.end(); ++i ) {
			if( i->isMtw() ) 
				mtwDeviceIds.push_back(*i);
		}

		XsDevicePtrArray mtwDevices;
		for( XsDeviceIdArray::const_iterator i = mtwDeviceIds.begin(); i != mtwDeviceIds.end(); ++i ) {
			XsDevicePtr mtwDevice = control->device(*i);
			if( mtwDevice != 0 ) 
				mtwDevices.push_back(mtwDevice);
			else 
				throw std::runtime_error(" [ERROR] Failed to create an MTW XsDevice instance");
		}

		std::cout << " Attaching callback handlers to MTw's..." << std::endl;
		mtwCallbacks.resize(mtwDevices.size());
		for( uint8_t i = 0; i < (uint8_t) mtwDevices.size(); ++i ) {
			mtwCallbacks[i] = new MtwCallback(i, mtwDevices[i]);
			mtwDevices[i]->addCallbackHandler(mtwCallbacks[i]);
		}

		std::cout << " Attaching the MTw's to the log file..." << std::endl;
		MTwLog.init(std::ref(mtwDeviceIds), connectedMTWCount);

		if( calibrationFlag ) {
			std::cout << green_key << " [INFO]" << color_key << " Applying resetOrientation() to all MTws" << std::endl;
			for ( uint8_t i = 0; i < (uint8_t)mtwDevices.size(); i++ ) {
				control->device(mtwDeviceIds[i])->resetOrientation(XRM_Heading);
				control->device(mtwDeviceIds[i])->resetOrientation(XRM_Inclination);
				control->device(mtwDeviceIds[i])->resetOrientation(XRM_Alignment);
			}
		}

		std::cout << green_key << "\n [INFO]" << color_key << " Starting measurement..." << std::endl;

		/// First received packet packetCounter
		static uint16_t _firstPacketRecv = 0;
		/// Next packet 
		uint16_t nextPacket = 0;
		/// Drop count (threshold: 500)
		volatile uint16_t dropCount = 0;
		std::vector<uint8_t> nextPacketReady(connectedMTWCount);
		std::fill(nextPacketReady.begin(), nextPacketReady.end(), 0); // Set all elements to 0
		/// Packet ready to be logged
		std::vector<XsDataIMU6> dataPacket(connectedMTWCount);
		/// Global packet count
		uint32_t globalPacketCount = 1;

		/// Absolute first packetCounter value, used to init bufferCount
		/// Detect the first received packet in the while() loop
		static bool _firstPacket = true;

		/* Start console logging */
		uint16_t printCounter = 1;
		std::thread logger(LoggerFunction, std::ref(dataPacket), std::ref(mtwCallbacks));

		volatile bool _droppedPacket = false;

		/* Log % of dropped packets */
		uint16_t totalDroppedPackets = 0;
		float ratio = 0.0f;

		//-------------------------------------------------------------------------------------------------------------
		// Handle Thread Priority
		//-------------------------------------------------------------------------------------------------------------

		/* Default: do not handle thread priority. Enabled with CLI option --high-priority (-hp) */
		if( high_priority ) {
			pid_t callbackThread_id = pstream_gettid(0);
			std::cout << " [INFO] Setting priority for thread with PID " << callbackThread_id << std::endl;
			const int NICE_VALUE = -20; // move this to 'p_gettid.h' > #define NICE_VALUE -20
			int retval = setpriority(PRIO_PROCESS, callbackThread_id, NICE_VALUE);
			if( retval != 0 ) {
				std::cout << boldred_key << " [ERROR]" << color_key << " Unable to set priority for thread " << callbackThread_id << std::endl;
				std::cout << " Try running: sudo setcap 'cap_sys_nice=eip' mtw_driver_imu_dev \n";
			} 
			else {
				std::cout << green_key << " [INFO]" << color_key << " NICE value correctly set for thread " << callbackThread_id << std::endl;
			}
			std::cout << "\nPress any key to start the acquisition loop...\n";
			std::cin.get();
		}

		std::cout << " Opening TCP connection...\n";
		//-------------------------------------------------------------------------------------------------------------
		// TCP SOCKET 
		//-------------------------------------------------------------------------------------------------------------
		int server_fd = -1;
		int new_socket = -1;
		/// TCP Data buffer
		uint32_t buf_size = sizeof(float)*6*mtwCallbacks.size();	// 4 bytes * 3 axis * 2 channels * 6 sensors = 144 bytes
		uint8_t buf[buf_size];
		// Open socket only if we are working with the FES controller
		if( _working_mode == ONLINE_FES_CONTROL ) {
			server_fd = socket(AF_INET, SOCK_STREAM, 0);
			if ( server_fd == 0 ) {
				std::cout << " [ERROR] Failed to open socket\n";
			}
			int opt = 1;
			setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
			struct sockaddr_in address;
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = INADDR_ANY;
			address.sin_port = htons(TCP_PORT);
			int r = bind(server_fd, (struct sockaddr *) &address, sizeof(address));
			if ( r < 0 ) {
				std::cout << " [ERROR] Failed to bind the TCP socket\n";
			}
			listen(server_fd, 3);
			int addrlen = sizeof(address);
			int new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t*) &addrlen);
			if ( new_socket < 0 ) {
				std::cout << " [ERROR] Failed to accept TCP connection\n";
			}
			else {
				std::cout << " [INFO] TCP connection started.\n";
			}
		}
		

		//-------------------------------------------------------------------------------------------------------------
		// Data Acquisition Loop
		//-------------------------------------------------------------------------------------------------------------
		
		std::cout << "\n Main loop. Press any key to quit\n" << std::endl;
		_running = true;

		while( !_kbhit() ) {

			/* Get the first packet and its counter value */
			while( _firstPacket ) {
				for( size_t i = 0; i < mtwCallbacks.size(); i++ ) {
					if( mtwCallbacks[i]->dataAvailable() ) {
						_firstPacketRecv = mtwCallbacks[i]->getFirstPacketCounter();
						nextPacket = _firstPacketRecv+1;
						std::cout << " Start with packet: " << nextPacket << std::endl;
						_firstPacket = false;
						break;
					}
				}
			}

			XsTime::udelay(1); // Loop frequency > adjust according to number of connected MTw's

			/* Check whether the next packet is available */
			for( size_t i = 0; i < mtwCallbacks.size(); i++ ) {

				/* Check only if packet not ready */
				if( nextPacketReady[i] == 0 ) {
					/* Check for nextPacket */
					if( mtwCallbacks[i]->assertPacketReady(nextPacket) ) {
						/* Packet ready for i-th MTw */
						nextPacketReady[i]++;
					}
					else {
						dropCount++;
					}
				}

			}

			// Break condition: PACKET_DROP_COUNT new packets are ready while we are waiting for this: 
			// drop it and continue
			if( dropCount > 2500 ) {  // > adjust according to number of connected MTw's

				std::cout << " [WARNING] Dropped packet " << nextPacket;

				/* Delete the packet in the packet buffer (if any) */
				for( size_t i = 0; i < mtwCallbacks.size(); i++ ) {
					if( nextPacketReady[i] == 1 ) {
						mtwCallbacks[i]->deletePacket(nextPacket);
					}
				}

				/* Reset Counters */
				dropCount = 0;
				std::fill(nextPacketReady.begin(), nextPacketReady.end(), 0);

				_droppedPacket = true; // Set flag for DEBUG logging

				/* Drop the packet */
				nextPacket++;

				/* Update percentage */
				totalDroppedPackets++;
				ratio = 100.0f * ((float)totalDroppedPackets/(float)globalPacketCount);
				std::cout << " (" << totalDroppedPackets << " " << std::to_string(ratio) << " %)\n";

			}

			/* If the packet is ready, log it to file */
			if( std::accumulate(nextPacketReady.begin(), nextPacketReady.end(), 0) == connectedMTWCount ) {

				globalPacketCount++; // Update packet counter
				/* Reset Counters */
				std::fill(nextPacketReady.begin(), nextPacketReady.end(), 0); 
				dropCount = 0;

				/* Get the packet for all MTw's */
				for( size_t i = 0; i < mtwCallbacks.size(); i++ ) {
					// Get the packet from the packet buffer
					dataPacket[i] = mtwCallbacks[i]->getPacket(nextPacket);
					// Delete the element in the packet buffer
					mtwCallbacks[i]->deletePacket(nextPacket);
				}
				
				if( _working_mode == ONLINE_FES_CONTROL ) {
					// Pack the data into buf
					for( size_t i = 0; i < mtwCallbacks.size(); i++ ) {
						memcpy(&buf[6*sizeof(float)*i],                 &dataPacket[i].axel, 3*sizeof(float));
						memcpy(&buf[6*sizeof(float)*i+3*sizeof(float)], &dataPacket[i].gyro, 3*sizeof(float));
					}

					// Send the packet via the TCP socket
					int bytes_sent = send(new_socket, buf, buf_size, 0);
					if( _debug ) std::cout << "(" << std::to_string(globalPacketCount) << ")" << std::to_string(bytes_sent) << std::endl;

					// Reset the TCP buffer
					memset(buf, 0, sizeof(buf));
				}
				
				// Log the packet to file
				MTwLog.writePacket(dataPacket);
				
				// Move to the next packet
				nextPacket++;

			}

			// Log to console at ~ 1 Hz
			if( globalPacketCount % ((uint16_t) newUpdateRate) == 0 ) {
				_logDataAvailable = true;
			}

		}
		(void)_getch();

		_running = false;

		/* Check for data still in the buffer */
		uint8_t packetsToRead = 0;
		if( mtwCallbacks[0]->getPacketCount() > 0 && mtwCallbacks[0]->getBackPacket() > nextPacket ) {
			std::cout << "Additional packets may be available in the buffer.\n";
			for( uint8_t i = 0; i < mtwCallbacks.size(); i++ ) {
				uint8_t tmp = mtwCallbacks[i]->getPacketCount();
				packetsToRead = tmp > packetsToRead ? tmp : packetsToRead;
			}
			std::cout << std::to_string(packetsToRead) << " packets to read...\n";
		}
		while( packetsToRead > 0 ) {

			std::fill(nextPacketReady.begin(), nextPacketReady.end(), 0);
			for( uint8_t i = 0; i < connectedMTWCount; i++ ) {
				nextPacketReady[i] += mtwCallbacks[i]->assertPacketReady(nextPacket);
			}
			if( std::accumulate(nextPacketReady.begin(), nextPacketReady.end(), 0) == connectedMTWCount ) {
				for( size_t i = 0; i < mtwCallbacks.size(); i++ ) {	
					// Get the packet from the packet buffer
					dataPacket[i] = mtwCallbacks[i]->getPacket(nextPacket);
					// Delete the element in the packet buffer
					mtwCallbacks[i]->deletePacket(nextPacket);
				}
				MTwLog.writePacket(dataPacket);
			}
			else {
				std::cout << "Packet " << nextPacket << " not available.\n";
			}

			std::fill(nextPacketReady.begin(), nextPacketReady.end(), 0);
			nextPacket++;
			packetsToRead--;

		}

		//-------------------------------------------------------------------------------------------------------------
		// Data Acquisition Loop : stopped
		//-------------------------------------------------------------------------------------------------------------
		

		std::cout << "\n Setting config mode..." << std::endl;
		if (!wirelessMasterDevice->gotoConfig()) {
			std::ostringstream error;
			error << " [ERROR] Failed to set config mode: " << *wirelessMasterDevice;
			throw std::runtime_error(error.str());
		}

		std::cout << " Disabling radio... " << std::endl;
		if( !wirelessMasterDevice->disableRadio() ) {
			std::ostringstream error;
			error << " [ERROR] Failed to disable radio: " << *wirelessMasterDevice;
			throw std::runtime_error(error.str());
		}

		// Stop the logging thread
		logger.join();

		// Close the log file
		MTwLog.mtw_file.close();
		std::cout << " Closing the log file..." << std::endl;

	}
	catch( std::exception const & ex ) {
		std::cout << ex.what() << std::endl;
		std::cout << "****ABORT****" << std::endl;
	}
	catch(...) {
		std::cout << "Unknown error..." << std::endl;
		std::cout << "****ABORT****" << std::endl;
	}

	std::cout << " Closing XsControl..." << std::endl;
	control->close();

	std::cout << " Deleting MTw callbacks..." << std::endl;
	for( std::vector<MtwCallback*>::iterator i = mtwCallbacks.begin(); i != mtwCallbacks.end(); ++i )
		delete (*i);

	std::cout << "\n Exit code: 0\n" << std::endl;
	return 0;

}
