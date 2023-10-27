/*
Versione Arduino dell'esempio Hunter con i2S alposto di un DAC
maurizio.conti@fablabromagna.org
+ delay

nota sul DAC MAX98357A
The digital audio interface accepts specified sample rates between 8kHz
and 96kHz for all supported data formats. The ICs can be configured to produce a
left channel, right channel, or (left/2 + right/2) output from the stereo input data.
The IC operate using 16/24/32-bit data for I2S.
Single-Supply Operation (2.5V to 5.5V)
3.2W Output Power into 4Ω at 5V
*/

// Sandro Grassia
// Arduino IDE rel. 2.1.1
// board "Raspberry Pi Pico"
// n.2 DAC MAX98357A: The output is a ~300KHz square wave PWM that is then 'averaged out' by the speaker coil.

// ** Dual core **
// Il codice dedica ciascun core all'elaborazione relativa ad uno dei canali: core0 --> Left, core1 --> Right

#include <arduino.h>
#include <I2S.h>

// Istanzia i2S per ADC canale L
I2S i2s_L(OUTPUT);

// collegamento DAC L
// +5V  --> VBUS
// GND  --> GND
// SD non collegato
// GAIN --> VBUS
#define pDOUT_L 22         // DIN  --> GPIO 22 - the DOUT pin, can be any valid GPIO pin - This is the pin that has the actual data coming in, both left and right data are sent on this pin, the LRC pin indicates when left or right is being transmitted.
#define pBCLK_L 20         // BCLK --> GPIO 20 - BCLK (Bit Clock) - This is the pin that tells the amplifier when to read data on the data pin.
#define pWS_L (pBCLK + 1)  // LRC  --> GPIO 21 - the WS (word select) signal, which toggles before the sample for each channel is sent

// Istanzia i2S per ADC canale R
I2S i2s_R(OUTPUT);

// collegamento DAC R
// +5V  --> VBUS
// GND  --> GND
// SD non collegato
// GAIN --> VBUS
#define pDOUT_R 15         // DIN  --> GPIO 15 - the DOUT pin, can be any valid GPIO pin - This is the pin that has the actual data coming in, both left and right data are sent on this pin, the LRC pin indicates when left or right is being transmitted.
#define pBCLK_R 16         // BCLK --> GPIO 16 - BCLK (Bit Clock) - This is the pin that tells the amplifier when to read data on the data pin.
#define pWS_R (pBCLK + 1)  // LRC  --> GPIO 17 - the WS (word select) signal, which toggles before the sample for each channel is sent

/*
** pin SD **

If SD is connected to ground directly (voltage is under 0.16V) then the amp is shut down
If the voltage on SD is between 0.16V and 0.77V then the output is (Left + Right)/2, that is the stereo average. 
If the voltage on SD is between 0.77V and 1.4V then the output is just the Right channel
If the voltage on SD is higher than 1.4V then the output is the Left channel.

SD is compounded by an internal 100K pulldown resistor on SD so you need to use a pullup resistor on SD to balance out the 100K internal pulldown.
For the breakout board, there's a 1Mohm resistor from SD to Vin which, when powering from 5V will give you the 'stereo average' output.
If you want left or right channel only, or if you are powering from non-5V power, you may need to experiment with different resistors
to get the desired voltage on SD

** GAIN **
You can have a gain of 3dB, 6dB, 9dB, 12dB or 15dB.

    15dB if a 100K resistor is connected between GAIN and GND
    12dB if GAIN is connected directly to GND
    9dB if GAIN is not connected to anything (this is the default)
    6dB if GAIN is connected directly to Vin
    3dB if a 100K resistor is connected between GAIN and Vin
*/

#define Fs 40000  // coerente con il timing di questo codice
// Istanzio due timer
struct repeating_timer timer_L;
struct repeating_timer timer_R;

// Ingresso audio
int in_PIN_L = A0;  // 0 --> 3.3V ; Raspberry Pi Pico has a 12-bit resolution GP26 (ADC0, pin 31), GP27 (ADC1, pin 32), GP28 (ADC2, pin 34); the digital value will be between 0-4095 (12bit)
uint16_t in_value_L = 0;
int in_PIN_R = A1;  // 0 --> 3.3V ; Raspberry Pi Pico has a 12-bit resolution GP26 (ADC0, pin 31), GP27 (ADC1, pin 32), GP28 (ADC2, pin 34); the digital value will be between 0-4095 (12bit)
uint16_t in_value_R = 0;

// La coppia di delay (L, R) e' realizzata con due array, della stesssa dimensione, utilizzati come code FIFO per salvare i campioni audio a 16bit
#define D_fifo_dim 14000  // 44100 --> 1 sec

