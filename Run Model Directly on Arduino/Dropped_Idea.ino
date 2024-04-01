#include <Arduino.h>
#include <ArduinoSTL.h>

#include <TensorFlowLite.h>

using namespace std;

#define req_size 60;

bool is_data_writing = false;

vector<int> ax, ay, az, gx, gy, gz;
vector<vector<int>> concatenated(6, vector<int>(req_size));

const unsigned long INACTIVITY_TIMEOUT = 4000;
unsigned long lastActivityTime = 0;

void setup() {
  Serial.begin(9600);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  Serial.println("IMU initialized! Please proceed to type: ");
  // Serial.println("Accelerometer and Gyroscope values:");

  // INCLUDE MODEL HERE ---------------------------------------------------
}

int linearInterpolation(vector<int>& values) {
    int originalSize = values.size();
    const int desiredSize = 60;

    if (originalSize > desiredSize) {
        // Reduce the size to 60 by removing elements
        int step = originalSize / desiredSize; // Calculate step size
        vector<int> interpolated;

        for (int i = 0; i < desiredSize; i++) {
            interpolated.push_back(values[i * step]); // Take every 'step' element
        }

        // Update the input vector with interpolated values
        values = interpolated;
    } else if (originalSize < desiredSize) {
        // Increase the size to 60 by interpolating additional values
        vector<int> interpolated(desiredSize, 0); // Initialize with zeros

        for (int i = 0; i < originalSize - 1; i++) {
            int startIdx = i * (desiredSize - 1) / (originalSize - 1);
            int endIdx = (i + 1) * (desiredSize - 1) / (originalSize - 1);
            int valueDiff = values[i + 1] - values[i];
            int stepDiff = endIdx - startIdx;

            for (int j = startIdx; j <= endIdx; j++) {
                interpolated[j] = values[i] + (j - startIdx) * valueDiff / stepDiff;
            }
        }

        interpolated[desiredSize - 1] = values[originalSize - 1];
        return interpolated;
    }
}


void make_final_array(vector<int>& ax, vector<int>& ay, vector<int>& az,
                      vector<int>& gx, vector<int>& gy, vector<int>& gz,
                      vector<vector<int>>& concatenated) {
    ax = linearInterpolation(ax);
    ay = linearInterpolation(ay);
    az = linearInterpolation(az);
    gx = linearInterpolation(gx);
    gy = linearInterpolation(gy);
    gz = linearInterpolation(gz);

    for (int i = 0; i < req_size; i++) {
        concatenated[0][i] = ax[i];
        concatenated[1][i] = ay[i];
        concatenated[2][i] = az[i];
        concatenated[3][i] = gx[i];
        concatenated[4][i] = gy[i];
        concatenated[5][i] = gz[i];
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
      lastActivityTime = millis();
    }
    if(g_y>300)
      is_data_writing=false;


    // Read and process the incoming data if needed
    if(is_data_writing)
    {
      ax.push_back(a_x);
      ay.push_back(a_y);
      az.push_back(a_z);
      gx.push_back(g_x);
      gy.push_back(g_y);
      gz.push_back(g_z);
    }
    // Add your data processing logic here
  }

  if (millis() - lastActivityTime > INACTIVITY_TIMEOUT) {
    Serial.println("Likh le kuch behen ke laude!!");
    make_final_array(ax, ay, az, gx, gy, gz, concatenated);
    Serial.println("Size of data collected: " + String(ax.size()) )    // PASS final array to model
    // PASS make_final_array to model
    // Process output - USE MODEL AND PREDICTION HERE AND THEN OUTPUT FINAL ANSWER
    Serial.println("Inactivity timeout reached!");
    
    lastActivityTime = millis();
  }
}
}