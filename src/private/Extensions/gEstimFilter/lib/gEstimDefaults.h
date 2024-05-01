/// \file gEstimDefaults.h
/// \brief Defaults and limits for g.Estim
///
/// This files contains defaults for the device. The API has additional defaults for the different modi set by st_operationmode() (\ref defaults).
/// Variant defaults a in extra files.

// Thomas Kerbl (kerbl@gtec.at)
// 10.12.2019
// Guger Technologies OG
//
// Project: g.Estim, g.Cube SC_06


#ifndef GESTIMDEFAULTS_H
#define GESTIMDEFAULTS_H

//ONE OF THESE DEFINES NEEDS TO BE PROVIDED VIA COMMAND LINE BY THE BUILD SYSTEM
//#define GESTIMPRO  ///< This define is used to compile for the g.EstimPRO
//#define GESTIMFES  ///< This define is used to compile for the g.EstimFES
//#define GCUBE_FES  ///< This define is used to compile for the g.Cube fES PCB configuration
//#define GCUBE_TES  ///< This define is used to compile for the g.Cube tES PCB configuration

#include "gEstimTypes.h"
#ifdef GESTIMPRO
#include "gEstimDefaultsEstimPRO.h"
#elif GESTIMFES
#include "gEstimDefaultsEstimFES.h"
#elif GCUBE_FES
#include "gEstimDefaultsCubeFES.h"
#elif GCUBE_TES
#include "gEstimDefaultsCubeTES.h"
#endif

#define UART_BAUDRATE 500000     ///< UART 1 baudrate definition. A value different from 1000000 must be placed at the end of the FTDI Description field as kBd. E.g. 500kBd.

//unit factors to SI unit
#define UNIT_RESISTANCE 20           ///< the unit factor used for GDevice::resistance_
#define UNIT_CURRENT 1e-5            ///< the unit factor used for GDevice::phase_current1_ and the like
#define UNIT_PULSE_RATE 0.1          ///< the unit factor used for GDevice::pulse_rate_ (GExtra::train_rate_ is in Hz, though)
#define UNIT_DO_AND_FADE_TIME 1e-3   ///< the unit factor used for GDevice::do1_pre_time_ and the like
#define UNIT_TOTAL_CHARGE 1e-11      ///< the unit factor used for GDevice::total_charge_
#define UNIT_VOLTAGE_uC2 0.1         ///< the unit factor used for GDevice::voltage_ and the like
#define UNIT_ELECTRODE_LEN 1e-6      ///< the unit factor used for GExtra::el_circ_diameter_ and the like
#define UNIT_TIME 1e-5               ///< the unit factor used for gt_pulse_duration() and the like
#define UNIT_TES_TRAIN_TIME 0.1      ///< the unit factor used for GDevice::pulses_ in case of tES
#define R_SENSE 150                  ///< Sensing resistor for current measurement in ohm

/// current average in uA
#define calcCurAvg(cur_sum,dur_sum) (dur_sum)?((cur_sum)/dur_sum):0

// Limits stimulation settings
#define G_ES_20PC_BATTERY 120               ///< 20% battery charge
#define G_ES_10PC_BATTERY 110               ///< 10% battery charge
#define G_ES_MAX_BATTERY 200                ///< Maximum battery charge in %

#define G_ES_LOW_RESISTANCE 5               ///< Value below which resistance is considered low, without finer distinction.
#define G_ES_MAX_RESISTANCE 30000           ///< lower limit value for resistance measurement with GOFUNCTION_NONE, applicable for PRO and FES
#define RESISTANCE_MEAS_I   8               ///< auxiliary current during resistance measurement [uA]

#define AUTOSTOP1_TIMOUT_SEC 300            ///< Device goes from GSTATE_ACTIVE to GSTATE_STOP after this time [s]
#define AUTOSTOP2_TIMOUT_SEC 2              ///< Device goes from GSTATE_ACTIVE to GSTATE_STOP after this time [s]

// use like: GDevice device = DEVICE_DEFAULTS;
#define FES_FACTOR 4                        ///< current for FES scaled by this factor with regard to settings

