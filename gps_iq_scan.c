/////////////////////////////////////////////////////////////////
// Scan 3ms of 16-bit I/Q data for GPS signals.
//
// Very inefficient, but just a proof of concept.
//
// (c) 2020 Mike Field
/////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>

#define SAMPLE_RATE (10*1000*1000)
#define IF_HZ       ( 2*1000*1000)

#define BAND_HZ 500
#define N_BANDS  50
#define N_SV     30

#define SAMPLES_PER_MS  (SAMPLE_RATE/1000)
#define SAMPLES_PER_2MS (SAMPLE_RATE/1000*2)
#define SAMPLES_PER_3MS (SAMPLE_RATE/1000*3)

struct sample {
   int16_t i;
   int16_t q;
};
struct sample samples[3*SAMPLE_RATE/1000]; // Enough for 3ms samples
struct sample_d {
   double i;
   double q;
};
struct sample baseband[3*SAMPLE_RATE/1000]; // Enough for 3ms samples

struct Space_vehicle {
   uint8_t sv_id;
   uint8_t tap1;
   uint8_t tap2;
} space_vehicles[] = {
  { 1,  2, 6},
  { 2,  3, 7},
  { 3,  4, 8},
  { 4,  5, 9},
  { 5,  1, 9},
  { 6,  2,10},
  { 7,  1, 8},
  { 8,  2, 9},
  { 9,  3,10},
  {10,  2, 3},
  {11,  3, 4},
  {12,  5, 6},
  {13,  6, 7},
  {14,  7, 8},
  {15,  8, 9},
  {16,  9,10},
  {17,  1, 4},
  {18,  2, 5},
  {19,  3, 6},
  {20,  4, 7},
  {21,  5, 8},
  {22,  6, 9},
  {23,  1, 3},
  {24,  4, 6},
  {25,  5, 7},
  {26,  6, 8},
  {27,  7, 9},
  {28,  8,10},
  {29,  1, 6},
  {30,  2, 7},
  {31,  3, 8},
  {32,  4, 9}
};

uint8_t gold_codes[N_SV][1023];
uint8_t gold_codes_stretched_2ms[N_SV][1023*SAMPLES_PER_2MS];

/**********************************************************************
* Generate the G1 LFSR bit stream
**********************************************************************/
static void g1_lfsr(unsigned char *out) {
  int state = 0x3FF,i;
  for(i = 0; i < 1023; i++) {
    int new_bit;
    out[i]   = (state >>9) & 0x1;
    /* Update the G1 LFSR */
    new_bit = ((state >>9) ^ (state >>2))&1;
    state   = ((state << 1) | new_bit) & 0x3FF;
  }
}

/**********************************************************************
* Generate the G2 LFSR bit stream. Different satellites have different
* taps, which effectively alters the relative phase of G1 vs G2 codes
**********************************************************************/
static void g2_lfsr(unsigned char tap0, unsigned char tap1, unsigned char *out) {
  int state = 0x3FF,i;
  /* Adjust tap number from 1-10 to 0-9 */
  tap0--;
  tap1--;
  for(i = 0; i < 1023; i++) {
    int new_bit;

    out[i] = ((state >> tap0) ^ (state >> tap1)) & 0x1;

    /* Update the G2 LFSR  */
    new_bit = ((state >>9) ^ (state >>8) ^
               (state >>7) ^ (state >>5) ^
               (state >>2) ^ (state >>1))&1;
    state = ((state << 1) | new_bit) & 0x3FF;
  }
}

/**********************************************************************
* Combine the G1 and G2 codes to make each satellites code
**********************************************************************/
static void combine_g1_and_g2(unsigned char *g1, unsigned char *g2, unsigned char *out)
{
  int i;
  for(i = 0; i < 1023; i++ ) {
    out[i] = g1[i] ^ g2[i];
  }
}

/*********************************************************************
* Build the Gold codes for each Satellite from the G1 and G2 streams
*********************************************************************/
static void generate_gold_codes(void) {
  int sv;
  static unsigned char g1[1023];
  static unsigned char g2[1023];
  g1_lfsr(g1);
  for(sv = 0; sv < N_SV; sv++) {
    g2_lfsr(space_vehicles[sv].tap1, space_vehicles[sv].tap2, g2);
    combine_g1_and_g2(g1, g2, gold_codes[sv]);
  }
}

