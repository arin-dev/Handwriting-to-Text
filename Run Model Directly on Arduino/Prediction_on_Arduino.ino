#include <Arduino.h>
// #include <String.h>
// #include <ArduinoSTL.h>
#include <Arduino_LSM9DS1.h>
#include <TensorFlowLite.h>
#include "model.h"

// INCUDING MODEL CODES HERE

tflite::MicroInterpreter* interpreter;
TfLiteTensor* input_tensor;
TfLiteTensor* output_tensor;

// TILL HERE
#define req_size 60
int curr_len = 0;
bool is_data_writing = false;
const unsigned long INACTIVITY_TIMEOUT = 2000;
unsigned long lastActivityTime = 0;
// const unsigned char model[] PROGMEM = {...};

// vector<int> ax, ay, az, gx, gy, gz;
// vector<vector<int>> concatenated(6, vector<int>(req_size));
float ax[150], ay[150], az[150], gx[150], gy[150], gz[150];
float concatenated[6*60];
// float input_to_model[6][60]

void setup() {
  tflite::Model model = tflite::GetModel(model);
  if (model == nullptr) {
    Serial.println("Failed to load model!");
    while (1);
  }
  // Create a TensorFlow Lite interpreter
  interpreter = new tflite::MicroInterpreter(model);

  // Allocate memory for the model's tensors
  interpreter->AllocateTensors();

  // Get pointers to the input and output tensors
  input_tensor = interpreter->input(0);
  output_tensor = interpreter->output(0);

  Serial.begin(9600);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  Serial.println("IMU initialized! Please proceed to type: ");
  // Serial.println("Accelerometer and Gyroscope values:");

  // INCLUDE MODEL HERE ---------------------------------------------------
}

void linearInterpolation(float values[], int originalSize) {
    // int originalSize = values.size();
    // int originalSize = sizeof(values) / sizeof(values[0])
    float interpolated[req_size];
    if (originalSize > req_size) {
        // Reduce the size to 60 by removing elements
        int step = originalSize / req_size; // Calculate step size
        // int interpolated[req_size];

        for (int i = 0; i < req_size; i++) {
            interpolated[i] = values[i * step]; // Take every 'step' element
        }

        // Update the input vector with interpolated values
        // values = interpolated; 
        // ADD AT A LATER INSTANCE
    } else if (originalSize < req_size) {
        // Increase the size to 60 by interpolating additional values
        // vector<int> interpolated(req_size, 0); // Initialize with zeros
        
        for (int i = 0; i < originalSize - 1; i++) {
            int startIdx = i * (req_size - 1) / (originalSize - 1);
            int endIdx = (i + 1) * (req_size - 1) / (originalSize - 1);
            int valueDiff = values[i + 1] - values[i];
            int stepDiff = endIdx - startIdx;

            for (int j = startIdx; j <= endIdx; j++) {
                interpolated[j] = values[i] + (j - startIdx) * valueDiff / stepDiff;
            }
        }
        interpolated[req_size - 1] = values[originalSize - 1];
    }

    for (int i = 0; i < req_size; i++) {
      values[i] = interpolated[i];
    }
}


void make_final_array(float ax[], float ay[], float az[],
                      float gx[], float gy[], float gz[],
                      float concatenated[6][60], int originalSize) {
    linearInterpolation(ax, originalSize);
    linearInterpolation(ay, originalSize);
    linearInterpolation(az, originalSize);
    linearInterpolation(gx, originalSize);
    linearInterpolation(gy, originalSize);
    linearInterpolation(gz, originalSize);

    // for (int i = 0; i < req_size*6; i++) {
        // concatenated[i] = ax[i];
        // concatenated[60*1+i] = ay[i];
        // concatenated[60*2+i] = az[i];
        // concatenated[60*3+i] = gx[i];
        // concatenated[60*4+i] = gy[i];
        // concatenated[60*5+i] = gz[i];
    // }

    for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 60; ++j) {
      input_tensor->data.int16[i * 60 + j] = input_data[i][j];
    }
  }
}



void loop() {
  float a_x, a_y, a_z;
  float g_x, g_y, g_z;

  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
  
    IMU.readAcceleration(a_x, a_y, a_z);
    IMU.readGyroscope(g_x, g_y, g_z);

    if(g_y<-300)
    {
      is_data_writing = true;
      Serial.println("You can start writing now: ");
    }
    if(g_y>300)
      is_data_writing=false;


    // Read and process the incoming data if needed
    if(is_data_writing)
    {
      ax[curr_len] = (a_x);
      ay[curr_len] = (a_y);
      az[curr_len] = (a_z);
      gx[curr_len] = (g_x);
      gy[curr_len] = (g_y);
      gz[curr_len] = (g_z);
      curr_len++ ;
      lastActivityTime = millis();

      Serial.println(curr_len);
    }
    // Add your data processing logic here
  }

  if (millis() - lastActivityTime > INACTIVITY_TIMEOUT) { 
    Serial.println("Likh le kuch behen ke laude!!");
    make_final_array(ax, ay, az, gx, gy, gz, concatenated, curr_len);
    Serial.println("Size of data collected: " + String(curr_len) );    // PASS final array to model
    // PASS concatenated to model
    // Process output - USE MODEL AND PREDICTION HERE AND THEN OUTPUT FINAL ANSWER
    Serial.println("Inactivity timeout reached!");
    curr_len = 0;
    lastActivityTime = millis();

    // OUTPUT from MODEL below

    // Run inference
    interpreter->Invoke();

    // Get the output value from the output tensor
    int output_value = output_tensor->data.int16[0];

    // Process the output value (e.g., print it)
    Serial.print("Output value: ");
    Serial.println(output_value);

    // TILL HERE
  }
  delay(23);
}