# Xsens MTw Awinda
## Linux SDK for Data Acquisition/Streaming

This software can be used to log/stream/acquire MTw data using the Xsens C++ SDK. As of now, it is configured to provide 6-axis raw data from the accelerometer and the gyroscope. 

### Build
Before building this software, you need to meet some dependencies, most of which are specified in the `CMakeLists.txt`.
The main dependency is the Xsens SDK library, that has to be installed before building. It can be downloaded from this [link](https://www.xsens.com/cs/c/?cta_guid=cdd8ff0f-c5ab-45b9-aa8c-ad03ca91c13b&signature=AAH58kGDL3db1KGoKTs3aWnNXzctg0pJog&pageId=27796161161&placement_guid=82a3929d-008c-405c-9f34-03ebe55882f4&click=c0673600-1f38-4d89-a43f-98c656af2326&hsutk=66f62a887d2b6f6bf73fab5784292024&canon=https%3A%2F%2Fwww.xsens.com%2Fsoftware-downloads&utm_referrer=https%3A%2F%2Fwww.xsens.com%2Fresources&portal_id=3446270&redirect_url=APefjpFIGCmjfHQUFgaKfu16cwx9C2QOYhcQXqWUwU3CSDl2DbPC1SJ3d4fYEGBbwShF7lxUmbU7dfMBjRwY2sG1vxXjyL55pnr_dljCAbmTjlUUi4bwx8ZbPehYT7Ukin-yxbeHFh5rP8B6cX4xiien80xg_r3gVa0aHsLRy6T3KcKT_ANe1ScNHwvRgurzIax8TPfKSgGyJzLjCWb-Cr1DH3EqP0tRZshihi_j2THBUs_HoySzsKUqQVfRg8F2o553gL_SzdVi--qG53ApIDOH1nuqDEpwQY1EhXSDgGtp-gcCSqB1QCOnkc71MObYzIm_YrC09tciqkMpTbJ-UJrg6YnC7uHPmXvdpZvBZSKj9_j_DivlzAc&__hstc=81749512.66f62a887d2b6f6bf73fab5784292024.1630340323048.1630340323048.1637321771445.2&__hssc=81749512.3.1637321771445&__hsfp=29020463&contentType=standard-page).

To install all the downloaded libraries, unpack the `tar.gz` package and run the shell script. First, we need to install a few required dependencies.

    sudo apt install -y sharutils liblapack3

Then, we can extract and install the Xsens libraries. When prompted, select the default install path (i.e., `/usr/local/xsens`).

    tar -xvfz MT_Software_Suite_Linux_4.6.tar.gz
    cd MT_Software_Suite_Linux/
    chmod +x mtsdk_linux_4.sh
    sudo ./mtsdk_linux_4.6.sh

You can build the software with `cmake` and `make`:

    mkdir build && cd build/
    cmake ..
    make -j$(nproc)

### Usage

From the main project folder, you can run the software simply with:

    cd bin/
    ./mtw_driver_imu

Note: you can set the NICE value (process priority) for the data acquisition thread, that runs the `onLiveDataAvailable()` callback from the Xsens API. This can be done also on the general purpose Linux kernel, running `setcap` on the compiled binary ([source](https://stackoverflow.com/a/37528755/7084591)):

    sudo setcap 'cap_sys_nice=eip' mtw_driver_imu

You can run the software eitherway, it will report an error, but then continue running on normal priority.

### A note on the serial port and the USB/FTDI latency_timer

#### `dialout` group
We need to give our user I/O priviliges for serial ports to communicate with them. The best way to do this is to add our user to the `dialout` group, so that we can access serial ports with no need to uso `sudo`.

    sudo usermod -a -G dialout $USER

#### The latency timer

FTDI devices convert RS-232 or TTL serial signals to/from USB signals. The FTDI USB interface of the Xsens MTw Awinda station - as all other FTDI devices - has a latency timer set by default to 16 ms. This means that the data stream that is being read on the USB interface is buffered up to 16 milliseconds, reducing the actual data rate. 
We can improve the throughput by setting the latency timer to 1 ms, using the utility `setserial`. First, install `setserial`:

    sudo apt install setserial

Then, you can run the following command (assuming the Xsens Awinda station is `/dev/ttyUSB0`):

    setserial /dev/ttyUSB0 low_latency

