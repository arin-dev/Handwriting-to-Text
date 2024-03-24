import time
import numpy as np
import serial
import tensorflow as tf
import pandas as pd
from preprocessing import preprocessing_data

#  LOAD MODEL HERE :
model = tf.keras.models.load_model('final_model.h5')

output_string = ""

ser = serial.Serial('COM7', 9600)  # Adjust the port name and baud rate

acc = []
gyro = []
prev_no_data = 'X'

from sklearn.preprocessing import LabelEncoder

encoder= LabelEncoder()

def predict_output(data):
    y_pred1 = model.predict(data)
    y_pred_vals1 = np.argmax(y_pred1,axis=1)
    labels1 = encoder.inverse_transform(y_pred_vals1)
    labels_series1 = pd.Series(labels1)
    return labels_series1
    # MAKE US OF MODEL AND PREDICT DATA

print("To start writing again lower the pen with a jerk to stop writing lift the pen with some jerk!")

def sentence_maker(predicted_output):
    if(predicted_output=='<'):
        if(len(output_string)):
           output_string = output_string[:-1]
    elif(predicted_output=='>'):
            output_string = output_string + " "
    else:
        output_string = output_string + predicted_output
    
    print(output_string)

print("To start writing again lower the pen with a jerk to stop writing lift the pen with some jerk!")

while True:
    line = ser.readline().decode('utf-8').strip()
    if line:
        if line == "X":
            if(prev_no_data=='Y'):
                preproccesed_data = preprocessing_data(acc,gyro)
                acc = []
                gyro = []
                # predict_output use this function and predict data
                predicted_output = predict_output(preproccesed_data)
                predict_output(predicted_output)
                print("To start writing again lower the pen with a jerk to stop writing lift the pen with some jerk!")
                prev_no_data='N'
        else:
            prev_no_data = 'Y'
            data_str = line
            data_parts = data_str.split(" | ")
            values = data_parts[0].split(",")
            values = [float(val) for val in data_parts[1].split(",")]
            acc.append(values)  # Append the first set of values to accelerometer
            values = data_parts[1].split(",")  # Split each set of values by ","
            values = [float(val) for val in data_parts[1].split(",")]
            gyro.append(values)  # Append the second set of values to gyroscope
    else:
        print("There maybe some problem with connnection!")