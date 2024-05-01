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
#ifndef IMPLANTAPI_IIMPLANT_H
#define IMPLANTAPI_IIMPLANT_H
#include "ImplantInfo.h"
#include "IImplantListener.h"
#include "IStimulationCommand.h"
#include <stdint.h>
#include <set>

namespace cortec { namespace implantapi {

    /**
    * @brief Enumeration of amplification factors that can be applied to recorded data 
    * on the implant.
    *
    * Note that the given values are selected to reduce noise over linearity. Only min and max values
    * for the amplification factor are possible.
    */
    enum class RecordingAmplificationFactor
    {
        AMPLIFICATION_57_5dB,
        AMPLIFICATION_51_5dB,
        AMPLIFICATION_45_5dB,
        AMPLIFICATION_39_5dB
    };

    /**
    * @brief Enumeration of the stimulation modes that can be applied to configure the 
    * preloading operation of stimulation commands and functions on the implant.
    */
    enum class StimulationMode
    {
        STIM_MODE_VOLATILE_CMD_PRELOADING = 0,
        STIM_MODE_PERSISTENT_CMD_PRELOADING,
        STIM_MODE_PERSISTENT_FUNC_PRELOADING,
        STIM_MODE_COUNT
    };

    /**
    * @brief Interface for an implant.
    * 
    * Generic implant interface for all kinds of implants. The implementing object of this interface acts as a facade to
    * the hardware abstraction layer. Stimulation is triggered as an IStimmulationCommand, which is checked for compliance
    * with the corresponding implant stimulation constraints and processed if it satisfies the requirements.
    */
    class IImplant
    {
    public:
        virtual ~IImplant() {}

        /**
          * Register listener object, which is notified on arrival of new data and errors. Needs to be called
          * once before starting the measurement loop. On consecutive calls only the latest registered listener
          * will be notified. If listener == nullptr, listener will be deregistered.
          */
        virtual void registerListener(IImplantListener* listener) = 0;

        /**
        * @return informations about the implant. The ownership of CImplantInfo is passed to the caller.
        */
        virtual CImplantInfo* getImplantInfo() const = 0;

        /**
        * Starts measurement of data. Note that only the measurement result of the channels that are not used as reference
        * electrodes is valid. The measurement result for the reference electrodes is the reference value.
        *
        * @param refChannels List of channel indices that are used as composite reference electrode. Can be empty.
        * @param amplificationFactor Amplification factor that is applied to the recorded data on the implant.
        * @param useGndElectrode Use ground electrode while measuring.
        *
        * \if INTERN_IMPLANTAPI
        *   @throws CRuntimeException if the measurement is started already
        * \else
        *   @throws std::exception if the measurement is started already
        * \endif
        */
        virtual void startMeasurement(const std::set<uint32_t>& refChannels = {},
            const RecordingAmplificationFactor amplificationFactor = RecordingAmplificationFactor::AMPLIFICATION_57_5dB,
            const bool useGndElectrode = false) = 0;

        /**
        * Stops measurement of data. 
        */
        virtual void stopMeasurement() = 0;

        /**
        * Starts impedance measurement of one channel. This is a blocking call. Impedance measurement is only possible
        * while no measurement or stimulation is running.
        *
        * @param[in] channel number of the channel, whose impedance should be measured. Number starts with 0.
        * @return impedance in Ohm.
        */
        virtual double getImpedance(const uint32_t channel) = 0;

        /**
        * Measures the temperature within implant capsule. This is a blocking call. It will
        * stop running measurements.
        *
        * @return temperature in degree Celsius.
        */
        virtual double getTemperature() = 0;
        
        /**
        * Measures the humidity within implant capsule. This is a blocking call. It will
        * stop running measurements.
        *
        * @return humidity in %rh.
        */
        virtual double getHumidity() = 0;

        /**
        * Checks a stimulation command for validity, as if it would be used in startStimulation.
        *
        * @param[in] cmd The stimulation command to be checked
        * @param[out] message An error message telling why the command is not valid. Equals the 
                              empty string if the command is valid.
        *
        * @return true if the command is valid and false otherwise.
        */
        virtual bool isStimulationCommandValid(const IStimulationCommand* cmd, std::string* message) = 0;

