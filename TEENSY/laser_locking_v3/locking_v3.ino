#include <SPI.h>
#include <cmath>
#include <Arduino.h>
#include <ADC.h>
#include <string.h>
#include <math.h>
#include <IntervalTimer.h>

#define Tsample 30 //sample time for timer in microseconds
#define pi 3.14159
/* Maximum resolution */
#define MAX_RESOLUTION 20
/* Register Map */
#define AD5791_NOP 0 // No operation (NOP).
#define AD5791_REG_DAC 1 // DAC register.
#define AD5791_REG_CTRL 2 // Control register.
#define AD5791_REG_CLR_CODE 3 // Clearcode register.
#define AD5791_CMD_WR_SOFT_CTRL 4 // Software control register(Write only).
/* Input Shift Register bit definition. */
#define AD5791_READ (1ul << 23)
#define AD5791_WRITE (0ul << 23)
#define AD5791_ADDR_REG(x) (((unsigned long)(x) & 0x7) << 20)
/* Control Register bit Definition */
#define AD5791_CTRL_LINCOMP(x) (((x) & 0xF) << 6) // Linearity error compensation.
#define AD5791_CTRL_SDODIS (1 << 5) // SDO pin enable/disable control.
#define AD5791_CTRL_BIN2SC (1 << 4) // DAC register coding selection.
#define AD5791_CTRL_DACTRI (1 << 3) // DAC tristate control.
#define AD5791_CTRL_OPGND (1 << 2) // Output ground clamp control.
#define AD5791_CTRL_RBUF (1 << 1) // Output amplifier configuration control.
/* Software Control Register bit definition */
#define AD5791_SOFT_CTRL_RESET (1 << 2) // RESET function.
#define AD5791_SOFT_CTRL_CLR (1 << 1) // CLR function.
#define AD5791_SOFT_CTRL_LDAC (1 << 0) // LDAC function.
/* DAC OUTPUT STATES */
#define AD5791_OUT_NORMAL 0x0
#define AD5791_OUT_CLAMPED_6K 0x1
#define AD5791_OUT_TRISTATE 0x2
#define AD5791_NOP 0 // No operation (NOP).
#define AD5791_REG_DAC 1 // DAC register.
#define AD5791_REG_CTRL 2 // Control register.
#define AD5791_REG_CLR_CODE 3 // Clearcode register.
#define AD5791_CMD_WR_SOFT_CTRL 4 // Software control register(Write only).

IntervalTimer myTimer;       //for triangular wave 
IntervalTimer mySecondTimer; //for PID
//configure the pin for TEENSY to correctly talking to DAC
const int reset1 = A0;
const int clr1   = A1;
const int ldac1  = A2;
const int sync1  = A3;

int reset = reset1;
int clr   = clr1;
int ldac  = ldac1;
int sync  = sync1;
//Copied from the Analog Device github, not sure what it does
//but since my code cannot run without it and it is the company's sdk
//I would keep it
typedef enum {
ID_AD5760,ID_AD5780,ID_AD5781,ID_AD5790,ID_AD5791,} AD5791_type;
 
struct ad5791_chip_info {
unsigned int resolution;
};
static const struct ad5791_chip_info ad5791_chip_info[] = {
[ID_AD5760] = {.resolution = 16,},
[ID_AD5780] = {.resolution = 18,},
[ID_AD5781] = {.resolution = 18,},
[ID_AD5790] = {.resolution = 20,},
[ID_AD5791] = {.resolution = 20,}
};
AD5791_type act_device;
//initiate the variable for PID
unsigned long lastTime;
volatile int input; 
double Output, Setpoint;
double errSum, lastErr;
double kp, ki, kd;
int init_set; 
int currentDac;
// Create an ADC object
ADC *adc = new ADC(); // ADC object
//0 to 3.3 v
volatile int adcValue = 0;
unsigned long oldCtrl;
int SampleTime = 1000; //1ms 

