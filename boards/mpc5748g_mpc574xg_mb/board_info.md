# Information

The MPC574XG-MB is an evaluation system supporting the NXP MPC574xC/G MCU family, which are used in a range of central body control modules, gateway modules and industrial networking and control applications.

## Board 
MPC574XG-324DS (324 BGA daughterboard) includes:
  Daughterboard with 324 BGA socket
  1 sample of the MPC5748G in the 324 BGA package

## MCU
MPC574xB-C-D-G Family features:
* 2 x Power Architecture e200z4 dual issue 32-bit cores
  * Up to 160 MHz operation
  * Single precision floating point operation
  * 8 KB instruction cache and 4 KB data cache
  * Variable length encoding (VLE) for significant code density improvement
* 1 x Power Architecture e200z2 single issue 32-bit core
  * Up to 80 MHz operation
  * Variable length encoding
    
* 6 MB Flash memory with error correction coding (ECC)
  * Three port Flash controller
  * 768 KB of SRAM with ECC, Three RAM ports

### Processor     

* Cores: 3                                                                     
  * E200Z420 Main Core 0: Max freq. 160MHz, 8KB i-cache, 4KB d-cache           
  * E200Z420 Main Core 1: Max freq. 160MHz, 8KB i-cache, 4KB d-cache           
  * E200Z210 Peripheral Core 2: Max freq: 80Mhz

## Documentation

* Eval board:
  * http://cache.nxp.com/files/microcontrollers/doc/user_guide/MPC5748GEVBUG.pdf?fpsp=1&WT_TYPE=Users%20Guides&WT_VENDOR=FREESCALE&WT_FILE_FORMAT=pdf&WT_ASSET=Documentation&fileExt=.pdf
    
* MPC5748g:
  * http://cache.nxp.com/files/32bit/doc/data_sheet/MPC5748G.pdf?fpsp=1&WT_TYPE=Data%20Sheets&WT_VENDOR=FREESCALE&WT_FILE_FORMAT=pdf&WT_ASSET=Documentation&fileExt=.pdf
  *  http://cache.nxp.com/files/32bit/doc/ref_manual/MPC5748GRM.pdf?fpsp=1&WT_TYPE=Reference%20Manuals&WT_VENDOR=FREESCALE&WT_FILE_FORMAT=pdf&WT_ASSET=Documentation&fileExt=.pdf
      
Supported compilers:
  GHS
  GCC
  
Memory Map: See memory.ldf   

# Testing

## Icu

7 input channels to the ICU (only 6 configured in Port)

The MPC5748GG have 3 eMIOS.

| eMIOS | Channels |
| ----- | -------- |
| 0     | 0 - 31   |
| 1     | 32 - 63  |
| 2     | 64 - 95  |

Signal to channel mapping example:

| Signal           | eMIOS | ICU Channel |
| ---------------- | ----- | ----------- |
| EMIOS0_E0UC_1_G  | 0     | 1           |
| EMIOS1_E1UC_12_H | 1     | 12+32 = 44  |
| EMIOS1_E1UC_15_H | 2     | 15+64 = 79  |
  
 	 
```C
EMIOS0_E0UC_1_G  I 173
EMIOS0_E0UC_4_G  I 4
EMIOS1_E1UC_12_H I 99
EMIOS1_E1UC_14_H I 101
EMIOS1_E1UC_15_H I 102
EMIOS2_E2UC_13_Y I 168
```

6 output channels configured as DIO.

```C
#define DIO_CHNL_EDGE_DETECTION_TEST        DioConf_DioChannel_DioChannel_161
#define DIO_CHNL_TIME_SAMPLING_TEST         DioConf_DioChannel_DioChannel_162
#define DIO_CHNL_GET_TIME_ELAPSED_TEST_1    DioConf_DioChannel_DioChannel_164
#define DIO_CHNL_GET_TIME_ELAPSED_TEST_2    DioConf_DioChannel_DioChannel_72
#define DIO_CHNL_GET_TIME_ELAPSED_TEST_3    DioConf_DioChannel_DioChannel_146
#define DIO_CHNL_DUTY_CYCLE_TEST            DioConf_DioChannel_DioChannel_163
```

```C
#define ICU_TEST_CHANNEL_1 					IcuConf_IcuChannel_EMIOS2_Ch13_EdgeDetect
#define ICU_TEST_CHANNEL_2 					IcuConf_IcuChannel_EMIOS2_Ch13_EdgeCount
#define ICU_TEST_CHANNEL_3 					IcuConf_IcuChannel_EMIOS0_Ch1_TimeStamp
#define ICU_TEST_CHANNEL_4 					IcuConf_IcuChannel_EMIOS1_Ch14_SignalMeasurement_High
#define ICU_TEST_CHANNEL_5 					IcuConf_IcuChannel_EMIOS1_Ch15_SignalMeasurement_Low
#define ICU_TEST_CHANNEL_6 					IcuConf_IcuChannel_EMIOS0_Ch4_SignalMeasurement_Period
#define ICU_TEST_CHANNEL_7 					IcuConf_IcuChannel_EMIOS1_Ch12_SignalMeasurement_DutyCycle
```


DioConf_DioChannel_DioChannel_163 -> ICU_TEST_CHANNEL_7 -> IcuConf_IcuChannel_EMIOS1_Ch12_SignalMeasurement_DutyCycle


