#ifndef __PROJECT_H
	#define __PROJECT_H

	#include <string>
	#include <stdexcept>
	#include <iostream>
	#include <iomanip>
	#include <sstream>
	#include <set>
	#include <list>
	#include <utility>
	#include <xsens/xsmutex.h>
	#include <xsensdeviceapi.h>

	#include "logger.h"
	#include "conio.h"
	#include <xsens/xsdevice.h>
	#include <xsens/xsportinfo.h>

	#include <numeric>

	#include <chrono>
	#include <thread>

	#define PACKET_DROP_COUNT	50

	// Text Color Identifiers
	const std::string boldred_key    = "\033[1;31m";
	const std::string red_key        = "\033[31m";
	const std::string boldpurple_key = "\033[1;35m";
	const std::string yellow_key     = "\033[33m";
	const std::string blue_key       = "\033[36m";
	const std::string green_key      = "\033[32m";
	const std::string boldgreen_key  = "\033[1;32m";
	const std::string color_key      = "\033[0m";
	const std::string purple_key     = "\033[35m";

	int findClosestUpdateRate(const XsIntArray& supportedUpdateRates, const int desiredUpdateRate);

	extern volatile bool _running;

	/*! \brief Stream insertion operator overload for XsPortInfo */
	std::ostream& operator << (std::ostream& out, XsPortInfo const & p)
	{
		out << "Port: " << std::setw(2) << std::right << p.portNumber() << " (" << p.portName().toStdString() << ") @ "
			<< std::setw(7) << p.baudrate() << " Bd"
			<< ", " << "ID: " << p.deviceId().toString().toStdString()
		;
		return out;
	}

	/*! \brief Stream insertion operator overload for XsDevice */
	std::ostream& operator << (std::ostream& out, XsDevice const & d)
	{
		out << "ID: " << "\033[36m" << d.deviceId().toString().toStdString() << "\033[0m" << " (" << d.productCode().toStdString() << ")";
		return out;
	}

	//----------------------------------------------------------------------
	// Callback handler for wireless master
	//----------------------------------------------------------------------
	class WirelessMasterCallback : public XsCallback
	{
	public:
		typedef std::set<XsDevice*> XsDeviceSet;

		XsDeviceSet getWirelessMTWs() const
		{
			XsMutexLocker lock(m_mutex);
			return m_connectedMTWs;
		}

	protected:
		virtual void onConnectivityChanged(XsDevice* dev, XsConnectivityState newState)
		{
			XsMutexLocker lock(m_mutex);
			switch (newState)
			{
			case XCS_Disconnected:		/*!< Device has disconnected, only limited informational functionality is available. */
				std::cout << " [EVENT] MTW Disconnected > " << *dev << std::endl;
				m_connectedMTWs.erase(dev);
				break;
			case XCS_Rejected:			/*!< Device has been rejected and is disconnected, only limited informational functionality is available. */
				std::cout << " [EVENT] MTW Rejected > " << *dev << std::endl;
				m_connectedMTWs.erase(dev);
				break;
			case XCS_PluggedIn:			/*!< Device is connected through a cable. */
				std::cout << " [EVENT] MTW PluggedIn > " << *dev << std::endl;
				m_connectedMTWs.erase(dev);
				break;
			case XCS_Wireless:			/*!< Device is connected wirelessly. */
				std::cout << " [EVENT] MTW Connected > " << *dev << std::endl;
				m_connectedMTWs.insert(dev);
				break;
			case XCS_File:				/*!< Device is reading from a file. */
				std::cout << " [EVENT] MTW File > " << *dev << std::endl;
				m_connectedMTWs.erase(dev);
				break;
			case XCS_Unknown:			/*!< Device is in an unknown state. */
				std::cout << " [EVENT] MTW Unknown > " << *dev << std::endl;
				m_connectedMTWs.erase(dev);
				break;
			default:
				std::cout << " [EVENT] MTW Error > " << *dev << std::endl;
				m_connectedMTWs.erase(dev);
				break;
			}
		}
	private:
		mutable XsMutex m_mutex;
		XsDeviceSet m_connectedMTWs;
	};

	//----------------------------------------------------------------------
	// Callback handler for MTw
	// Handles onDataAvailable callbacks for MTW devices
	//----------------------------------------------------------------------
	class MtwCallback : public XsCallback
	{
	public:
		MtwCallback(int mtwIndex, XsDevice* device)
			:m_mtwIndex(mtwIndex)
			,m_device(device)
		{}

		/*! \brief Returns true/false whether the data buffer contains packets */
		bool dataAvailable() const
		{
			XsMutexLocker lock(m_mutex);
			return !m_packetBuffer.empty();
		}

		/*! \brief Get the index of this MTw */
		int getMtwIndex() const
		{
			return m_mtwIndex;
		}

		/*! \brief Get a const reference of this XsDevice */
		XsDevice const & device() const
		{
			assert(m_device != 0);
			return *m_device;
		}

		/*! \brief Get the number of packets stored in the data buffer */
		size_t getPacketCount() const 
		{
			XsMutexLocker lock(m_mutex);
			return m_packetBuffer.size();
		}

		/*! \brief Get the packet->packetCounter() for the first received packet */
		int32_t getFirstPacketCounter() const
		{
			return m_firstPacket;
		}

		/*! \brief Assert if the requested packet is ready (i.e., stored in the data buffer) */
		uint8_t assertPacketReady(uint16_t packetCounter) const 
		{

			XsMutexLocker lock(m_mutex);
			// Look for the packet with the specified packetCounter in the data buffer
			if( this->dataAvailable() ) {
				auto it = std::find_if(m_packetBuffer.begin(), m_packetBuffer.end(), [&](const std::tuple<XsDataPacket,uint16_t>& e) {return std::get<1>(e) == packetCounter;});
				if( it != m_packetBuffer.end() ) {
					m_nextPacketIndex = std::distance(m_packetBuffer.begin(), it);
					return 1;
				}
				else return 0;
			}
			else return 0;

		}

		/*! \brief Get the requested packet */
		XsDataIMU6 const getPacket(uint16_t packetCounter) const
		{
			
			XsMutexLocker lock(m_mutex);
			/* Get the XsDataPacket from the packet buffer */
			XsDataPacket const packet = std::get<0>(m_packetBuffer[m_nextPacketIndex]);
			/* Construct the XsDataIMU data packet */
			XsDataIMU6 const dataPacket = XsDataIMU6(packet.packetCounter(), packet.calibratedAcceleration(), packet.calibratedGyroscopeData());

			return dataPacket;

		}

		/*! \brief Delete the packet. Warning: this does not check wheter the packet to be deleted has been processed */
		void deletePacket(uint16_t packetCounter) 
		{

			XsMutexLocker lock(m_mutex);
			auto it = std::find_if(m_packetBuffer.begin(), m_packetBuffer.end(), [&](const std::tuple<XsDataPacket,uint16_t>& e) {return std::get<1>(e) == packetCounter;});
			uint16_t index = std::distance(m_packetBuffer.begin(), it);
			m_packetBuffer.erase(m_packetBuffer.begin() + index);

			lastLoggedPacket = packetCounter;

		}

		/*! \brief Returns the packetCounter of the first packet in the buffer */
		uint16_t getFrontPacket(void) const 
		{
			return std::get<1>(m_packetBuffer.front());
		}

		/*! \brief Returns the packetCounter of the last packet in the buffer */
		uint16_t getBackPacket(void) const
		{
			if( dataAvailable() ) return std::get<1>(m_packetBuffer.back());
			else 				  return 0;
		}

	protected:
		virtual void onLiveDataAvailable(XsDevice*, const XsDataPacket* packet) override
		{
			
			if( !_running ) return;
			
			XsMutexLocker lock(m_mutex);
			// NOTE: Processing of packets should not be done in this thread.

			uint16_t pCounter = packet->packetCounter();
			m_packetBuffer.push_back(std::make_tuple(*packet, pCounter));
			lastPacket = std::get<1>(m_packetBuffer.back());
						
			if( m_firstPacket == -1 && _running ) {
				m_firstPacket = lastPacket;
				lastPacket = m_firstPacket;
				lastLoggedPacket = m_firstPacket;
			}

			// Keep the size of m_packetBuffer limited --> resize this?
			// ATTENTION to memory usage and speed of std::find_if
			if( m_packetBuffer.size() > 800 ) m_packetBuffer.erase(m_packetBuffer.begin());

		}

	private:
		mutable XsMutex m_mutex;
		std::vector<std::tuple<XsDataPacket, uint16_t>> m_packetBuffer;
		int m_mtwIndex;
		XsDevice* m_device;
		int32_t m_firstPacket = -1;
		mutable uint32_t m_nextPacketIndex = 0;

	public:
		uint32_t lastPacket = 0;	
		uint32_t lastLoggedPacket = 0;

	};

	// -------------------------------------------------------------------------------------------------

	#define LOG(x) std::cout << x << std::endl;

#endif

