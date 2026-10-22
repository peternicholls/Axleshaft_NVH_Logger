# Timing and synchronisation

Target: 1,000 six-axis samples/s. Stage 0B must exercise real storage buffering with synthetic acquisition and explicitly count missed scheduling opportunities. Define numerical tolerances before accepting results.

The SD reference measured a 58 ms block write; this is not a guaranteed maximum. RAM buffers must be measured under stalls. Later IMU FIFO service must continue independently of storage: 1,024 bytes hold approximately 85 twelve-byte frames, around 85 ms at the target rate.

The MPU-6050 and ESP32 use different clocks. FIFO read time is not sample acquisition time. Stage 3 needs timing anchors, actual sample-period/drift estimation and an explicit mapping of sample indices into the same monotonic domain as optical timestamps. Capture optical timestamps independently of SD and LED operations. Implementation and qualification remain pending.