        /**
        * Sends stimulation functions of a command for enqueuing. The behaviour of enqueuing will be determined 
        * depending on the stimulation mode selected.
        *
        * @param[in] cmd the stimulation command whose functions are needed to be enqueued.
        * @param[in] mode the Stimulation mode.
        *
        * @return A list of command faults that were corrected automatically.
        *
        * \if INTERN_IMPLANTAPI
        *   @throws CInvalidArgumentException if cmd is nullptr.
        *   @throws CRuntimeException if stimulation command cannot be sent to implant, e.g. because it is currently
        *                             disconnected.
        * \else
        *   @throws std::exception if cmd is nullptr or if cmd is not suitable for the selected stimulation mode.
        *   @throws std::exception if enqueue command cannot be sent to implant, e.g. because it is currently
        *                          disconnected.
        * \endif
        */
        virtual std::vector<IStimulationCommand::StimulationCommandFault> enqueueStimulationCommand(const IStimulationCommand* cmd,
            const StimulationMode mode) = 0;

        /**
        * Starts a stimulation. This is a non-blocking call in the sense that it does not wait till the stimulation is done,
        *     rather it only waits till an acknowledgment is received from the implant.
        * This start stimulation function can be used exclusively with STIM_MODE_PERSISTENT_FUNC_PRELOADING. It can only
        * execute one individual stimulation function specified via a function ID.
        *
        * @param[in] stimulationFunctionID The index of the stimulation function to be executed.
        * @throws CRuntimeException if the selected stimulation mode is not compitable.
        */
        virtual void startStimulation(const uint8_t stimulationFunctionID) = 0;

        /**
        * Starts a stimulation. This is a non-blocking call in the sense that it does not wait till the stimulation is done,
        *     rather it only waits till an acknowledgment is received from the implant.
        * This stimulation call is compatible with STIM_MODE_VOLATILE_CMD_PRELOADING and STIM_MODE_PERSISTENT_CMD_PRELOADING
        * Modes.
        *
        * \if INTERN_IMPLANTAPI
        *   @throws CRuntimeException if the selected stimulation mode is not compitable.
        *   @throws CRuntimeException if stimulation command cannot be sent to implant, e.g. because it is currently  
        *                             disconnected.
        * \else
        *   @throws std::exception if the selected stimulation mode is not compitable.
        *   @throws std::exception if stimulation command cannot be sent to implant, e.g. because it is currently 
        *                          disconnected.
        * \endif
        */
        virtual void startStimulation() = 0;

        /**
        * Stops stimulation.
        *
        * \if INTERN_IMPLANTAPI
        *   @throws CRuntimeException if stimulation command cannot be sent to implant, e.g. because it is currently
        *                             disconnected.
        * \else
        *   @throws std::exception if stimulation command cannot be sent to implant, e.g. because it is currently
        *                          disconnected.
        * \endif
        */
        virtual void stopStimulation() = 0;

        /**
        * Enable or disable power transfer to the implant (enabled by default). Note that powering on the
        * implant takes some milliseconds before it is responsive.
        * @param[in] enabled if true the power transfer to the implant is initiated else it is stopped
        * \if INTERN_IMPLANTAPI
        *   @throws CTimeoutException if timeout
        *   @throws CRuntimeException if invalid response.
        * \else
        *   @throws std::exception if timeout
        *   @throws std::exception if invalid response.
        * \endif
        */
        virtual void setImplantPower(const bool enabled) = 0;

        /**
        * Tells the implant to propagate the current measurement and stimulation states through the registered
        * listener.
        *
        * This method can be used to synchronize GUIs with the implant's state (i.e., is it currently measuring or
        * stimulating or both?).
        *
        * Usage:
        * 1. register listener
        * 2. call pushState()
        * 3. update GUI based on callback from listener (onMeasurementStateChanged()/onStimulationStateChanged())
        *
        * \if INTERN_IMPLANTAPI
        *   @throws CTimeoutException if timeout
        *   @throws CRuntimeException if invalid response.
        * \else
        *   @throws std::exception if timeout
        *   @throws std::exception if invalid response.
        * \endif
        */
        virtual void pushState() = 0;

        /**
        * Enables or disables the periodic polling of the radio connection quality. Quality updates are only
        * polled during implant idle mode.
        * 
        * @param[in] enable Starts rf quality polling if true, stops it otherwise.
        */
        virtual void setRFQualityPolling(const bool enable) = 0;
    };
}}


#endif //IMPLANTAPI_IIMPLANT_H