#define DEVICE_DEFAULTS {\
  /* serial_          */ {"\x0"},\
  /* identifier_      */ {"\x0"},\
  /* HW_version_      */ {"\x0"},\
  /* FW_version_      */ {"\x0"},\
  /* coeff_cs1        */ {1000,0},\
  /* coeff_cs2        */ {1000,0},\
  /* coeff_i          */ {1000,0},\
  /* coeff_v          */ {1000,0},\
  /* coeff_r          */ {1000,0},\
  /* voltage_limit_  */  G_ES_VOLTAGE_LIMIT,\
  /* alternate_       */  GALTERNATE_NO,\
  /* phase_current1_  */  0,\
  /* phase_current2_  */  0,\
  /* phase_duration1_ */  5,\
  /* phase_duration2_ */  5,\
  /* fade_in_time_    */  0,\
  /* fade_out_time_   */  0,\
  /* fade_type_       */  GFADE_PHASE,\
  /* interphase_      */  G_ES_MIN_INTERPHASE_DURATION,\
  /* pulse_rate_       */ G_ES_MIN_PULSE_RATE,\
  /* pulses_          */  G_ES_MIN_PULSES,\
  /* do1_pre_time_    */  G_ES_MIN_DO1_PRE_TIME,\
  /* do1_post_time_   */  G_ES_MIN_DO1_POST_TIME,\
  /* do2_pre_time_    */  G_ES_MIN_DO2_PRE_TIME,\
  /* do2_post_time_   */  G_ES_MIN_DO2_POST_TIME,\
  /* do1_function_    */  GDO_TRAIN,\
  /* do2_function_    */  GDO_TRAIN,\
  /* dio_enabled_     */  0,\
  /* highvoltage_led_    */  0,\
  /* o_function_      */  GOFUNCTION_TEST,\
  /* state_           */  GSTATE_STOP,\
  /* total_charge_    */  0,\
  /* event_           */  GEVENT_START,\
  /* error_           */  GERROR_SUCCESS,\
  /* stim_event_      */  GEVENT_START,\
  /* battery1_        */  G_ES_MIN_BATTERY,\
  /* battery2_        */  G_ES_MIN_BATTERY,\
  /* di_state_        */  0,\
  /* fade_progress_   */  0,\
  /* current1_sum_    */  NOT_AVAILABLE_CURRENT1_SUM_   ,\
  /* current2_sum_    */  NOT_AVAILABLE_CURRENT2_SUM_   ,\
  /* voltage1_sum_    */  NOT_AVAILABLE_VOLTAGE1_SUM_   ,\
  /* voltage2_sum_    */  NOT_AVAILABLE_VOLTAGE2_SUM_   ,\
  /* duration1_sum_   */  0 ,\
  /* duration2_sum_   */  0 ,\
  /* common1_         */  NOT_AVAILABLE_COMMON,\
  /* common2_         */  NOT_AVAILABLE_COMMON,\
  /* common3_         */  NOT_AVAILABLE_COMMON,\
  /* sec_5V_supply_   */  NOT_AVAILABLE_SEC_5V,\
  /* applied_pulses_  */  NOT_AVAILABLE_APPLIED_PULSES_ ,\
  /* resistance_      */  NOT_AVAILABLE_RESISTANCE_     ,\
  /* current_         */  {0},\
  /* voltage_         */  {0}\
  }

/// This function does the same as error_state_change()
uint16_t validStateChange(GState new_state, GDevice *d);

/** Checks whether the GDevice field produces any inconsistencies with the limits or other fields.

This function allows to see whether the field is OK, before sending the field to the device.

\return GERROR_SUCCESS or an appropriate \ref GError code.
*/
GError error_device_field(
    volatile GDevice *d, ///< device handle
    uint16_t ofs///< offset given by RW value or rwcode()
    );
/** Checks whether the next impulse would produces a charge limit error.

This depends on the charge of the next train, i.e. on the pulse settings.

\return GERROR_SUCCESS or GERROR_CHARGELIMIT
*/
GError error_chargelimit(
    volatile GDevice *d ///< device handle
    );
/** \brief Checks if a write command can be accepted (according to SRS)

\return GError constant
*/
uint16_t error_write (
     volatile GDevice *d,///< pointer to device configuration
     uint16_t command,   ///< a GCommand value
     uint16_t offset,    ///< the offset of the field using an RW value or rwcode()
     uint16_t length     ///< the length in bytes to read or write
     );

#define abs(x) ((x)<0 ? -(x) : (x)) ///< abs() function

#define PING_TIME_MS 200 ///< This is the ping (heartbeat) period in which the PC fetches status information from the device.
#define DEVICE_READY_TIMEOUT 800 ///< This is the timeout, after which deviceReady() returns false.

#define GESTIM_MODE_COUNT 5    ///< The number of different \ref GExtra::operationmode_ values
#define GESTIM_MODE_DEVICE 0   ///< Limits and defaults are as prescribed by the device. A value of \ref GExtra::operationmode_.
#define GESTIM_MODE_BASIC 1    ///< Limits and defaults for basic mode. A value of \ref GExtra::operationmode_
#define GESTIM_MODE_HANDHELD 2 ///< Limits and defaults for handheld mode. A value of \ref GExtra::operationmode_
#define GESTIM_MODE_ADVANCED 3 ///< Limits and defaults for advanced mode. A value of \ref GExtra::operationmode_
#define GESTIM_MODE_FES 4      ///< Limits and defaults for FES mode. A value of \ref GExtra::operationmode_

#define GESTIM_MODE_MASK 0xF   ///< Mask for bits containing actual \ref GExtra::operationmode_.
#define OPERATION_NO_HEARTBEAT 0x10 ///< An option also stored in \ref GExtra::operationmode_, but beyond \ref GESTIM_MODE_MASK.

#define MIN_TRAIN_RATE 0.001       ///< in Hz
#define MAX_N_TRAINS 1000          ///< maximum number of trains in sequence
#define MIN_N_TRAINS 1             ///< minimum number of trains in sequence
#define MAX_JITTER 100             ///< maximum jitter value
#define MIN_JITTER 0               ///< minimum jitter value

#define G_TEC "g.tec"              ///< The company name as stored in the FTDI chip
#define MAX_DEVICES 16             ///< The api can only connect to #MAX_DEVICES devices simultaneously.
#define MAX_TRAIN_RATE 5           ///< In Hz (limited by \ref PING_TIME_MS)

#define MAX_ERROR_TEXT_LEN 1024    ///< Error texts must not be longer

//this order because certain modes don't allow depth
#define ELECTRODE_CIRCULAR 0 ///< A \ref GExtra::el_type_ value.
#define ELECTRODE_OTHER 1    ///< A \ref GExtra::el_type_ value.
#define ELECTRODE_DEPTH 2    ///< A \ref GExtra::el_type_ value.
#define ELECTRODE_NONE 3     ///< A \ref GExtra::el_type_ value.

#endif //GESTIMDEFAULTS_H