// Triangle wave parameters
const unsigned long maxDacValue;  
const unsigned long minDacValue;  
int A=2; 
int freq = 10; 
unsigned long currentDacValue ; //this means -1V
int cts; 
int N; 
int increment; 
int slope  = 1;
int scanswitch = 1;
//////////////////////////////////////////////////////////////////////////////////////////////////////
//trigger for the serial port communicationv
volatile int  receivedValue = 0; 
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
long AD5791_SetRegisterValue(unsigned char registerAddress, unsigned long registerValue) {
  unsigned char writeCommand[3] = {0, 0, 0};
  unsigned long spiWord = 0;
  char status = 0;
  spiWord = AD5791_WRITE | AD5791_ADDR_REG(registerAddress) | (registerValue & 0xFFFFF);
  writeCommand[0] = (spiWord >> 16) & 0x0000FF;
  writeCommand[1] = (spiWord >> 8 ) & 0x0000FF;
  writeCommand[2] = (spiWord >> 0 ) & 0x0000FF;
   
  digitalWrite(sync,LOW);
  status = SPI.transfer(writeCommand[0]);
  status = SPI.transfer(writeCommand[1]);
  status = SPI.transfer(writeCommand[2]);
  digitalWrite(sync,HIGH);
 
  return 0;
}
 
long AD5791_GetRegisterValue(unsigned char registerAddress) {
  unsigned char registerWord[3] = {0, 0, 0};
  unsigned long dataRead = 0x0;
  char status = 0;
  registerWord[0] = (AD5791_READ | AD5791_ADDR_REG(registerAddress)) >> 16;
 
  digitalWrite(sync,LOW);
 
  status = SPI.transfer(registerWord[0]);
  status = SPI.transfer(registerWord[1]);
  status = SPI.transfer(registerWord[2]);
  digitalWrite(sync,HIGH);
 
 
  registerWord[0] = 0x00;
  registerWord[1] = 0x00;
  registerWord[2] = 0x00;
    digitalWrite(sync,LOW);
  registerWord[0] = SPI.transfer(0x00);
  registerWord[1] = SPI.transfer(0x00);
  registerWord[2] = SPI.transfer(0x00);
    digitalWrite(sync,HIGH);
  dataRead = ((long)registerWord[0] << 16) |
             ((long)registerWord[1] << 8) |
             ((long)registerWord[2] << 0);
  return dataRead;
}
#include <Arduino.h>  // Ensure the Arduino library is included for Serial and other functionalities

int voltage_to_codespace(double a) {
    // Check if the absolute value of 'a' exceeds 2.0
    if (abs(a) > 2.0) {
        Serial.println("Hitting the safety limit, returning 0");
        double result = (0 + 10) * 524287 * (1.0 / 20.0);;
        return static_cast<int>(round(result));;  // Return 0 if the safety limit is exceeded
    }

    // Calculate the DAC value based on the voltage input
    double result = (a + 10) * 524287 * (1.0 / 20.0);

    // Return the rounded and casted result
    return static_cast<int>(round(result));
}

void adc_start(uint8_t pin) {
  adc->adc0->setAveraging(4); // set number of averages
  adc->adc0->setResolution(12); // set bits of resolution
  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED);
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED);


  adc->adc0->enableInterrupts(adc0_isr); // enable interrupts
  adc->adc0->startContinuous(pin); // start continuous conversion on the pin
}

void handleScanSwitch() {
    switch (scanswitch) {
        case 0:
            myTimer.end();
            delay(100);
            AD5791_SetRegisterValue(AD5791_REG_DAC, voltage_to_codespace(-1.0652008335999998));
            Serial.println("set to -1.06V");
            myTimer.end();  // Ensure the timer is stopped
            //digitalWrite(reset1, LOW);  // Assuming you want to activate this
            break;

        case 1:
            cts = 0; 
            currentDacValue = voltage_to_codespace(-A);
            slope = 1;
            digitalWrite(reset1, HIGH);  // Assuming you want to activate this
            myTimer.begin(updateTriangleWave, 10);  // Start the timer
            Serial.println("Rebegin triangular");
            break;

        default:
            // Handle unexpected cases, potentially with an error message or a safe default
            Serial.println("Error: Invalid scanswitch value");
            break;
    }
}
void SetTunings(double Kp, double Ki, double Kd)
{
  double SampleTimeInSec = ((double)SampleTime)/1000000;
   kp = Kp;
   ki = Ki * SampleTimeInSec;
   kd = Kd / SampleTimeInSec;
}
 


