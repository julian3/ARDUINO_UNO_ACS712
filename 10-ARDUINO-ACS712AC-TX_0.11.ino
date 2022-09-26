/** 
 * ACS712 current measurement
 * 
 * Read and measure current usage over a given time. Will give as output the total
 * current usage in ampere/hour. If test lastsless than an hour the output measurement
 * will always be in Ah, for e.g. 5.5Ah for 0.5 hours.
 * 
 * Uses alpha-beta smoothing algorithm on data read from the ACS712 to eliminate peaks. And also 
 * applies an error correction factor, from 0.93 to 0.985, to fix readings. You might not need this
 * last set of fixings, I added them based on my measurement of known load.
 * 
 * ACS712-30 (front view)
 *    1 2 3 
 *    + D -
 *    
 * PINOUT
 *    Arduino A4 <-> ACS712 PIN2
 * 
 * @version 0.1 30-03-2021 
 * @version 0.11 30-08-2022 code cleanup
 * @author Julian E.Spina (jes@acca3.it)
 */

#define ARDUINO_REF 5000
#define ACS712_PIN A4

long testLengthMillis = 10000;  // Set the total time test, for e.g. 60000 = 1 minute, 360000 = 6 minutes
#define TIME_FRAME_MS 1000      // 5000 would be a 5 second frame, read current every 5 seconds

float currentSum = 0.0f;  // To calculate total amps
int iterations = 0;       // To count how many readings we did in the testLengthMillis time

float currentValue = 0.0f, previousValue = 0.0f;  // Variables for alpha-beta smoothing algoritm
long startupMillis = 0;                           // Starting time
long lastReadMillis = 0;                          // To handle each read timing

void setup() {   
    Serial.begin(115200);
    Serial.println("ARDUINO ACS712");    

    Serial.print("Startup ");
  
    // Create a first reading to have valid previous value for the alpha-beta algorithm
    for (int i=0; i<30; i++) {
      makeReading();
      delay(200);
      Serial.print(".");
    }
    Serial.println(""); 
    
    // Show actual settings info 
    Serial.print("Test length: ");
    if (testLengthMillis>60000){
      Serial.print(testLengthMillis/1000/60, 1); // print with 1 decimal
      Serial.println("m"); // minutes
    }
    else{
      Serial.print(testLengthMillis/1000);
      Serial.println("s"); // seconds   
    }

    Serial.print("Sampling frame: ");
    Serial.print(TIME_FRAME_MS);
    Serial.println("ms");
    
    Serial.println("Collecting data");

    delay(1000);
    // Ok, start counter
    startupMillis = millis();
}

void loop() {      

    makeReading(); // Read value from ACS712

    // Lets get elapsed time
    long diff = millis() - lastReadMillis;
    long totalDiff = millis() - startupMillis;

    // Use abs() on a calculated value, don't do arithmetics directly in abs() function
    if (abs(diff)> (TIME_FRAME_MS-50)) { 
        
        Serial.print("Value: ");        
        Serial.print(currentValue, 2);
        Serial.print("A ");
        Serial.print((currentValue*225), 0);
        Serial.print("W Remaining time: ");
        Serial.print((testLengthMillis-totalDiff)/1000);
        Serial.println("s");
         
        // Start calculating power usage 
        iterations++; 
        currentSum += currentValue;

        lastReadMillis = millis();
    }
    
    if (totalDiff>testLengthMillis) { // Code ends when time limit is reached
      
        float averageKwh = currentSum / iterations; // calculation on the TIME_FRAME_MS window
        float timeFrameRatio = 3600 / (testLengthMillis/1000); // how many TIME_FRAME_MS spaces in one hour
        Serial.println("Test finished");
        Serial.print("Total time: ");
        Serial.print(totalDiff/1000);        
        Serial.println("s");
        Serial.print("Current Usage: ");
        Serial.print(averageKwh, 2);
        Serial.print("Ah for a total time of ");
        Serial.print(1/timeFrameRatio, 3);
        Serial.println(" hours");

        while(true);
    }    
} // end loop()

/**
 * Read value from ACS712, apply alfa-beta algorithm and correct final value (optional)
 */
void makeReading() {

      float adcValue = getVpp();
      float mV = adcValue * 4.8828f;              // (adcValue / 1024.0f) * 5000.0f; // mV
      float eCurrent = ((mV - 2500.0f) / 0.66f);  // 2500 = voltageRef/2, 0.66 ACS712-30 granularity 
      float amp = eCurrent / 141.42f;             // eCurrent/(sqrt(2):  sqrt(2) = 1.4142, we divide result by 100 to obtain amps
      currentValue = 0.1f * amp + 0.9f * previousValue; // alfa-beta smoothing formula

      // This is a correction on measured amps. It could not be necessary for you, in case just remove the if..else chain
      if (currentValue<=1.5f)
        currentValue = currentValue * 0.93f;
      if (currentValue>1.5f && currentValue<=2.5f)
        currentValue = currentValue * 0.94;
      else if (currentValue>2.5f && currentValue<=8.0)
        currentValue = currentValue * 0.97f;
      else if (currentValue>8.0f && currentValue<=10.0f)
        currentValue = currentValue * 0.98f;
      else 
        currentValue = currentValue * 0.985f;
  
      if (currentValue < 0.0f) currentValue = 0.0f; // We don't want negative readings
      previousValue = currentValue;
}

/**
 * Makes 100 reads from ACS712 data pin and returns average value
 */
int getVpp()
{
  long total=0;

  int c=100;
  while(c-->0) {       
       int readValue = analogRead(ACS712_PIN);
       total += readValue;
       delay(1);
   }

  int retv = total / 100;   // Lets average 100 readings
 
  if (retv<512) return 512; // We exclude any negative measurements
  
  return retv;
 }
