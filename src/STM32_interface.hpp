//------------------------------------------------------------------------------
//   TumanakoVC - Electric Vehicle and Motor control software
//   Copyright (C) 2009 Philip Court <philip@greenstage.co.nz>
//
//   This file is part of TumanakoVC.
//
//   TumanakoVC is free software: you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published
//   by the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   TumanakoVC is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU Lesser General Public License for more details.
//
//   You should have received a copy of the GNU Lesser General Public License
//   along with TumanakoVC.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

/*******************************************************************************
 * File Name          : STM32_interface.h
 * Author             : Philip Court
 * Date First Issued  : 11/Aug/2009
 * Description        : Motor Control main program interface.  This is a generic 
*                       wrapper that can wrap any motor control lib.
*                       TODO change file name.
 *******************************************************************************
 * History:
 * 11/Aug/09 v1.0 - PCC: First Cut
 *  7/Dec/09 v1.1 - PCC: Prepared for first release.
 * 15/Mar/11 v1.2 - PCC: First release of Sine Motor Control combined with VC
 ******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32_INTERFACE_HPP_INCLUDED
#define STM32_INTERFACE_HPP_INCLUDED

#define USE_FILTER

/* Includes ------------------------------------------------------------------*/
#ifdef TUMANAKO_USE_FILTER
#include "filter/filter.hpp"
#endif

extern "C"
{
  #include <libopencm3/cm3/common.h>  //u8 etc
}

#define u32 uint32_t
#define u8 uint8_t

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
typedef enum {Input_IGN, Input_START, Input_CRAWL, Input_FWD, Input_REV} Input_T;
typedef enum {MotorParamTest_OK, MotorParamTest_PWR_STG_OVERHEAT, MotorParamTest_TUMANAKO_MAX_BUS_V, MotorParamTest_UNDERVOLTAGE, MotorParamTest_BRK_HIGH} MotorParamTest_T;

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
//Generic timer class with 1ms ticks
class MyTimer {
  public:
  MyTimer();
  void reset();
  u32 getElapsed(); 

  private:
    u32 myTimeBase_;  //used to record start point of TimeBase_
};

class STM32Interface {
public:
  STM32Interface();

  void sysTickInit();

  void systemReset();
  unsigned long getMillisecTimer();
  void resetMillisecTimer();

  //returns the scaled bus voltage direct from a single digital conversion
  unsigned short getRawScaledBusVolt();
    
  //returns the digital bus voltage A/D conversion without SI unit scaling
  unsigned short getRawBusVolt();

  //Provide access to various motor control variables
  unsigned short getPhaseAOffset();
  unsigned short getPhaseBOffset();
  short getPhase1();
  short getPhase2();
  short getPhase3();
  unsigned long getRotorTimeConstant();
  void setRotorTimeConstant(unsigned long);
  long getSlipFreq();
  short getFluxAngle();
  short getElectricalAngle();
  long getInstantaniousCurrent();
  
  bool getWatchdogTimout();  //used by logging to determine if watchdog has fired

  /**IO from the vehcile loom*/

  //Physical INPUT
  bool getBrakeOn(void); //Brake pedal pushed?
  bool getEnableRegen(void); //Is Regen mode turned on?
  bool getIGN(void);  //Ignition switch on or off?
  bool getRAWStart(void);  //Raw one off read of Start button digital line (is it engaged or noise though?)
  bool getStart(void);  //Filtered Start button engaged? (same as above, except through a software noise filter)
  bool getRAWCrawl(void);  //One off read, Crawl switch on or off?
  bool getCrawl(void);  //Filtered Crawl switch on or off?
  bool getFWD(void);   //Forward selected?
  bool getREV(void);  //Reverse selected?

  bool getNET(void)  //Netural (neither FWD or REV selected)?
  {
    return (!getREV() && !getFWD());
  }

  bool getContactorsEngaged(void);  //Are the contactors engaged (Physical test)?
  bool getEmergencyStop(void);  //Has the emergency stop been activated?

