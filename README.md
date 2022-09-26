ACS712 current measurement
==========================

Read and measure AC or DC current usage over a given time. Will give as output the total
current usage in ampere/hour. If test lasts less than an hour the output measurement
will always be in Ah, for e.g. 5.5Ah for 0.5 hours.

Uses alpha-beta smoothing algorithm on data read from the ACS712 to eliminate peaks. And also 
applies an error correction factor, from 0.93 to 0.985, to fix readings. You might not need this
last set of fixings, I added them based on my measurement of known load.

ACS712-30 (front view)
   1 2 3 
   + D -
   
PINOUT (Using A4 pin on Arduino)
   Arduino A4 <-> ACS712 PIN2


In the code the ACS712-30 (66mV/A) is used, if you use a 5A (185mV/A) or 20A (100mV/a) version you can change the 0.66f value in the line:
```
float eCurrent = ((mV - 2500.0f) / 0.66f); // 0.66f for 30A module; 0.1f for 20A, 0.185f for 5A
```