//-----------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
//Low pass butterworth filter order=2

class  FilterBuLp2
{
  public:
    FilterBuLp2()
    {
      v[0]=0.0;
      v[1]=0.0;
      v[2]=0.0;
    }
  private:
    float v[3];
  public:
    float main(float x) //class II 
    {
      v[0] = v[1];
      v[1] = v[2];
      v[2] = (8.870817736483793681e-7 * x)
         + (-0.99733782013962946067 * v[0])
         + (1.99733427181253486715 * v[1]);
      return 
         (v[0] + v[2]) + 2 * v[1];
    }
};

//Low pass filter, order=2 

const float lpf2_freq1 = 100;
const float lpf2_freq2 = 15;

class  LPF2
{
  public:
    LPF2(float corner_freq)
    {
      v[0]=0.0;
      v[1]=0.0;      
      LPF2_al = 0.03 / 213 * corner_freq;
    }
  private:
    float LPF2_al;
  private:
    float v[2];
  public:
    float main(float x) //class II 
    {
      if (v[0] == 0.0 && v[1] == 0.0)   //initialize
      {
        v[0] = x;
        v[1] = x;        
      }
      else
      {
        v[0] = (1 - LPF2_al) * v[0] + LPF2_al * x;
        v[1] = (1 - LPF2_al) * v[1] + LPF2_al * v[0];
      }
      
      return v[1];
    }
};
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long oldCtrl_c = 2097152; //do not edit this, this is for configuring the ADC
FilterBuLp2 LPF;
LPF2 lpf2_1(lpf2_freq1);
LPF2 lpf2_2(lpf2_freq2);


int r = 0; //index used for sine
int Refl =0;
float error = 0;
float errorsum = 0;
float olderror = 0;
float C = 0; //DC correction in 12-bit units


const int samplefreq = 1000000/Tsample; //sampling freq for timer in Hertz

volatile boolean flag = false;
void flagpost(){
  flag = true;
}

float AC_ampl_bits = 0;
float dV = 0;
float Volt=0;
float volt_start;
bool flag_dV_reduced = false;
int monitor_shift = 0;

const int max_bits=524288-1;


double volt_limit_down = 1000;
double volt_limit_up = 50;
float DAC2_offset = 200300;//find the proper voltage by delta source
const double abs_volt_limit_up = 200300; // limits to protect piezo
const double abs_volt_limit_down = 30000;
//PDH LPF 1.28kHz

//works best with 1 kHz ZI LPF
//float P = 1.0;   //0.5
//float I = 0.1;  //0.4
//float D = 0.0; //0.15

//works best with 9 Hz ZI LPF, up to 3 Hz
//float P = 0.5;   //0.5
//float I = 0.1;  //0.4



//float P = 0.1;   //0.5
//float I = 2.0;  //320 -> 250 Hz in ZI LPF min; 32 -> 15 Hz in ZI LPF min
//float D = 0.05; //0.15
// Constants for ADC and DAC limits and setpoints
const double Setpoint = 1452.218; // Corresponding to 1V
const int MaxDACValue = 393215;
const int MinDACValue = 131072;

// PID parameters
const double Kp = 0.1;  // Proportional gain
const double Ki = 0.01; // Integral gain
const double Kd = 0.05; // Derivative gain

// Variables to store state
double lastError = 0;
double errSum = 0;
double currentDACValue = 0;
float setpoint = 0.;
int phi = 8;
//float P = 0.0;   //0.5
//float I = 25.0;  //320 -> 250 Hz in ZI LPF min; 32 -> 15 Hz in ZI LPF min
//float D = 0.0; //0.15

float Refl_max=2000;
float Lock_trigger=1600;


