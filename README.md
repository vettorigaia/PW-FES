![logo_polimi](https://github.com/vettorigaia/PW-FES/assets/150171386/26bab3f8-3d90-495f-8a41-2200aa9273e0)

# Scope
This project was realized as a team project in the course "Neuroengineering" during my Master's Degree in Biomedical Engineering at Politecnico di Milano.

A system was developed to assist individuals with muscle impairments in performing essential movements by utilizing accelerometers and gyroscopic sensors to track arm movements such as reaching for targets and directional pointing. 

Signal processing of the sensor data in MATLAB was undertaken to extract key features for each movement. 

Subsequently, these features were utilized to train a Long Short-Term Memory (LSTM) neural network model in Python for real-time recognition of movements and rest positions through the Xsens device.

Furthermore, the device was integrated with electrodes using C++ coding to issue electrical stimulation commands upon the identification of specific movements. This practical support facilitated the completion of intended actions effectively.

This collaborative project was undertaken during the second year of the Master’s Degree course in Neuroengineering, under the guidance of Professor Pedrocchi Alessandra Laura Giulia and Professor Cerveri Pietro.

# Intention detection for Functional Electrical Stimulation based Rehabilitation Exercises

### Setup
Python 3.9.x is needed. A virtual environment is recommended (e.g. 'pyenv', more info [here](https://realpython.com/intro-to-pyenv/)).

    pip install upgrade pip
    pip install -r requirements.txt

### `xsens`

This folder contains the data acquisition software (C++) for the Xsens inertial sensors.

### `tensorflow`

This folder contains the Python code to be used both to train, validate, and test online the deep-learning models to perform human activity identification. The Jupyter notebook `train_lstm.ipynb` can be used to load the training datasets, tune the structure of the LSTM model, and evaluate it offline. The Python script `lstm_predict.py` can be later used to load a trained model and infer online.

### `fes`

This folder contains the C++ software to be used to communicate the Function Electrical Stimulation control unit.
