/**********************************************************************
* Copyright 2015-2022, CorTec GmbH
* All rights reserved.
*
* Redistribution, modification, adaptation or translation is not permitted.
*
* CorTec shall be liable a) for any damage caused by a willful, fraudulent or grossly 
* negligent act, or resulting in injury to life, body or health, or covered by the 
* Product Liability Act, b) for any reasonably foreseeable damage resulting from 
* its breach of a fundamental contractual obligation up to the amount of the 
* licensing fees agreed under this Agreement. 
* All other claims shall be excluded. 
* CorTec excludes any liability for any damage caused by Licensee's 
* use of the Software for regular medical treatment of patients.
**********************************************************************/
#ifndef IMPLANTAPI_IIMPLANT_LISTENER_H
#define IMPLANTAPI_IIMPLANT_LISTENER_H
#include "ConnectionInfo.h"
#include "Sample.h"
#include <vector>


namespace cortec { namespace implantapi {
  
    /**
    * @brief Interface for receiving measurement data, implant state changes or implant health state changes from an
    * IImplant.
    */
    class IImplantListener
    {
    public:
        virtual ~IImplantListener() {}

        /**
          * Callback receiving stimulation state changes.
          * @param[in] isStimulating True if stimulation is in progress, false otherwise
          */
        virtual void onStimulationStateChanged(const bool isStimulating) = 0;

        /**
          * Callback receiving measurement state changes.
          * @param[in] isMeasuring True if measurement loop is running, false otherwise
          */
        virtual void onMeasurementStateChanged(const bool isMeasuring) = 0;

        /**
        * Callback receiving connection state changes.
        * @param[in] info Contains connection state for each connection.
        */
        virtual void onConnectionStateChanged(const connection_info_t& info) = 0;

        /**
        * Callback receiving measurement data.
        *
        * @param samples Vector of samples. First samples is the oldest sample, second sample is the second oldest and 
        *        so on... The pointer's ownership is passed to the listener and needs to be deleted after 
        *        processing.
        */
        virtual void onData(const std::vector<CSample>* samples) = 0;

        /**
        * Callback triggered when new supply voltage value is received from the implant.
        *
        * @param[in] voltageV Voltage of implant in volts.
        */
        virtual void onImplantVoltageChanged(const double voltageV) = 0;

        /**
        * Callback triggered when new primary coil current value is received from the external unit.
        * The primary coil refers to the coil inside the head piece of the external unit.
        *
        * @param[in] currentMilliA Coil current in milli amps.
        */
        virtual void onPrimaryCoilCurrentChanged(const double currentMilliA) = 0;

        /**
        * Callback triggered when new current control value is received from the external unit.
        * The power of the implant is controlled by the external unit. The control value
        * provides a measure of how good the coupling between the two coils is and how much
        * more power can be provided if necessary.
        *
        * @param[in] controlValue The value is between 0.0 and 100.0 percent, where 0.0 translates
        *            to no power and 100.0 translates to maximum power applied.
        */
        virtual void onImplantControlValueChanged(const double controlValue) = 0;

        /**
        * Callback triggered when new temperature value is received from the implant.
        *
        * @param[in] temperature Temperature of implant in degree Celsius.
        */
        virtual void onTemperatureChanged(const double temperature) = 0;

        /**
        * Callback when new humidity value is received from the implant.
        *
        * @param[in] humidity Relative humidity of implant in percent.
        */
        virtual void onHumidityChanged(const double humidity) = 0;

        /**
        * Callback triggered when errors inside driver library occurred during measurement and stimulation.
        * Examples:
        * - another implant is attached at runtime
        * - another external unit is attached at runtime
        *
        * @param err The error message.
        */
        virtual void onError(const std::exception& err) = 0;

        /**
        * Callback when the onData calls are processed too slow.
        */
        virtual void onDataProcessingTooSlow() = 0;

        /**
        * Callback triggered during an active stimulation, if a stimulation function or pause has been executed.
        *
        * @param[in] numFinishedFunctions The number of so far executed functions/pauses of the current command.
        */
        virtual void onStimulationFunctionFinished(const uint64_t numFinishedFunctions) = 0;

        /**
        * Callback triggered when an RF communication status package has been received from the external unit and 
        * processed by the application layer. The package contains the number of intactly received frames and 
        * information on how often certain types of transmission errors have occurred.
        *
        * @param[in] antennaQualitydBm Antenna quality as reported from the rf-link in dBm.
        * @param[in] validFramesReceived Number of valid packets.
        * @param[in] invalidHandshake Number of invalid packets due to an erronous handshake.
        * @param[in] radioCrcErrors Number of invalid packets due to CRC error.
        * @param[in] otherRxErrors Number of invalid packets due to other errors.
        * @param[in] rxQueueOverflows Number of packets lost due to the RX queue overflowing.
        * @param[in] txQueueOverflows Number of packets lost due to the TX queue overflowing.
        */
        virtual void onRfQualityUpdate(const int8_t antennaQualitydBm,
            const uint16_t validFramesReceived, const uint16_t invalidHandshake,
            const uint16_t radioCrcErrors, const uint16_t otherRxErrors,
            const uint32_t rxQueueOverflows, const uint32_t txQueueOverflows) = 0;

        /**
        * Callback triggered when the RF connection participants agreed on a channel.
        * @param[in] rfChannel ID of the RF channel.
        */
        virtual void onChannelUpdate(const uint8_t rfChannel) = 0;
    };
}}


#endif //IMPLANTAPI_IIMPLANT_LISTENER_H