void setup() 
{
  //Prepare the DAC pin
  pinMode(A8, INPUT);
  Serial.begin(38400);
  SPI.begin();
  pinMode(reset1, OUTPUT);
  pinMode(clr1  , OUTPUT);
  pinMode(ldac1 , OUTPUT);
  pinMode(sync1 , OUTPUT);

  digitalWrite(ldac1,LOW);
  digitalWrite(reset1,HIGH);
  digitalWrite(clr1,HIGH);
  digitalWrite(sync1,HIGH);

  SPI.beginTransaction(SPISettings(3800000, MSBFIRST, SPI_MODE1));
  //configure the status for the DAC board
  long status = AD5791_GetRegisterValue(AD5791_REG_CTRL);  
  unsigned long oldCtrl = status;
  oldCtrl = oldCtrl & ~(AD5791_CTRL_LINCOMP(-1) | AD5791_CTRL_SDODIS | AD5791_CTRL_BIN2SC | AD5791_CTRL_RBUF | AD5791_CTRL_OPGND);
  status = AD5791_SetRegisterValue(AD5791_REG_CTRL, oldCtrl);  
  status = AD5791_GetRegisterValue(AD5791_REG_CTRL);

  //Initialized the ADC module 
  adc_start(A7); //I am using A7, pin 21

  //Prepare the triangular wave
  // Configure the IntervalTimer
  myTimer.begin(updateTriangleWave, 10); //Enter the interrupt every 10 us 

  //configure the triangular scanning mode 
  N = 100000/(4*freq);
  increment = (voltage_to_codespace(A)-voltage_to_codespace(-A))/N;
  cts = 0; 
  currentDacValue = voltage_to_codespace(-A);
  Serial.println("Initializied");  
  Serial.println(N);

 

}

void loop() {


}

// Use IntervalTimer to update DAC value at regular intervals
void updateTriangleWave() {
    if (cts < N) {
        if (slope) {
            currentDacValue += increment;
            AD5791_SetRegisterValue(AD5791_REG_DAC, currentDacValue);
            //Serial.println(currentDacValue);
            cts++;
        } else {
            currentDacValue -= increment;
            AD5791_SetRegisterValue(AD5791_REG_DAC, currentDacValue);
            //Serial.println(currentDacValue);
            cts++;
        }
    } else {
        slope = !slope;  // Toggle the slope direction
        cts = 0;  
    }
}

//Use interrupt to initiate the communication 
void serialEvent() {
  while (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command.startsWith("Set:")) {
      float x = command.substring(4).toFloat();
      Serial.print("Setting DAC to: ");
      currentDacValue = voltage_to_codespace(x);
      Serial.println(currentDacValue);
      myTimer.end();
      delay(100);
      AD5791_SetRegisterValue(AD5791_REG_DAC, currentDacValue);
      myTimer.end();  
    } 
    if  (command.startsWith("Toggle:")){
      int temp = command.substring(7).toInt();
      scanswitch = temp;
      handleScanSwitch(); 
    }
    if (command.startsWith("PID:")){
      myTimer.end(); 
      SetTunings(1,0,0);
      mySecondTimer.begin(PID, 1000);
    }
  }
}

//Use interrupt to read the ADC
void adc0_isr() { //A7, Pin21
  adcValue = adc->adc0->readSingle(); // Read the ADC value
  //Serial.print("ADC Value: ");
  Serial.println(adcValue);
  adc->adc0->startContinuous(A7); // Restart continuous conversion
} 

void PID() {
    double input = readADC();
    computeAndApplyPID(input);
}

double readADC() {
    double input = adc->adc0->readSingle();
    Serial.print("Input: ");
    Serial.println(input);
    return input;
}

void computeAndApplyPID(double input) {
    double error = -1 * (input - Setpoint);
    Serial.print("Error: ");
    Serial.println(error);
    errSum += error;
    double de = error - lastError;
    double output = Kp * error + Ki * errSum + Kd * de;
    lastError = error;

    currentDACValue += output;
    updateDAC(currentDACValue);
}

void updateDAC(double value) {
    Serial.print("Current DAC Value: ");
    Serial.println(value);
    if (value > MinDACValue && value < MaxDACValue) {
        AD5791_SetRegisterValue(AD5791_REG_DAC, static_cast<int>(value));
        Serial.println("DAC Value within safety zone");
    } else {
        Serial.println("Hit the limit");
    }
}