  //Physical OUTPUT
  void setErrorLED(bool value); //Show red error light
  void setRunLED(bool value); //Show green run light
  void setKiwiACRedLED(bool value); //Show red light on kiwiAC board
  void setKiwiACBlueLED(bool value); //Show blue light on kiwiAC board

  // Reads the value of the specified ADC channel
  unsigned short readADC(unsigned char channel);  //:TODO: needs to be encapsulated into +ve torque and -ve torque etc

  /** Used to initialise the Motor Control libraries*/
  int init(void);

  // Getter/Setters for the current torque setting (Iq)
  signed short getTorque(void); //read system feedback value for Torque
  signed short getTorqueVq(void);  //PID Vq output
  void setTorque(signed short); //Set the target Torque value (used in torque control algorithm)

  // Getter/Setters for the current speed
  signed short getSpeed(void); //read current value of Speed
  void setSpeed(signed short); //Set the target Speed value (not implemented yet)


  // Getter/Setters for the current flux setting (Id)
  signed short getFlux(void); //read system feedback value for flux
  signed short getFluxVd(void);  //PID Vd output
  void setFlux(signed short); //Set the target rotor Flux value (used in torque control algorithm)

  // Getter/Setters for the current speed (RPM) setting
  signed short getRPM(void);  //read current value of motor RPM
  void setRPM(signed short);  //Set the target RPM (speed control loop only - not currently supported by Tumanako)

  //Is the bus voltage OK? (TODO - more work needed here.  Must expose limits etc...)
  bool busVoltageOK(void);

  //Get current bus voltage (via a historical 16 reading average)
  short busVoltage(void);

  //Get current temperature from power stage (via a running average)
  short powerStageTemperature(void);
  
  //Get current temperature from motor (returns most recent reading from previous instantanious value calculated once every secound.  TODO compute running average).
  short motorTemperature(void);
  
  //for debugging temperature calibration
  short motorTempFreq(void);

  //returns 0 if no issues (1 = Powerstage overheat, 2 = Bus over voltage, 3 = Bus undervoltage. TODO needs enum)
  short testVariousMotorParam(void);

  //Contactor controls (See diagram here: http://liionbms.com/php/precharge.php)
  bool getK1();  //reads K1 physical contactor feedback
  bool getK2();  //reads K2 physical contactor feedback
  bool getK3();  //reads K3 physical contactor feedback
  void setK1(bool status);
  void setK2(bool status);
  void setK3(bool status);

  /**
  * Returns true if the contactors are safely engaged (according to digital feedback)
  */
  bool getContactorsInRunStateConfiguration()
  {
    if ((getK1() == false) && (getK2() == true) && (getK3() == true))
      return true;
    else
      return false;
  }
  
  //Blocking wait. Waits specified number of milliseconds (ms)
  void wait(unsigned short time);

  //Sanity checks (Same as testVariousMotorParam, except it takes action internally without interaction with vehicle control layer. Updates motor control internal state machine if fault detected)
  void checkPowerStageLimits();

  //Prep motor for start (init PID loop, encoder buffer, IFOC algorithm variables, Enable PWM outputs and calibrate 3 phase current sensors)
  void motorInit();

  //Start motor (startup checks and switch motor control state machine to GO!)
  void motorStart();

  //test to be executed in the main loop when motor is running (checks against motor hardware limits - Max RPM)
  void motorTestForSpeedError();

  //Shutdown motor control and power stage (disable PWM, disconnect contactors, zero IFOC outputs and shutdown motor controller state machine)
  void shutdownPower();
  
private:
  //Prepare Timer 3 to measure motor temp (KiwiAC STM32MCU board specific - converts KTY84 resistance to freq)
  void motorTempSensorInit();

  //TODO tidy these methods away
  void adc_setup(u32 adc_port);
  u8 adcchfromport(int command_port, int command_bit);

};
#endif // STM32_INTERFACE_HPP_INCLUDED
