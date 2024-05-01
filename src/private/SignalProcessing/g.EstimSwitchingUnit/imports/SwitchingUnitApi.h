// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SwitchingUnitSimulator_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SwitchingUnitSimulator_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#pragma once

#ifdef GSU_API_EXPORTS
#define GSU_API __declspec(dllexport)
#else
#define GSU_API __declspec(dllimport)
#endif

#include <stdint.h>

/** \mainpage Definitions and Conventions
 *
 *
 * \section SwitchingUnit Switching Unit
 * Switching Unit, an accessory of the g.Estim PRO cortical stimulator.
 *
 * \section Indexing Channel Indexing
 * All channel indexes processed by the API are supposed to be zero-based.
 *
 * \section StimulationSetting Stimulation Setting
 * A Stimulation Setting consists of two non-empty and disjoint sets of up to \ref MAX_STIM_SETTING_SIZE electrode channels each, where either set is supposed
 * to be routed to the positive or negative stimulation input, respectively.
 *
 * \section StimulationSettingList Stimulation Setting List
 * The Stimulation Setting List contains up to \ref MAX_STIM_SETTING_LIST_LENGTH Stimulation Settings and is stored directly on the Switching Unit.
 *
 * \section StimulationSettingListIterator Stimulation Setting List Iterator
 * The Stimulation Setting List Iterator addresses a Stimulation Setting within the Stimulation Setting List. This element is
 * the currently active Stimulation Setting and routed to the stimulation input channels in \ref ACTIVE state.
 *
 * \section Logging Logging
 * All function calls of the C-API are logged into the directory ``%%APPDATA%\gtec\SwitchingUnit\``
 *
 * \section Heartbeat Heartbeat
 * In idle state, the API communicates with the Switching Unit regularly to ensure the connection is intact. Once this heartbeat is interrupted for some reason (e.g., the USB cable is unplugged),
 * the Switching Unit automatically advances to the \ref READY state.
 *
 * \section Dongle g.Estim PRO Advanced Mode Dongle
 * The following functions are only available if the g.Estim PRO Advanced Mode dongle is connected to the PC:
 *  - \ref GSU_SetSwitchingMode
 *  - \ref GSU_SetAudioOutChannel
 *  - \ref GSU_AddStimulationSetting (for more than one Stimulation Setting)
 *  - \ref GSU_SetTriggerSource
 */

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct
  {

    /** \defgroup States State Machine
     * \brief Switching Unit State Machine.
     *
     * The Switching Unit features an internal state machine, which is depicted below. Solid lines indicate transitions triggered externally, e.g.,
     * by API calls, and dashed lines indicate implicit state transistions carried out by the device automatically. The state machine
     * comprises an \ref OFF, \ref STOPPED, \ref SELFTEST, \ref READY, \ref PREPARED, \ref ACTIVE, and \ref IMPEDANCE_CHECK state.
     * For details, please refer to the individual sections.
     *
     * \image html state_machine_user.png "Switching Unit State Machine." width=800px
     * \image latex state_machine_user.png "Switching Unit State Machine." width=15cm
     *
     * \see GSU_GetState, GSU_SetState
     * @{
     */

     /** \anchor OFF
      * \name OFF State
      *
      * The OFF state relates to a Switching Unit that is turned off. It can be reached from any state by powering off the device. Note that
      * only disconnecting the Switching Unit from the PC does not provoke a state transition to the OFF state. Instead, the Switching Unit
      * will automatically enter the \ref READY state if the heartbeat is lost. Upon powering on the device, it will automatically
      * advance to the \ref STOPPED state.
      */
      /**@{*/

      /** Switching Unit is in \ref OFF state. Value is \c 0x00.*/
    uint8_t STATE_OFF;

    /**@}*/


    /** \anchor READY
     * \name READY State
     *
     * The READY state is the default state without any switching elements being active. All electrode channels
     * are directly forwarded to the amplifier outputs via an internal buffer stage. In READY state, the
     * configuration of the Switching Unit should take place. The READY state is implicitly entered from
     * the \ref SELFTEST state upon a successful self-test and from the \ref IMPEDANCE_CHECK state upon a finished
     * (or canceled) impedance check. It can also be entered from the \ref PREPARED state and the \ref ACTIVE state
     * via \ref GSU_SetState. The READY state can advance to the \ref PREPARED state if (and only if) a at least one
     * Stimulation Setting was properly configured.
     */
     /**@{*/

     /** Switching Unit is in \ref READY state. Value is \c 0x01.*/
    uint8_t STATE_READY;

    /**@}*/

    /** \anchor PREPARED
     * \name PREPARED State
     *
     * In the PREPARED state, the Switching Unit is configured for programmed channel switching. Technically,
     * this state is identical to the \ref READY state, except that the Switching Unit listens on the digital in
     * for a trigger signal. If such a trigger signal is received, the device further advances to the \ref ACTIVE
     * state. The \ref ACTIVE state can also be reached from the PREPARED state via \ref GSU_SetState. The PREPARED
     * state can be entered from the \ref READY and the \ref ACTIVE state.
     */
     /**@{*/

     /** Switching Unit is in \ref PREPARED state. Value is \c 0x02.*/
    uint8_t STATE_PREPARED;

    /**@}*/

    /** \anchor ACTIVE
     * \name ACTIVE State
     *
     * In the ACTIVE state, the Switching Unit has disconnected the currently active \ref StimulationSetting
     * from the amplifier output (unless in Fast Mode, see \ref SwitchingMode) and has connected it to the stimulation input. The ACTIVE state can be entered
     * and left based on a trigger signal at the digital in. Alternatively, a software trigger can
     * be issued via appropriate calls of \ref GSU_SetState. If the ACTIVE state was entered by an API call,
     * it cannot be left by a falling edge on the digital in port. During ACTIVE state, the ACTIVE LED is turned
     * on.
     */
     /**@{*/

     /** Switching Unit is in \ref ACTIVE state. Value is \c 0x03.*/
    uint8_t STATE_ACTIVE;

    /**@}*/

    /** \anchor STOPPED
    * \name STOPPED State
    *
    * In STOPPED state, the device's behavior is identical to the READY state, except that it cannot be configured
    * or operated otherwise without completing a successful self-test (and thereby advancing to the READY state) first.
    * The STOPPED state is automatically entered upon powering on the device or a failed self-test or any other internal
    * error. It can be manually reached from the READY, PREPARED, and ACTIVE state via \ref GSU_SetState. The STOPPED
    * state can only be left via issuing a self-test, which must succeed.
    */
    /**@{*/

    /** Switching Unit is in \ref STOPPED state. Value is \c 0x04.*/
    uint8_t STATE_STOPPED;

    /**@}*/

    /** \anchor SELFTEST
     * \name SELFTEST State
     *
     * If the Switching Unit is in SELFTEST state, it performs an internal self-test. If the self-test succeeds, the
     * state machine implicitly advances to \ref READY state. If the self-test fails, the state-machine immediately enters
     * the \ref STOPPED state. The result of the self-test (also during its execution) can be queried by \ref GSU_GetSelftestResult.
     *
     * \see GSU_GetSelftestResult
     */
     /**@{*/

    /** Switching Unit is in \ref SELFTEST state. Value is \c 0x05.*/
    uint8_t STATE_SELFTEST;

    /** \anchor IMPEDANCE_CHECK
    * \name IMPEDANCE_CHECK State
    *
    * In IMPEDANCE_CHECK state, the Switching Unit performs an impedance check. If the Stimulation Setting List is not empty,
    * the impededances over all entries are determined. If the Stimulation Setting List is empty, the impedance of all channels
    * versus ground is determined. The impedance check can be interrupted by an explicit API state transition call via \ref GSU_SetState.
    * Upon completion, the state machine implicitly advances to \ref READY state.
    */
    /**@{*/

    /** Switching Unit is in \ref IMPEDANCE_CHECK state. Value is \c 0x06.*/
    uint8_t STATE_IMPEDANCE_CHECK;

    /**@}*/

    /**
     * @}
     */


     /** \defgroup Errors API Errors
      * \brief Switching Unit API errors.
      *
      * Here, the errors returned by the \ref Functions are encoded and described.
      *
      * <table>
      * <caption id="errors">Error codes overview.</caption>
      * <tr><th>Value<th>Error
      * <tr><td>\c 0x0000<td>\ref ERR_NONE
      * <tr><td>\c 0x0005<td>\ref ERR_SEMANTIC
      * <tr><td>\c 0x0006<td>\ref ERR_GENERAL
      * <tr><td>\c 0x0007<td>\ref ERR_DISCONNECTED
      * <tr><td>\c 0x0008<td>\ref ERR_DONGLE_MISSING
      * <tr><td>\c 0x0009<td>\ref ERR_INVALID_DEVICE_INDEX
      * <tr><td>\c 0x0030<td>\ref ERR_SELF_TEST_FAILED
      * <tr><td>\c 0x0031<td>\ref ERR_CMD_DENIED
      * <tr><td>\c 0x0032<td>\ref ERR_INVALID_TRANSITION
      * <tr><td>\c 0x0033<td>\ref ERR_INVALID_CHANNEL_INDEX
      * <tr><td>\c 0x0034<td>\ref ERR_STATE
      * <tr><td>\c 0x0035<td>\ref ERR_DEVICE_STOPPED
      * <tr><td>\c 0x0036<td>\ref ERR_BUSY
      * </table>
      *
      * @{
      */

    /** \anchor ERR_NONE
     * \name ERR_NONE Error Code
     *
     * This value is returned if an API call succeeds. Value is \c 0x0000.
     */
    /**@{*/

    /** No error (success). */
    uint16_t ERR_NONE;
    /**@}*/

    /** \anchor ERR_SEMANTIC
     * \name ERR_SEMANTIC Error Code
     *
     * This is a data transmission error. It occurs when the transmitted communication frames
     * are syntactically correct, but violate the internal protocol. Value is \c 0x0005.
     */
     /**@{*/
     /** Protocol error. */
    uint16_t ERR_SEMANTIC;
    /**@}*/

    /** \anchor ERR_GENERAL
     * \name ERR_GENERAL Error Code
     *
     * This error occurs if no other errors are applicable. Value is \c 0x0006.
     */
     /**@{*/
     /** General error. */
    uint16_t ERR_GENERAL;
    /**@}*/

    /** \anchor ERR_DISCONNECTED
     * \name ERR_DISCONNECTED Error Code
     *
     * This error occurs if the device has been disconnected. Value is \c 0x0007.
     */
    /**@{*/
    /** Connection error. */
    uint16_t ERR_DISCONNECTED;
    /**@}*/

    /** \anchor ERR_DONGLE_MISSING
     * \name ERR_DONGLE_MISSING Error Code
    *
    * This error occurs if the dongle is missing. Value is \c 0x0008.
    */
    /**@{*/
    /** Dongle is missing. */
    uint16_t ERR_DONGLE_MISSING;
    /**@}*/

    /** \anchor ERR_INVALID_DEVICE_INDEX
     * \name ERR_INVALID_DEVICE_INDEX Error Code
    *
    * This error occurs if the device index is out of range. Value is \c 0x0009.
    */
    /**@{*/
    /** Device index error. */
    uint16_t ERR_INVALID_DEVICE_INDEX;
    /**@}*/

    /** \anchor ERR_SELF_TEST_FAILED
     * \name ERR_SELF_TEST_FAILED Error Code
     *
     * This error occurs as a return value of \ref GSU_GetSelftestResult if the self-test
     * was not completed successfully. Value is \c 0x0030.
     */
     /**@{*/
     /**  Self-test failed. */
    uint16_t ERR_SELF_TEST_FAILED;
    /**@}*/

    /** \anchor ERR_CMD_DENIED
     * \name ERR_CMD_DENIED Error Code
     *
     * This error occurs when an API command can not be executed since it needs
     * elevated rights. This error does not occur on functions from the standard API.
     * Value is \c 0x0031.
     */
     /**@{*/
     /** Command was denied. */
    uint16_t ERR_CMD_DENIED;
    /**@}*/

    /** \anchor ERR_INVALID_TRANSITION
     * \name ERR_INVALID_TRANSITION Error Code
     *
     * This error occurs whenever a requested state transition is not allowed. Value is \c 0x0032.
     *
     * \see States
     */
     /**@{*/
     /** Invalid transition error. */
    uint16_t ERR_INVALID_TRANSITION;
    /**@}*/

    /** \anchor ERR_INVALID_CHANNEL_INDEX
     * \name ERR_INVALID_CHANNEL_INDEX Error Code
     *
     * This error occurs whenever a submitted channel index is invalid (e.g., out of range).
     * It may also happen if a submitted \ref StimulationSetting is not disjoint or is empty.
     * Value is \c 0x0033.
     */
     /**@{*/
     /** Index error. */
    uint16_t ERR_INVALID_CHANNEL_INDEX;
    /**@}*/

    /** \anchor ERR_STATE
     * \name ERR_STATE Error Code
     *
     * This error occurs whenever an API call is not allowed to be executed in the
     * current state. Value is \c 0x0034.
     */
     /**@{*/
     /** State error. */
    uint16_t ERR_STATE;
    /**@}*/

    /** \anchor ERR_DEVICE_STOPPED
     * \name ERR_DEVICE_STOPPED Error Code
    *
    * This error occurs whenever an API call cannot be executed because the device is
    * in \ref STOPPED state. Value is \c 0x0035.
    */
    /**@{*/
    /** Device is in \ref STOPPED state. */
    uint16_t ERR_DEVICE_STOPPED;
    /**@}*/

    /** \anchor ERR_BUSY
     * \name ERR_BUSY Error Code
    *
    * This error occurs whenever an API call cannot be executed because the device is
    * still busy carrying out other operations. Value is \c 0x0036.
    */
    /**@{*/
    /** Device is busy. */
    uint16_t ERR_BUSY;
    /**@}*/

    /**@}*/

    /** \defgroup Status Device Status
    * \brief Switching Unit status.
    *
    * The current device status (not to be confused with the state) can be retrieved
    * asynchronuously via \ref GSU_GetDeviceStatus. The available status codes are listed below
    *
    * <table>
    * <caption id="status">Status codes overview.</caption>
    * <tr><th>Value<th>Status
    * <tr><td>\c 0x1000<td>\ref STATUS_MAIN_INVALID_SETTING
    * <tr><td>\c 0x1001<td>\ref STATUS_MAIN_I2C_TIMEOUT
    * <tr><td>\c 0x1002<td>\ref STATUS_MAIN_ADC_OVERFLOW
    * <tr><td>\c 0x1003<td>\ref STATUS_MAIN_HW_OVERCURRENT
    * <tr><td>\c 0x1004<td>\ref STATUS_MAIN_FW_OVERCURRENT
    * <tr><td>\c 0x1005<td>\ref STATUS_MAIN_NO_PING
    * <tr><td>\c 0x1006<td>\ref STATUS_MAIN_RESET_WATCHDOG
    * <tr><td>\c 0x1007<td>\ref STATUS_MAIN_RESET_TRAP
    * <tr><td>\c 0x1008<td>\ref STATUS_MAIN_RESET_OPCODE
    * <tr><td>\c 0x1009<td>\ref STATUS_MAIN_RESET_EXT
    * <tr><td>\c 0x100A<td>\ref STATUS_MAIN_RESET_SOFTWARE
    * <tr><td>\c 0x100B<td>\ref STATUS_MAIN_RESET_BROWN_OUT
    * <tr><td>\c 0x100C<td>\ref STATUS_MAIN_RESET_POWER_ON
    * <tr><td>\c 0x100D<td>\ref STATUS_MAIN_STARTUP_ERROR
    * <tr><td>\c 0x100E<td>\ref STATUS_MAIN_U_Z_LOW
    * <tr><td>\c 0x100F<td>\ref STATUS_MAIN_CALIBRATION_ERROR
    * <tr><td>\c 0x1010<td>\ref STATUS_MAIN_MODULE_MISSING
    * <tr><td>\c 0x1F10<td>\ref STATUS_MOD_ITERATOR_WRONG
    * <tr><td>\c 0x1F11<td>\ref STATUS_MOD_CRC_ERROR
    * <tr><td>\c 0x1F12<td>\ref STATUS_MOD_ITERATOR_TIMEOUT
    * <tr><td>\c 0x1F13<td>\ref STATUS_MOD_I2C_TIMEOUT
    * <tr><td>\c 0x1F14<td>\ref STATUS_MOD_WATCHDOG
    * <tr><td>\c 0x1F15<td>\ref STATUS_MOD_UNKNOWN_COMMAND
    * <tr><td>\c 0x1F16<td>\ref STATUS_MOD_RESET_EXT
    * <tr><td>\c 0x1F17<td>\ref STATUS_MOD_RESET_TRAP
    * <tr><td>\c 0x1F18<td>\ref STATUS_MOD_RESET_OPCODE
    * <tr><td>\c 0x1F19<td>\ref STATUS_MOD_RESET_SOFTWARE
    * <tr><td>\c 0x1F1A<td>\ref STATUS_MOD_RESET_POWER_ON
    * <tr><td>\c 0x1F1B<td>\ref STATUS_MOD_RESET_BROWN_OUT
    * <tr><td>\c 0x1F1C<td>\ref STATUS_MOD_STARTUP_ERROR
    * <tr><td>\c 0xFFFF<td>\ref STATUS_OK
    * </table>
    *
    * @{
    */

    /** \name STATUS_MAIN_INVALID_SETTING
    *
    * This value is returned by \ref GSU_GetDeviceStatus if the Switching Unit main uC has experienced an internal Stimulation Setting consistency error. Value is \c 0x1000.
    */
    /**@{*/
    /** Main uC invalid Stimulation Setting. */
    uint16_t STATUS_MAIN_INVALID_SETTING;
    /**@}*/


    /** \name STATUS_MAIN_I2C_TIMEOUT
    *
    * This value is returned by \ref GSU_GetDeviceStatus if the Switching Unit main uC has experienced a timeout error on the I2C bus. Value is \c 0x1001.
    */
    /**@{*/
    /** Main uC I2C timeout error. */
    uint16_t STATUS_MAIN_I2C_TIMEOUT;
    /**@}*/


    /** \name STATUS_MAIN_ADC_OVERFLOW
    *
    * This value is returned by \ref GSU_GetDeviceStatus if the Switching Unit main uC has experienced an ADC overflow. Value is \c 0x1002.
    */
    /**@{*/
    /** Main uC ADC overflow error. */
    uint16_t STATUS_MAIN_ADC_OVERFLOW;
    /**@}*/


    /** \name STATUS_MAIN_HW_OVERCURRENT
    *
    * This value is returned by \ref GSU_GetDeviceStatus if the Switching Unit main uC has experienced an overcurrent detected by the hardware. Value is \c 0x1003.
    */
    /**@{*/
    /** Main uC hardware overcurrent. */
    uint16_t STATUS_MAIN_HW_OVERCURRENT;
    /**@}*/


    /** \name STATUS_MAIN_FW_OVERCURRENT
    *
    * This value is returned by \ref GSU_GetDeviceStatus if the Switching Unit main uC has experienced an overcurrent detected by the firmware. Value is \c 0x1004.
    */
    /**@{*/
    /** Main uC firmware overcurrent. */
    uint16_t STATUS_MAIN_FW_OVERCURRENT;
    /**@}*/


    /** \name STATUS_MAIN_NO_PING
    *
    * This value is returned by \ref GSU_GetDeviceStatus if the Switching Unit main uC has lost the connection to the PC, i.e., the \ref Heartbeat has stopped. Value is \c 0x1005.
    */
    /**@{*/
    /** \ref Heartbeat between PC and main uC has stopped. */
    uint16_t STATUS_MAIN_NO_PING;
    /**@}*/


    /** \name STATUS_MAIN_RESET_WATCHDOG
    *
    * This value is returned by \ref GSU_GetDeviceStatus if the Switching Unit main uC was reset due to a watchdog error. Value is \c 0x1006.
    */
    /**@{*/
    /** Main uC reset due to watchdog error. */
    uint16_t STATUS_MAIN_RESET_WATCHDOG;
    /**@}*/


    /** \name STATUS_MAIN_RESET_TRAP
    *
    * This value is returned by \ref GSU_GetDeviceStatus if the Switching Unit main uC was reset due to a trap. Value is \c 0x1007.
    */
    /**@{*/
    /** Main uC reset due to a trap. */
    uint16_t STATUS_MAIN_RESET_TRAP;
    /**@}*/


    /** \name STATUS_MAIN_RESET_OPCODE
    *
    * This value is returned by \ref GSU_GetDeviceStatus if the Switching Unit main uC was reset due to opcode. Value is \c 0x1008.
    */
    /**@{*/
    /** Main uC reset due to OPCODE. */
    uint16_t STATUS_MAIN_RESET_OPCODE;
    /**@}*/


    /** \name STATUS_MAIN_RESET_EXT
    *
    * This value is returned by \ref GSU_GetDeviceStatus if the Switching Unit main uC was reset due to an external trigger signal. Value is \c 0x1009.
    */
    /**@{*/
    /** Main uC reset due to external trigger. */
    uint16_t STATUS_MAIN_RESET_EXT;
    /**@}*/


    /** \name STATUS_MAIN_RESET_SOFTWARE
    *
    * This value is returned by \ref GSU_GetDeviceStatus if the Switching Unit main uC was reset due to a software command. Value is \c 0x100A.
    */
    /**@{*/
    /** Main uC reset due to software command. */
    uint16_t STATUS_MAIN_RESET_SOFTWARE;
    /**@}*/


    /** \name STATUS_MAIN_RESET_BROWN_OUT
    *
    * This value is returned by \ref GSU_GetDeviceStatus if the Switching Unit main uC was reset due to a brown out. Value is \c 0x100B.
    */
    /**@{*/
    /** Main uC reset due to brown out. */
    uint16_t STATUS_MAIN_RESET_BROWN_OUT;
    /**@}*/


    /** \name STATUS_MAIN_RESET_POWER_ON
    *
    * This value is returned by \ref GSU_GetDeviceStatus if the Switching Unit main uC was reset due to power on. This is regular behavior. Value is \c 0x100C.
    */
    /**@{*/
    /** Main uC reset due to power on. */
    uint16_t STATUS_MAIN_RESET_POWER_ON;
    /**@}*/


    /** \name STATUS_MAIN_STARTUP_ERROR
    *
    * This value is returned by \ref GSU_GetDeviceStatus if the Switching Unit main uC experienced an unknown startup error. Value is \c 0x100D.
    */
    /**@{*/
    /** Main uC unknown startup error. */
    uint16_t STATUS_MAIN_STARTUP_ERROR;
    /**@}*/


    /** \name STATUS_MAIN_U_Z_LOW
    *
    * This value is returned by \ref GSU_GetDeviceStatus if the Switching Unit main uC experienced a low voltage for the impedance measurement. Value is \c 0x100E.
    */
    /**@{*/
    /** Invalid waveform generator voltage */
    uint16_t STATUS_MAIN_U_Z_LOW;
    /**@}*/

    /** \name STATUS_MAIN_CALIBRATION_ERROR
    *
    * This value is returned by \ref GSU_GetDeviceStatus if the Switching Unit main uC experienced a calibration error. Value is \c 0x100F.
    */
    /**@{*/
    /** Calibration error. */
    uint16_t STATUS_MAIN_CALIBRATION_ERROR;
    /**@}*/

    /** \name STATUS_MAIN_MODULE_MISSING
    *
    * This value is returned by \ref GSU_GetDeviceStatus if the Switching Unit main uC reports a missing module. Value is \c 0x1010.
    */
    /**@{*/
    /** Module missing. */
    uint16_t STATUS_MAIN_MODULE_MISSING;
    /**@}*/

    /** \name STATUS_MOD_ITERATOR_WRONG
    *
    * This value is returned by \ref GSU_GetDeviceStatus if a Switching Unit module controller experienced an error due to a wrong \ref StimulationSettingListIterator. Value is \c 0x1F10.
    */
    /**@{*/
    /** Module uC wrong \ref StimulationSettingListIterator. */
    uint16_t STATUS_MOD_ITERATOR_WRONG;
    /**@}*/

    /** \name STATUS_MOD_CRC_ERROR
    *
    * This value is returned by \ref GSU_GetDeviceStatus if a Switching Unit module controller experienced a CRC error. Value is \c 0x1F11.
    */
    /**@{*/
    /** Module uC CRC error. */
    uint16_t STATUS_MOD_CRC_ERROR;
    /**@}*/


    /** \name STATUS_MOD_ITERATOR_TIMEOUT
    *
    * This value is returned by \ref GSU_GetDeviceStatus if a Switching Unit module controller experienced an error due to an unconfirmed \ref StimulationSettingListIterator. Value is \c 0x1F12.
    */
    /**@{*/
    /** Module uC unconfirmed \ref StimulationSettingListIterator. */
    uint16_t STATUS_MOD_ITERATOR_TIMEOUT;
    /**@}*/


    /** \name STATUS_MOD_I2C_TIMEOUT
    *
    * This value is returned by \ref GSU_GetDeviceStatus if a Switching Unit module controller experienced an I2C bus timeout. Value is \c 0x1F13.
    */
    /**@{*/
    /** Module uC I2C bus timeout. */
    uint16_t STATUS_MOD_I2C_TIMEOUT;
    /**@}*/


    /** \name STATUS_MOD_WATCHDOG
    *
    * This value is returned by \ref GSU_GetDeviceStatus if a Switching Unit module controller experienced a watchdog error. Value is \c 0x1F14.
    */
    /**@{*/
    /** Module uC watchdog error. */
    uint16_t STATUS_MOD_WATCHDOG;
    /**@}*/


    /** \name STATUS_MOD_UNKNOWN_COMMAND
    *
    * This value is returned by \ref GSU_GetDeviceStatus if a Switching Unit module controller received an unknown command. Value is \c 0x1F15.
    */
    /**@{*/
    /** Unknown command. */
    uint16_t STATUS_MOD_UNKNOWN_COMMAND;
    /**@}*/

    /** \name STATUS_MOD_RESET_EXT
    *
    * This value is returned by \ref GSU_GetDeviceStatus if a Switching Unit module uC was reset due to an external trigger signal. Value is \c 0x1F16.
    */
    /**@{*/
    /** Module uC reset due to external trigger. */
    uint16_t STATUS_MOD_RESET_EXT;
    /**@}*/

    /** \name STATUS_MOD_RESET_TRAP
    *
    * This value is returned by \ref GSU_GetDeviceStatus if a Switching Unit module controller was reset due to a trap. Value is \c 0x1F17.
    */
    /**@{*/
    /** Module uC reset due to a trap. */
    uint16_t STATUS_MOD_RESET_TRAP;
    /**@}*/

    /** \name STATUS_MOD_RESET_OPCODE
    *
    * This value is returned by \ref GSU_GetDeviceStatus if a Switching Unit module controller was reset due to opcode. Value is \c 0x1F18.
    */
    /**@{*/
    /** Module uC reset due to opcode. */
    uint16_t STATUS_MOD_RESET_OPCODE;
    /**@}*/

    /** \name STATUS_MOD_RESET_SOFTWARE
    *
    * This value is returned by \ref GSU_GetDeviceStatus if a Switching Unit module controller was reset due to a software command. Value is \c 0x1F19.
    */
    /**@{*/
    /** Module uC reset due to a software command. */
    uint16_t STATUS_MOD_RESET_SOFTWARE;
    /**@}*/

    /** \name STATUS_MOD_RESET_POWER_ON
    *
    * This value is returned by \ref GSU_GetDeviceStatus if a Switching Unit module controller was reset due to power on. This is regular behavior. Value is \c 0x1F1A.
    */
    /**@{*/
    /** Module uC reset due to power on. */
    uint16_t STATUS_MOD_RESET_POWER_ON;
    /**@}*/

    /** \name STATUS_MOD_RESET_BROWN_OUT
    *
    * This value is returned by \ref GSU_GetDeviceStatus if a Switching Unit module controller was reset due to brown out. Value is \c 0x1F1B.
    */
    /**@{*/
    /** Module uC reset due to brown out. */
    uint16_t STATUS_MOD_RESET_BROWN_OUT;
    /**@}*/

    /** \name STATUS_MOD_STARTUP_ERROR
    *
    * This value is returned by \ref GSU_GetDeviceStatus if a Switching Unit module controller was reset due to an unknown startup error. Value is \c 0x1F1C.
    */
    /**@{*/
    /** Module uC reset due to an unknown startup error. */
    uint16_t STATUS_MOD_STARTUP_ERROR;
    /**@}*/

    /** \name STATUS_OK
    *
    * This value is returned by \ref GSU_GetDeviceStatus if the Switching Unit is in a consistent status without any errors. Value is \c 0xFFFF.
    */
    /**@{*/
    /** Status is OK. */
    uint16_t STATUS_OK;
    /**@}*/


    /**@}*/

    /** \defgroup SwitchingMode Switching Mode
    * \brief Switching Mode.
    *
    * The Switching Unit features two modes of operation: a Standard Switching Mode, where the stimulation channels are disconnected from the amplifier output in \ref ACTIVE state. This mode is activated
    * by default and should not be changed unless it is absolutely necessary. In Fast Switching Mode, the stimulation channels are NOT disconnected from the amplifier output in \ref ACTIVE state. This leads to faster switching cycles but causes
    * potential current flow into the amplifier, which may yield saturation effects and diminish the current delivered to the electrodes. Use the Fast Switching Mode with care.
    *
    * <table>
    * <caption id="modes">Switching Modes.</caption>
    * <tr><th>Value<th>Mode
    * <tr><td>\c 0x0000<td>\ref SWITCHING_MODE_STANDARD
    * <tr><td>\c 0x0001<td>\ref SWITCHING_MODE_FAST
    * </table>
    *
    * @{
    */

    /** \name SWITCHING_MODE_STANDARD
    *
    * Standard Switching Mode. Value is \c 0x0000.
    */
    /**@{*/
    /** Standard Switching Mode. */
    uint16_t SWITCHING_MODE_STANDARD;
    /**@}*/

    /** \name SWITCHING_MODE_FAST
    *
    * Fast Switching Mode. Value is \c 0x0001.
    */
    /**@{*/
    /** Fast Switching Mode. */
    uint16_t SWITCHING_MODE_FAST;
    /**@}*/

    /**@}*/

    /**@}*/

    /** \defgroup TriggerSource Trigger Source
    * \brief Trigger Source.
    *
    * The Switching Unit features two trigger sources: Software and Hardware.
    *
    * <table>
    * <caption id="sources">Trigger Sources.</caption>
    * <tr><th>Value<th>Source
    * <tr><td>\c 0x0000<td>\ref TRIGGER_SOURCE_SOFTWARE
    * <tr><td>\c 0x0001<td>\ref TRIGGER_SOURCE_HARDWARE
    * </table>
    *
    * @{
    */

    /** \name TRIGGER_SOURCE_SOFTWARE
    *
    * Software trigger source. Value is \c 0x0000.
    */
    /**@{*/
    /** Software trigger source. */
    uint16_t TRIGGER_SOURCE_SOFTWARE;
    /**@}*/

    /** \name TRIGGER_SOURCE_HARDWARE
    *
    * Hardware trigger source. Value is \c 0x0001.
    */
    /**@{*/
    /** Hardware trigger source. */
    uint16_t TRIGGER_SOURCE_HARDWARE;
    /**@}*/

    /**@}*/


    /**@}*/

    /** \defgroup Constants API Constants
    * \brief Switching Unit API constants.
    *
    * Here, some general API constants are documented.
    *
    * @{
    */

    /** \anchor MAX_STIM_SETTING_LIST_LENGTH
     * \name MAX_STIM_SETTING_LIST_LENGTH
     *
     * Maximum length of the \ref StimulationSettingList. Value is \c 32.
     */

     /**@{*/

     /** Maximum \ref StimulationSettingList length. */
    uint8_t MAX_STIM_SETTING_LIST_LENGTH;

    /**@}*/


    /** \anchor MAX_STIM_SETTING_SIZE
     * \name MAX_STIM_SETTING_SIZE
     *
     * Maximum size of a \ref StimulationSetting. Value is \c 16.
     */

     /**@{*/

     /** Maximum \ref StimulationSetting size. */
    uint8_t MAX_STIM_SETTING_SIZE;

    /**@}*/

    /** \anchor MAX_GND_CHANNELS
    * \name MAX_GND_CHANNELS
    *
    * Maximum number of dedicated ground channels. Value is \c 16.
    */

    /**@{*/

    /** Maximum number of audio out channels. */
    uint8_t MAX_GND_CHANNELS;

    /**@}*/

    /** \anchor IMPEDANCE_NOT_AVAILABLE
    * \name IMPEDANCE_NOT_AVAILABLE
    *
    * Impedance is not (yet) available. Value is \c -1.0.
    */

    /**@{*/

    /** Impedance not available. */
    double IMPEDANCE_NOT_AVAILABLE;

    /**@}*/

    /** \anchor IMPEDANCE_HIGH
    * \name IMPEDANCE_HIGH
    *
    * High impedance. Value is \c -2.0.
    */

    /**@{*/

    /** High impedance. */
    double IMPEDANCE_HIGH;

    /**@}*/

    /** \anchor IMPEDANCE_LOW
    * \name IMPEDANCE_LOW
    *
    * Low impedance. Value is \c -3.0.
    */

    /**@{*/

    /** Low impedance. */
    double IMPEDANCE_LOW;

    /**@}*/


  } GSU_Constants;


  /** \defgroup Functions API Functions
   * \brief Switching Unit API functions.
   *
   * Here, the API functions of the Switching Unit are described.
   *
   * @{
   */

   /** Provides the API constants.
    *
    * \returns API constants defined in \ref GSU_Constants.
    */
  GSU_API GSU_Constants* GSU_GetConstants();


  /** Gets the number of currently connected devices.
   *
   * \param[out]  num_dev  Number of devices
   * \returns              API error code (see \ref Errors)
   */
  GSU_API uint16_t GSU_GetNumberOfAvailableDevices(uint8_t* num_dev);


  /** Gets the label of a device.
   *
   * Note that \ref GSU_GetNumberOfAvailableDevices must be called beforehand.
   *
   * \param[in]   idx    Device index
   * \param[out]  label  Device label
   * \returns            API error code (see \ref Errors)
   */
  GSU_API uint16_t GSU_GetDeviceLabel(uint8_t idx, char const** label);


  /** Opens a device.
   *
   * \param[in]  idx  Device index
   * \returns         API error code (see \ref Errors)
   *
   * \see GSU_Close
   */
  GSU_API uint16_t GSU_Open(uint8_t idx);

  /** Closes a device.
   *
   * \param[in]  idx  Device index
   * \returns         API error code (see \ref Errors)
   *
   * \see GSU_Open
   */
  GSU_API uint16_t GSU_Close(uint8_t idx);


  /** Gets the current status of the device.
  *
  * \see \ref Status
  *
  * \param[in]   idx     Device index
  * \param[out]  status  Device status
  * \returns             API error code (see \ref Errors)
  */
  GSU_API uint16_t GSU_GetDeviceStatus(uint8_t idx, uint16_t* status);


  /** Gets the results of the most recently executed self-test.
  *
  * This function can be called from any state.
  *
  * <table>
  * <caption id="progress">Self-test scenarios.</caption>
  * <tr><th>``progress``<th>``step``<th>Description
  * <tr><td>     0      <td>   0    <td>Self-test not started yet (default). ``module`` are ``0x0000``. Return value is \ref ERR_NONE.
  * <tr><td>  0 ... 1   <td>   4    <td>Self-test running. Electrodes are being tested (all electrodes must be disconnected). ``substep`` encodes the module currently checked. Return value is \ref ERR_NONE.
  * <tr><td>  0 ... 1   <td>   5    <td>Self-test running. Channel switching is being tested. ``substep`` encodes the module currently checked. Return value is \ref ERR_NONE.
  * <tr><td>  0 ... 1   <td>   6    <td>Self-test running. Impedance check is being tested. ``substep`` encodes the current substep. Return value is \ref ERR_NONE.
  * <tr><td>     1      <td>   3    <td>Self-test has finished. If failed, ``substep`` encodes the failed substep(s). Return value is \ref ERR_NONE if the self-test succeeded, and \ref ERR_SELF_TEST_FAILED otherwise.
  * </table>
  *
  * \param[in]   idx       Device index
  * \param[out]  progress  Self-test progress (between 0 and 1)
  * \param[out]  step      Self-test step
  * \param[out]  substep   One-hot encoded substep
  * \returns               API error code (see \ref Errors)
  *
  */
  GSU_API uint16_t GSU_GetSelftestResult(uint8_t idx, double *progress, uint8_t* step, uint16_t* substep);


  /** Gets the hardware version of the device.
   *
   * The hardware version is returned as a string of the format ``MM.mm.bbbb``, where ``MM`` is the
   * two-digit major version, ``mm`` is the two-digit minor version, and ``bbbb`` is the four-digit
   * batch number.
   *
   * This function can be called in any state of the \ref States.
   *
   * \param[in]   idx  Device index
   * \param[out]  ver  Hardware version
   * \returns          API error code (see \ref Errors)
   */
  GSU_API uint16_t GSU_GetHardwareVersion(uint8_t idx, char const** ver);


  /** Gets the firmware version of the device.
   *
   * The firmware version is returned as a string of the format ``MM.mm.bbbb``, where ``MM`` is the
   * two-digit major version, ``mm`` is the two-digit minor version, and ``bbbb`` is the four-digit
   * batch number.
   *
   * <table>
   * <caption id="uc_idx">Microcontroller Index.</caption>
   * <tr><th>``uc_idx``<th>Addressed microcontroller
   * <tr><td>0  <td>uC1 (main microcontroller)
   * <tr><td>i  <td>ucM i (i-th module microcontroller, i ranging from 1 to no. of modules)
   * </table>
   *
   * This function can be called in any state of the \ref States.
   *
   * \param[in]   idx      Device index
   * \param[in]   uc_idx   Microcontroller index
   * \param[out]  ver      Firmware version
   * \returns              API error code (see \ref Errors)
   */
  GSU_API uint16_t GSU_GetFirmwareVersion(uint8_t idx, uint8_t uc_idx, char const** ver);


  /** Gets the serial number of the device.
   *
   * The serial number is returned as a string of the format ``YYYY.MM.DD``, where ``YYYY`` is the
   * four-digit year of production, ``MM`` is the two-digit month of production, and ``DD`` is the two-digit
   * device number.
   *
   * This function can be called in any state of the \ref States.
   *
   * \param[in]   idx  Device index
   * \param[out]  sn   Serial number
   * \returns          API error code (see \ref Errors)
   */
  GSU_API uint16_t GSU_GetSerialNumber(uint8_t idx, char const** sn);


  /** Gets the number of available modules in the device.
   *
   * This function can be called in any state of the \ref States.
   *
   * \param[in]   idx     Device index
   * \param[out]  no_mod  Number of modules
   * \returns             API error code (see \ref Errors)
   */
  GSU_API uint16_t GSU_GetNumberOfModules(uint8_t idx, uint8_t* no_mod);


  /** Gets the current state of the device.
   *
   * This function can be called in any state of the \ref States.
   *
   * \param[in]   idx    Device index
   * \param[out]  state  Device state (see \ref States)
   * \returns            API error code (see \ref Errors)
   *
   * \see GSU_GetState.
   */
  GSU_API uint16_t GSU_GetState(uint8_t idx, uint8_t* state);


  /** Advances the device to a state.
   *
   * Depending on the allowed transitions and conditions of the \ref States, a call of this function might
   * fail or succeed.
   *
   * \param[in]  idx    Device index
   * \param[in]  state  Device state (see \ref States)
   * \returns           API error code (see \ref Errors)
   *
   * \see GSU_GetState
   */
  GSU_API uint16_t GSU_SetState(uint8_t idx, uint8_t state);


  /** Gets the \ref SwitchingMode.
  *
  * This function can be called in any state of the \ref States.
  *
  * \param[in]   idx      Device index
  * \param[out]  mode     \ref SwitchingMode
  * \returns              API error code (see \ref Errors)
  *
  * \see GSU_SetSwitchingMode
  */
  GSU_API uint16_t GSU_GetSwitchingMode(uint8_t idx, uint8_t* mode);


  /** Sets the \ref SwitchingMode.
  *
  * This function can be called only in \ref READY state. Note that this function requires the \ref Dongle.
  *
  * \param[in]   idx      Device index
  * \param[in]   mode     \ref SwitchingMode
  * \returns              API error code (see \ref Errors)
  *
  * \see GSU_GetSwitchingMode
  */
  GSU_API uint16_t GSU_SetSwitchingMode(uint8_t idx, uint8_t mode);


  /** Retrieves the amplifier ground channels.
  *
  * This function can be called in any state of the \ref States.
  *
  * \param[in]   idx     Device index
  * \param[out]  ch      List of electrode channels to be routed to the amplifier ground
  * \param[out]  no_ch   Length of electrode channel list ``ch``
  * \returns             API error code (see \ref Errors)
  *
  * \see GSU_SetAmplifierGroundChannels
  */
  GSU_API uint16_t GSU_GetAmplifierGroundChannels(uint8_t idx, uint8_t** ch, uint8_t* no_ch);


  /** Sets the amplifier ground channels.
  *
  * The g.Estim PRO Switching unit allows setting up to \ref MAX_GND_CHANNELS electrode channels ``ch`` as
  * the amplifier ground. If this list is empty, the headbox channel is used.
  *
  * This function can be called only in \ref READY state.
  *
  * \param[in]  idx     Device index
  * \param[in]  ch      List of channel indexes to be used as ground channels. If ``no_ch`` is zero, it can be a null pointer.
  * \param[in]  no_ch   Number of ground channels to be used (if zero, the headbox ground will be used)
  * \returns            API error code (see \ref Errors)
  *
  * \see GSU_GetAmplifierGroundChannels
  */
  GSU_API uint16_t GSU_SetAmplifierGroundChannels(uint8_t idx, uint8_t* ch, uint8_t no_ch);


  /** Gets the Audio Out channel.
  *
  * Retrieves the channel ``ch`` that is currently routed to the g.Estim PRO Switching Unit Audio Out.
  *
  * \param[in]   idx     Device index
  * \param[out]  ch      Channel index routed to audio out (if enabled)
  * \param[out]  enabled Audio enabled
  * \returns             API error code (see \ref Errors)
  *
  * \see GSU_SetAudioOutChannel
  */
  GSU_API uint16_t GSU_GetAudioOutChannel(uint8_t idx, uint8_t** ch, uint8_t* enabled);


  /** Sets the Audio Out channel.
  *
  * The g.Estim PRO Switching Unit allows routing a dedicated electrode channel ``ch`` to the
  * Audio Out of the device. This requires ``enable`` to be set to ``true``, otherwise
  * this feature is disabled.
  *
  * This function can be called only in \ref READY state.  Note that this function requires the \ref Dongle.
  *
  * \param[in]  idx     Device index
  * \param[in]  ch      Electrode index to be routed to audio out. If ``enable`` is zero, it can be a null pointer.
  * \param[in]  enable  Enable audio
  * \returns            API error code (see \ref Errors)
  *
  * \see GSU_GetAudioOutChannel
  */
  GSU_API uint16_t GSU_SetAudioOutChannel(uint8_t idx, uint8_t* ch, uint8_t enable);


  /** Adds a \ref StimulationSetting to the \ref StimulationSettingList.
   *
   * This function can be called only in \ref READY state.  Note that this function requires the \ref Dongle.
   * To set single Stimulation Settings without the dongle, use \ref GSU_SetStimulationSetting.
   *
   * \param[in] idx        Device index
   * \param[in] ch_pos     List of electrode channels to be routed to the positive stimulation input
   * \param[in] no_ch_pos  Length of electrode channel list ``ch_pos``
   * \param[in] ch_neg     List of electrode channels to be routed to the negative stimulation input
   * \param[in] no_ch_neg  Length of electrode channel list ``ch_neg``
   * \returns              API error code (see \ref Errors)
   *
   * \see GSU_SetStimulationSetting, GSU_GetStimulationSetting, GSU_ClearStimulationSettingList
   */
  GSU_API uint16_t GSU_AddStimulationSetting(uint8_t idx, uint8_t* ch_pos, uint8_t no_ch_pos, uint8_t* ch_neg, uint8_t no_ch_neg);


  /** Sets a \ref StimulationSetting.
   *
   * This function applies a Stimulation Setting. Specifically, it clears the \ref StimulationSettingList
   * and inserts the provided Stimulation Setting as a single new item into the list. This function allows
   * hot-switching the stimulation channels.
   *
   * This function can be called in \ref READY and \ref READY state.
   *
   * \param[in]  idx         Device index
   * \param[in]  ch_pos      List of electrode channels to be routed to the positive stimulation input
   * \param[in]  no_ch_pos   Length of electrode channel list ``ch_pos``
   * \param[in]  ch_neg      List of electrode channels to be routed to the negative stimulation input
   * \param[in]  no_ch_neg   Length of electrode channel list ``ch_neg``
   * \returns                API error code (see \ref Errors)
   *
   * \see GSU_AddStimulationSetting, GSU_GetStimulationSetting, GSU_ClearStimulationSettingList
   */
  GSU_API uint16_t GSU_SetStimulationSetting(uint8_t idx, uint8_t* ch_pos, uint8_t no_ch_pos, uint8_t* ch_neg, uint8_t no_ch_neg);


  /** Clears the \ref StimulationSettingList
   *
   * This function can be called only in \ref READY state.
   *
   * \param[in]  idx  Device index.
   * \returns         API error code (see \ref Errors)
   *
   * \see GSU_AddStimulationSetting, GSU_SetStimulationSetting, GSU_GetStimulationSetting
   */
  GSU_API uint16_t GSU_ClearStimulationSettingList(uint8_t idx);


  /** Gets the length of the \ref StimulationSettingList
   *
   * This function can be called in any state of the \ref States.
   *
   * \param[in]   idx     Device index
   * \param[out]  length  Stimulation Setting List length
   * \returns             API error code (see \ref Errors)
   */
  GSU_API uint16_t GSU_GetStimulationSettingListLength(uint8_t idx, uint8_t* length);


  /** Retrieves a \ref StimulationSetting within the \ref StimulationSettingList.
   *
   * This function can be called in any state of the \ref States.
   *
   * \param[in]   idx         Device index
   * \param[in]   iter        \ref StimulationSettingListIterator
   * \param[out]  ch_pos      List of electrode channels to be routed to the positive stimulation input
   * \param[out]  no_ch_pos  Length of electrode channel list ``ch_pos``
   * \param[out]  ch_neg      List of electrode channels to be routed to the negative stimulation input
   * \param[out]  no_ch_neg  Length of electrode channel list ``ch_neg``
   * \returns                 API error code (see \ref Errors)
   *
   * \see GSU_GetStimulationSettingListLength, GSU_AddStimulationSetting
   */
  GSU_API uint16_t GSU_GetStimulationSetting(uint8_t idx, uint8_t iter, uint8_t** ch_pos, uint8_t* no_ch_pos, uint8_t** ch_neg, uint8_t* no_ch_neg);


  /** Gets the current \ref StimulationSettingListIterator.
   *
   * This function can be called in any state of the \ref States.
   *
   * \param[in]   idx   Device index
   * \param[out]  iter  \ref StimulationSettingListIterator
   * \returns           API error code (see \ref Errors)
   *
   * \see GSU_GetStimulationSettingListLength
   */
  GSU_API uint16_t GSU_GetStimulationSettingListIterator(uint8_t idx, uint16_t* iter);


  /** Configures the impedance check.
  *
  *
  * If \ref StimulationSettingList is populated, the vector \c k (of length \c Nk) addresses the elements therein. Otherwise,
  * the vector \c k relates to the channels per se, whose impedances are checked against GND.
  * Likewise, the vector \c f (of length \c Nf) contains the frequencies in Hz to be checked.
  *
  * This function can be called in \ref READY state.
  *
  * \param[in]    idx    Device index
  * \param[in]    k      Channel (or Stimulation Setting) indices to be checked.
  * \param[in]    f      Frequencies (Hz) to be checked.
  * \param[in]    Nk  	 Length of k.
  * \param[in]    Nf     Length of f.
  * \param[in]    Nr     No. of measurement repetitions.
  * \param[in]    calib  1: Apply calibration; 0: Keep raw impedances.
  * \returns             API error code (see \ref Errors)
  *
  * \see GSU_GetImpedanceCheckResult
  */
  GSU_API uint16_t GSU_ConfigureImpedanceCheck(uint8_t idx, uint8_t* k, double* f, uint16_t Nk, uint16_t Nf, uint16_t Nr, uint8_t calib);


  /** Gets the results of the latest impedance check.
  *
  * Impedance is checked according to \ref GSU_ConfigureImpedanceCheck.
  *
  * The impedance values are returned in kOhms as an array ``z`` containing ``Nk*Nf*Nr`` values, where
  *
  * - \c Nk is the length of the channel (or Stimulation Settings) vector ``k``,
  * - \c Nf is the length of the frequency vector ``f, and
  * - \c Nr is the number of measurement repetitions.
  *
  * The respective impedance at channel (or Stimulation Setting) ``k[ki]``,
  * frequency ``f[fi]``, and repetition ``r`` can be accessed by ``z[r*Nk*Nf+fi*Nk+ki]``, where
  * ``0\<=ki\<Nk``, ``0\<=fi\<Nf``, and ``0\<=r\<Nr``. If incomplete, missing values are indicated by zero. For convenience,
  * ``prog`` provides the current measurement progress.
  *
  * This function can be called in any state of the \ref States.
  *
  * \param[in]    idx      Device index
  * \param[out]   z        Impedance values. See \ref IMPEDANCE_NOT_AVAILABLE, \ref IMPEDANCE_HIGH, and \ref IMPEDANCE_LOW for special values.
  * \param[out]   prog     Measurement progress, between 0 (no values available) and 1 (completed). Optional parameter (can be set to nullptr).
  * \param[out]   k        Channel (or Stimulation Setting) vector. Optional parameter (can be set to nullptr).
  * \param[out]   f        Frequency vector (Hz). Optional parameter (can be set to nullptr).
  * \param[out]   Nk  	   Length of k. Optional parameter (can be set to nullptr).
  * \param[out]   Nf       Length of f. Optional parameter (can be set to nullptr).
  * \param[out]   Nr       No. of measurement repetitions. Optional parameter (can be set to nullptr).
  * \param[out]   calib    1: values are calibrated; 0: values are raw impedances.
  * \returns               API error code (see \ref Errors)
  *
  * \see GSU_ConfigureImpedanceCheck GSU_GetImpedanceCheckResultAt
  */
  GSU_API uint16_t GSU_GetImpedanceCheckResult(uint8_t idx, double** z, double* prog, uint8_t** k, double** f, uint16_t* Nk, uint16_t* Nf, uint16_t* Nr, uint8_t *calib);


  /** Gets the results of the latest impedance check for given indices.
  *
  * Convenience access function for \ref GSU_GetImpedanceCheckResult. Impedance check must have completed before this function can be called.
  *
  * This function can be called in any state of the \ref States.
  *
  * \param[in]    idx      Device index
  * \param[out]   z        Impedance value
  * \param[out]   k_idx    Channel index
  * \param[out]   f_idx    Frequency index
  * \param[out]   r_idx    Repetition index
  * \param[out]   calib    1: values are calibrated; 0: values are raw impedances.
  * \returns               API error code (see \ref Errors)
  *
  * \see GSU_GetImpedanceCheckResult GSU_ConfigureImpedanceCheck
  */
  GSU_API uint16_t GSU_GetImpedanceCheckResultAt(uint8_t idx, double* z, uint16_t k_idx, uint16_t f_idx, uint16_t r_idx, uint8_t *calib);


  /** Gets the software version of the device.
  *
  * The software version is returned as "major.minor.build.revision" string.
  *
  * This function can be called in any state of the \ref States.
  *
  * \param[out]     version   Software version
  * \returns				          API error code (see \ref Errors)
  */
  GSU_API uint16_t GSU_GetSoftwareVersion(char const** version);


  /** Sets the trigger source.
  *
  * This function can be called in \ref READY state.  Note that this function requires the \ref Dongle.
  *
  * \param[in]   idx         Device index
  * \param[in]   source	     \ref TriggerSource
  * \returns                 API error code (see \ref Errors)
  *
  * \see GSU_GetTriggerSource
  */
  GSU_API uint16_t GSU_SetTriggerSource(uint8_t idx, uint8_t source);


  /** Gets the trigger source.
  *
  * This function can be called in any state of the \ref States.
  *
  * \param[in]   idx         Device index
  * \param[out]  source	     \ref TriggerSource
  * \returns                 API error code (see \ref Errors)
  *
  * \see GSU_SetTriggerSource
  */
  GSU_API uint16_t GSU_GetTriggerSource(uint8_t idx, uint8_t* source);


  /**@}*/

  /**@}*/

#ifdef __cplusplus
}
#endif
