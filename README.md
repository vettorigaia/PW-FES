# Intention detection for Functional Electrical Stimulation based Rehabilitation Exercises

## Setup

First, make sure you have Python 3.9.x and run the following commands. We recommend using a virtual environment (e.g., `pyenv`, more info [here](https://realpython.com/intro-to-pyenv/)).

    pip install upgrade pip
    pip install -r requirements.txt

### `xsens`

This folder contains the data acquisition software (C++) for the Xsens inertial sensors.

### `tensorflow`

This folder contains the Python code to be used both to train, validate, and test online the deep-learning models to perform human activity identification. The Jupyter notebook `train_lstm.ipynb` can be used to load the training datasets, tune the structure of the LSTM model, and evaluate it offline. The Python script `lstm_predict.py` can be later used to load a trained model and infer online.

### `fes`

This folder contains the C++ software to be used to communicate the Function Electrical Stimulation control unit.