## Adc

Two ADCs:
* ADC_0, 10-bit
* ADC_1, 12-bit

Channels 

| Source         | Index | Ref    |
| -------------- | ----- | ------ |
| ADC0_S[8] (8)  | 0     | GND    |
| ADC0_S[2] (7)  | 1     | Vref   |
| ADC0_S[5] (53) | 2     | Vref/2 |
|                | rest  | != 0   |

...and then you have the mapping of channels to source input ( see Table 32-4 in TRM )

| Channel | ADC_0      | ADC_1     |
| ------- | ---------- | --------- |
| 0       | ADC0_S[43] | --        |
| 1       | ADC0_S[42] | --        |
| ..      | ..         | ..        |
| 7       | ADC0_S[2]  | --        |
| 8       | ADC0_S[8]  | --        |
| ..      | ..         | ..        |
| 10      | ?          | ADC1_P[1] |
| 11      | ?          | ADC1_P[2] |
| 12      | ?          | ADC1_P[3] |
| ..      | ..         | ..        |
| 40      | ADC0_S[18] | ADC1_S[6] |
| 41      | ADC0_S[17] | ADC1_S[7] |
| ..      | ..         | ..        |
| 53      | ADC0_S[5]  | ?         |





## Connections


### Experiment Area
 

| MCU | Pin | Mod | I/O | Signal    | Test Card | Comment                                 |
| --- | --- | --- | --- | --------- | --------- | --------------------------------------- |
| A1  | 1   | DIO | I   | GPIO[1]   | ICU2      |                                         |
| A4  | 4   | ICU | I   | E0UC_4_G  | ICU3      | Connected to p24.23/PJ2                 |
| B5  | 21  | ADC | I   | ADC1_P[1] | AD2/Vr_2  | Note! no GND here, test just check != 0 |
| B6  | 22  | ADC | I   | ADC1_P[2] | AD2/Vref  |                                         |
| B7  | 22  | ADC | I   | ADC1_P[3] | AD2/Vr_2  |                                         |
| G6  | 102 | ICU | I   | E1UC_15_H |           | Connected to p24.35/PE8/72*             |
| G7  | 103 | PWM | O   | E1UC_16_X | ICU1      |                                         |
| J13 | 157 |     |     |           |           | Not in port config                      |
| J14 | 158 |     |     |           |           | Not in port config                      |
| K2  | 162 | DIO | O   | GPIO[162] |           | Connected to K13*                       |
| K3  | 163 | DIO | O   | GPIO[163] |           | Connected to p25.13/PG3/99              |
| K4  | 164 | DIO | O   | GPIO[164] |           | Connected to p25.15/PG4/100             |
| K8  | 168 | ICU | I   | E2UC_13_Y |           |                                         |
| K13 | 173 | ICU | I   | E0UC_1_G  |           | Connected to K2*                        |

*) ICU channel not going to TC

### P24
 

| P24/MCU      | Pin | Mod | I/O | Signal    | Test Card | Comment                    |
| ------------ | --- | --- | --- | --------- | --------- | -------------------------- |
| p24.1/3V3_SR |     |     |     |           | GND       |                            |
| p24.2/GND    |     |     |     |           | 3V3_EVB   |                            |
| p24.13/PB10  | 26  | ADC | I   | ADC0_S[2] | AD1/Vref  |                            |
| p24.15/PF0   | 80  | ADC | I   | ADC0_S[8] | AD1/GND   |                            |
| p24.17/PD13  | 61  | ADC | I   | ADC0_S[5] | AD1/Vr_2  |                            |
| p24.23/PJ2   | 146 | DIO | O   | GPIO[146] | ICU3      |                            |
| p24.35/PE8   | 72  | DIO | O   | GPIO[72]  |           | Connected to G6(exp area)* |
| p24.39/PJ1   | 145 | SPI |     |           |           |                            |
| p24.41/PJ0   | 144 | DIO |     | GPIO[144] |           |                            |
| p24.47/PA5   | 5   |     |     |           | ICU2      | Not in port config         |
|              |     |     |     |           |           |                            |


*) ICU channel not going to TC


### P25

| P25/MCU    | Pin | Mod | I/O | Signal    | Test Card | Comment               |
| ---------- | --- | --- | --- | --------- | --------- | --------------------- |
| p25.3/PA12 | 12  | SPI | I   | dSIN_0    | SPI1/MISO |                       |
| p25.5/PA13 | 13  | SPI | O   | dSOUT_0   | SPI1/MOSI |                       |
| p25.7/PA14 | 14  | SPI | O   | dSCLK_0   | SPI1/CLK  |                       |
| p25.9/PA15 | 15  | SPI | O   | dCS0_0    |           | No connection on card |
| p25.11/PG2 | 98  | X   |     |           | ICU1      |                       |
| p25.13/PG3 | 99  | ICU |     |           |           | Connected to K3       |
| p25.15/PG4 | 100 | X   |     |           |           | Connected to K4.      |
| p25.17/PG5 | 101 | ICU | I   | E1UC_14_H |           |                       |
| p25.19/NC  | NC  |     |     |           |           |                       |
|            |     |     |     |           |           |                       |


X - not in port config