// coda fifo L
uint16_t D_fifo_L[D_fifo_dim] = { 0x00 };  // array usato come FIFO per il delay
int D_read_sample_L = 0;
int D_write_sample_L = 0;
int D_rw_value_L = 12000;  // ritardo (in samples) tra scrittura e lettura

// coda fifo R
uint16_t D_fifo_R[D_fifo_dim] = { 0x00 };  // array usato come FIFO per il delay
int D_read_sample_R = 0;
int D_write_sample_R = 0;
int D_rw_value_R = 8000;  // ritardo (in samples) tra scrittura e lettura

// variabili per performance check
uint32_t T0;
uint32_t T1;
uint32_t T;
bool f0 = false;


// ***********************************************************
// ***********  TUTTO SI MATERIALIZZA QUI...  ****************
// ***********************************************************
// codice Hunter Timer ISR
// Ad ogni chiamata va fornito agli ADC L ed R 1 solo campione, quindi il tempo disponibile per ogni ciclo e': 1/40KHz --> 25us

// Procedura per il canale Left
bool repeating_timer_callback_L(struct repeating_timer *t)
{
  //check
  T0 = micros();

  // OUTPUT
  // aggiorna I2S_L ch.0 e ch.1
  i2s_L.write(D_fifo_L[D_write_sample_L]);
  i2s_L.write(D_fifo_L[D_write_sample_L]);

  // INPUT
  // leggi input L
  in_value_L = 16 * analogRead(in_PIN_L);

  // check
  T1 = micros();
  T = T1 - T0;
  digitalWriteFast(2, f0);
  f0 = !f0;

  // *************** delay LEFT  *******************
  // aggiornamento indice scrittura D_fifo
  D_write_sample_L++;
  if (D_write_sample_L == D_fifo_dim)
    D_write_sample_L = 0;

  // aggiornamento indice lettura D_fifo
  D_read_sample_L = D_write_sample_L - D_rw_value_L;
  if (D_read_sample_L < 0)
    D_read_sample_L += D_fifo_dim;

  // somma e scrivi D_fifo
  D_fifo_L[D_write_sample_L] = 0.7 * (in_value_L) + 0.6 * D_fifo_L[D_read_sample_L];

  return true;
}

// Procedura per il canale Right
bool repeating_timer_callback_R(struct repeating_timer *t)
{
  // OUTPUT
  // aggiorna I2S_R ch.0 e ch.1
  i2s_R.write(D_fifo_R[D_write_sample_R]);
  i2s_R.write(D_fifo_R[D_write_sample_R]);

  // INPUT
  // leggi input R
  in_value_R = 16 * analogRead(in_PIN_R);

  // *************** delay RIGHT  *******************
  // aggiornamento indice scrittura D_fifo
  D_write_sample_R++;
  if (D_write_sample_R == D_fifo_dim)
    D_write_sample_R = 0;

  // aggiornamento indice lettura D_fifo
  D_read_sample_R = D_write_sample_R - D_rw_value_R;
  if (D_read_sample_R < 0)
    D_read_sample_R += D_fifo_dim;

  // somma e scrivi D_fifo
  D_fifo_R[D_write_sample_R] = 0.7 * (in_value_R) + 0.6 * D_fifo_R[D_read_sample_R];

  return true;
}

// core0
void setup()
{
  // Setup di i2S_L
  i2s_L.setBCLK(pBCLK_L);
  i2s_L.setDATA(pDOUT_L);
  i2s_L.setBitsPerSample(16);

  if (!i2s_L.begin(Fs))
  {
    Serial.println("Errore nella i2S_L!");
    while (1)
      ;  // fermo qui
  }

  // Pin usato per debug hardware
  pinMode(2, OUTPUT);

  // Hunter: Negative delay so means we will call repeating_timer_callback, and call it again
  // 25us (40kHz) later regardless of how long the callback took to execute
  add_repeating_timer_us(-25, repeating_timer_callback_L, NULL, &timer_L);
}

// core1
void setup1()
{
  // Setup di i2S_R
  i2s_R.setBCLK(pBCLK_R);
  i2s_R.setDATA(pDOUT_R);
  i2s_R.setBitsPerSample(16);

  if (!i2s_R.begin(Fs))
  {
    Serial.println("Errore nella i2S_R!");
    while (1)
      ;  // fermo qui
  }

  // Hunter: Negative delay so means we will call repeating_timer_callback, and call it again
  // 25us (40kHz) later regardless of how long the callback took to execute
  add_repeating_timer_us(-25, repeating_timer_callback_R, NULL, &timer_R);
}

// core0
void loop()
{
  // Qui si esegue con priorità inferiore, nel tempo residuo tra una chiamata e l'altra a repeating_timer_callback() 

  // check
  Serial.println(T);
}

// core1
void loop1()
{

}