static void stretch_gold_codes(void) {
  for(int sv = 0; sv < N_SV; sv++) {
    for(int i = 0; i < SAMPLES_PER_2MS; i++) {
//    int j = (1023*(long)i/(SAMPLES_PER_2MS/2))%1023;
      int j = (1023*i/SAMPLES_PER_MS)%1023;
      gold_codes_stretched_2ms[sv][i] = gold_codes[sv][j];
    }
    sleep(1);
  }
}

int read_samples(char *fname) {
   int nRead = 0;
   int toRead =  sizeof(samples)/sizeof(struct sample);
   FILE *f;
   f = fopen(fname, "rb");
   if(f == NULL) {
      fprintf(stderr,"Unable to open file '%s'\n", fname);
      return 0;
   } 
   printf("File opened\n");
   while(nRead != toRead) {
     int l,h;

     l = getc(f);
     if(l == EOF) break;
     h = getc(f);
     if(h == EOF) break;
     samples[nRead].i = (int16_t)((h<<8)|l);

     l = getc(f);
     if(l == EOF) break;
     h = getc(f);
     if(h == EOF) break;
     samples[nRead].q = (int16_t)((h<<8)|l);
     nRead++;
   } 
   fclose(f);
   if(nRead != sizeof(samples)/sizeof(struct sample)) {
      fprintf(stderr,"Unable to read enough samples\n");
      return 0;
   }
   return 1;
}

double search(int sv,int offset) {
   double total_i = 0.0;
   double total_q = 0.0;
   for(int i = 0; i < SAMPLES_PER_2MS; i++) {
      if(gold_codes_stretched_2ms[sv][i]) {
         total_i += baseband[offset+i].i;
         total_q += baseband[offset+i].q;
      } else {
         total_i -= baseband[offset+i].i;
         total_q -= baseband[offset+i].q;
      }
   }
   total_i /= SAMPLES_PER_2MS; 
   total_q /= SAMPLES_PER_2MS; 
   return total_i*total_i+total_q*total_q;
}

void move_to_baseband(int frequency) {
   // Process 3ms of samples
   for(int i = 0; i < SAMPLE_RATE/1000*3; i++) {
      double phase = -((double)i)*frequency/SAMPLE_RATE;
      double c = cos(phase*2*M_PI);
      double s = sin(phase*2*M_PI);
      baseband[i].i = c*samples[i].i - s*samples[i].q; 
      baseband[i].q = s*samples[i].i + c*samples[i].q; 
   }
}

void processSamples(void) {
   printf("Processing\n");
   printf("SVID, Frequency, offset,   Strength\n");
   // Search for each space vehicle
   for(int sv = 0; sv < N_SV; sv++) {
      double max_power  = 0;
      int max_offset = 0;
      int max_freq = 0;

      // At each IF band
     // for(int freq = IF_HZ; freq <= IF_HZ; freq += BAND_HZ) {
      for(int freq = IF_HZ-BAND_HZ*(N_BANDS/2); freq <= IF_HZ+BAND_HZ*(N_BANDS/2); freq += BAND_HZ) {
         move_to_baseband(freq);

         // At each alignment
         for(int offset = 0; offset < SAMPLES_PER_MS; offset += SAMPLE_RATE/3000000) {
            double power = search(sv, offset);
            if(power > max_power) {
               max_power  = power;
               max_offset = offset;
               max_freq   = freq;
            }
         }
      }
      printf("SV%02i, %9i, %6i, %10.1f\n", space_vehicles[sv].sv_id, max_freq, max_offset, max_power);
   } 
}

int main(int argc, char *argv[]) {
   if(argc != 2) {
      printf("Need sample file\n");
      return 1;
   }

   printf("Building Gold codes\n");
   generate_gold_codes();
   printf("Stretching Gold codes\n");
   stretch_gold_codes();
   printf("Reading samples\n");

   if(read_samples(argv[1])) {
     printf("Samples read\n");
     processSamples();
   }

   return 0;
}
