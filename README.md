# GPS_IQ_Scan
Scan I/Q data for GPS space vehicle signals

Requires 3ms of 16bit I/Q data, stored in this format:
   LSB I , MSB I, LSB Q, MSB Q
  
## Example output

    $ ./gps_iq_scan ../gps2/samples/airspy-8ms-gain-10.bin
    Building Gold codes
    Stretching Gold codes
    Reading samples
    File opened
    Samples read
    Processing
    SVID, Frequency, offset,   Strength
    SV01,   1998500,   5229,       30.6
    SV02,   1987500,   2160,       29.2
    SV03,   1989500,   4722,       28.8
    SV04,   2002500,   8181,       30.7
    SV05,   2002500,   6969,       25.8
    SV06,   2012500,   1815,       33.7
    SV07,   2006500,   1380,       32.7
    SV08,   1997500,   3216,      893.2
    SV09,   1997500,   6558,       33.6
    SV10,   1988500,   6408,       30.3
    SV11,   2012500,   6624,       26.1
    SV12,   2006500,   4269,       29.1
    SV13,   1997500,   8997,       33.3
    SV14,   1999500,   6120,       25.5
    SV15,   2009000,   4926,       30.0
    SV16,   1999500,   8670,       42.1
    SV17,   1990500,   1518,       30.2
    SV18,   1993500,   4065,       28.4
    SV19,   1988500,   5688,       27.8
    SV20,   1991500,   6078,       32.4
    SV21,   2005500,   6675,       28.7
    SV22,   1994500,   2178,       25.0
    SV23,   1994500,    459,       32.7
    SV24,   2007500,   8037,       27.4
    SV25,   2008500,   2121,       25.5
    SV26,   1999500,   3549,       25.5
    SV27,   2004000,   6111,       28.3
    SV28,   2008500,   7593,       25.5
    SV29,   2000500,   2727,       24.7
    SV30,   1996500,   4350,       33.5
