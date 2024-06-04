# Install and Setup

We will use Python and TensorFlow under our native OS (Windows/MacOS), and a virtual machine with Ubuntu 18.04 (Linux) with all the code pre-installed. You can edit the C++ source code with your native OS, using any visual editor you like (we recommend Visual Studio Code). For building and running the code, you will need to switch to the VM.
Let's see how to set up everything correctly.

### Install the Virtual Machine

1. Download and install VirtualBox 7.0
2. Download the virtual machine image from this [link](https://mega.nz/file/egkSHBaA#clABiSZ1keY_2XGSyEyRbgl6RDqnN85Zyz5QelKM4F8)
3. Open VirtualBox and click on `File` > `Import Appliance...` and load the `.ova` file you just downloaded. Follow the guided procedure.
4. Select the VM and click on `Settings` > `Shared folders`, and edit the entry `PW-FES-master`: change the folder path field and select the location of the project folder you downloaded from WeBeep (e.g., in my case: `C:\Users\Mattia\Desktop\PW-FES-master`).
   NOTE: Flag the Auto-mount option 
5. Save the settings and close. 
6. Run the script `setup_env.bat` as Administator (right click on the `.bat` file). **Warning**: the script will restart your computer, so be prepared.

NOTE: running `setup_env.bat` will disable Hyper-V on your machine to enable AVX instructions in the virtual machine. If you don't know what this means, you probably won't care too much... If you'll ever have problems with virtualization, re-enable Hyper-V with `bcdedit /set hypervisorlaunchtype on` and `DISM /Online /Enable-Feature:Microsoft-Hyper-V`.

### Install Python 3 and TensorFlow

You can install Python (we recommend version 3.9.6) following [these instructions](https://www.howtogeek.com/197947/how-to-install-python-on-windows/). We recommend also install a virtual environment manager, like [venv](https://docs.python.org/3/library/venv.html).

Once you're all set, you can install all the dependencies of the project with:

    python -m pip install -U pip
    python -m pip install -r requirements.txt

Note: while you could run also model training under the VM, there are several reasons why you should not do that.

### Running the Virtual Machine

Open VirtualBox and click 'Start' on the VM named `PW-FES`. When prompted for login, enter `pwfes` for the user and `20pwfes22` for the password. The VM should automatically mount the folder with the source code of the project and move to that directory. It should also automatically pass through the USB devices when you connect the FES stimulator or the Xsens Awinda station, so that you're ready to use them in your code.

### Build the code 

To build the C++ code and generate our applications, we can use the following commands:

    # Build FES code
    cd FES/build
    cmake .. 
    make -j4
    # Run the program
    ../stimulation_control

    # Build xsens code
    cd xsens/build
    cmake ..
    make -j4
    # Run the program
    ../mtw_driver_imu

Note: for startes, you can run directly the compiled binaries `mtw_driver_imu` and `stimulation_control` that you'll find in the directories `xsens` and `FES`. Every time you change the source, you will need to build as above before running it.

### Launch the XSENS app to acquire data

We can use the xsens app `mtw_driver_imu` to acquire data and save it to file. We can simply do this with:

    cd xsens
    ./mtw_driver_imu --working-mode 0

### Train the TensorFlow model

Once we have acquired some data, we can train our TensorFlow models. We can run directly TensorFlow on our main OS, opening the Jupyter Notebook and launching each cell.

### Run everything together

The Virtual Machine we provided does not have any graphical user interface to optimize the communication with the external peripherals. We can split the terminal into 3 windows and launch all the code that we need. To split it, just press `Ctrl+B` followed by `"` (i.e., `Shift+2`). To move between split screens you can use your arrows, i.e. you can press `Ctrl+B` followed by the up arrow key to go up.