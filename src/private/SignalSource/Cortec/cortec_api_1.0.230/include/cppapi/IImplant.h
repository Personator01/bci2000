/**********************************************************************
* Copyright 2015-2023, CorTec GmbH
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
#include "cppapi/ImplantInfo.h"
#include "cppapi/IImplantListener.h"
#include "cppapi/IStimulationCommand.h"
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
    /// In this mode, the stimulation command is deleted when the execution is finished.
    STIM_MODE_VOLATILE_CMD_PRELOADING = 0,
    /// In this mode, the stimulation command is NOT deleted when the execution is finished.
    /// A stimulation with the same command can be directly started.
    STIM_MODE_PERSISTENT_CMD_PRELOADING,
    /// In this mode, a single function of the stimulation command can be individually started by
    /// referencing to its id. The functions are not deleted after their execution.
    STIM_MODE_PERSISTENT_FUNC_PRELOADING,
    STIM_MODE_COUNT
};

/// Ground electrode channel index.
constexpr uint32_t GROUND_CHANNEL_INDEX = std::numeric_limits<uint32_t>::max();

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
    * will be notified. If listener == nullptr, a previously registered listener is deregistered.
    *
    * @param listener Listener to be registered (or nullptr if no listener shall be used). The ownership is not passed.
    */
    virtual void registerListener(IImplantListener* listener) = 0;

    /**
    * @return Information about the implant. The ownership of CImplantInfo is passed to the caller.
    */
    virtual CImplantInfo* getImplantInfo() const = 0;

    /**
    * Starts measurement of data. Note that only the measurement results of the channels that are not used as reference
    * channels are valid. The measurement result for the reference channels is the reference value.
    *
    * If a persistent stimulation mode was used to preload stimulation commands,
    * the stimulation channels and the measurement reference channels need to be disjoint.
    *
    * @param refChannels List of channel indices that are used as composite reference electrode. Can be empty.
    * @param amplificationFactor Amplification factor that is applied to the recorded data on the implant.
    * @param useGndElectrode Use ground electrode while measuring.
    *
    * \if INTERN_IMPLANTAPI
    *   @throws CRuntimeException if the measurement is started already.
    * \else
    *   @throws std::exception if the measurement is started already.
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
    * @param[in] channel Number of the channel whose impedance should be measured. 
    *                    Regular channel number starts with 0. 
    *                    Use GROUND_CHANNEL_INDEX to test the impedance of the ground electrode channel.
    * 
    * @return Impedance in Ohm.
    */
    virtual double getImpedance(const uint32_t channel) = 0;

    /**
    * Measures the temperature within the implant capsule. This is a blocking call. It will stop running measurements.
    *
    * @return Temperature in degree Celsius.
    */
    virtual double getTemperature() = 0;
        
    /**
    * Measures the humidity within the implant capsule. This is a blocking call. It will stop running measurements.
    *
    * @return Humidity in %rh.
    */
    virtual double getHumidity() = 0;

    /**
    * Checks a stimulation command for validity (checks if it could be used in enqueueStimulationCommand
    * without corrections).
    *
    * @param[in] cmd The stimulation command to be checked.
    * @param[out] message An error message telling why the command is not valid, or an empty string if 
    *                     the command is valid.
    *
    * @return True if the command is valid and false otherwise.
    */
    virtual bool isStimulationCommandValid(const IStimulationCommand* cmd, std::string* message) = 0;

    /**
    * Sends the stimulation functions of a given stimulation command to the implant. The stimulation behavior depends 
    * on the selected stimulation mode, @see StimulationMode.
    *
    * A call to this function replaces a previously enqueued stimulation command.
    *
    * @param[in] cmd The stimulation command whose stimulation functions are sent to the implant. The ownership of the
    * command remains with the caller.
    * @param[in] mode The stimulation mode.
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
    * Starts a stimulation. This is a non-blocking call in the sense that it does not wait until the stimulation is done,
    * rather it only waits until an acknowledgment is received from the implant.
    * This start stimulation function can be used exclusively with STIM_MODE_PERSISTENT_FUNC_PRELOADING. It 
    * executes one individual stimulation function specified via a its stimulation function ID.
    *
    * @param[in] stimulationFunctionID The index of the stimulation function to be executed.
    *
    * \if INTERN_IMPLANTAPI
    *   @throws CInvalidArgumentException if the selected stimulation mode is not compatible.
    *   @throws CRuntimeException if stimulation command cannot be sent to implant, e.g. because it is currently
    *                             disconnected.
    * \else
    *   @throws std::exception if the selected stimulation mode is not compatible.
    *   @throws std::exception if stimulation command cannot be sent to implant, e.g. because it is currently disconnected.
    * \endif
    */
    virtual void startStimulation(const uint8_t stimulationFunctionID) = 0;

    /**
    * Starts a stimulation. This is a non-blocking call in the sense that it does not wait until the stimulation is done,
    * rather it only waits until an acknowledgment is received from the implant.
    * This start stimulation function is compatible with STIM_MODE_VOLATILE_CMD_PRELOADING and 
    * STIM_MODE_PERSISTENT_CMD_PRELOADING modes.
    * All enqueued stimulation functions are executed in the order in which they are specified in the stimulation command,
    * and repeated as specified in the stimulation command repetitions.
    *
    * \if INTERN_IMPLANTAPI
    *   @throws CInvalidArgumentException if the selected stimulation mode is not compatible.
    *   @throws CRuntimeException if stimulation command cannot be sent to implant, e.g. because it is currently  
    *                             disconnected.
    * \else
    *   @throws std::exception if the selected stimulation mode is not compatible.
    *   @throws std::exception if stimulation command cannot be sent to implant, e.g. because it is currently disconnected.
    * \endif
    */
    virtual void startStimulation() = 0;

    /**
    * Stops an ongoing stimulation. In stimulation mode STIM_MODE_VOLATILE_CMD_PRELOADING, the stimulation function queue
    * on the implant is cleared. In the other modes, the stimulation functions are still available on the implant for the
    * next call to startStimulation.
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
    * Enable or disable the power transfer to the implant (enabled by default). Note that powering on the
    * implant takes some milliseconds before it is responsive.
    *
    * @param[in] enabled If true, the power transfer to the implant is initiated. If false, it is stopped.
    *
    * \if INTERN_IMPLANTAPI
    *   @throws CTimeoutException if the external unit does not respond before timeout.
    *   @throws CRuntimeException if the request fails.
    * \else
    *   @throws std::exception if the external unit does not respond before timeout.
    *   @throws std::exception if the request fails.
    * \endif
    */
    virtual void setImplantPower(const bool enabled) = 0;

    /**
    * Tells the implant to propagate its states once to the registered listeners. This used to synchronize the
    * state of the caller's implant model with the system's actual state.
    * The implant power state and radio quality events packets are updated with a 1-second period.
    * 
    * Usage:
    * 1. Register listeners.
    * 2. Call pushState().
    * 3. Update GUI or your logging based on the callbacks from the listeners 
    *
    * \if INTERN_IMPLANTAPI
    *   @throws CTimeoutException if the implant does not respond before timeout.
    *   @throws CRuntimeException if the request fails.
    * \else
    *   @throws std::exception if the implant does not respond before timeout.
    *   @throws std::exception if the request fails.
    * \endif
    */
    virtual void pushState() = 0;

    /**
    * Enables or disables the forwarding of the radio connection quality information. Quality updates are only
    * propagated during implant idle mode (no measurement, stimulation or self-tests active). Updates are received 
    * with a 1-second period.
    * Note that function @relates pushState will also activate radio quality forwarding.
    * 
    * @param[in] enable Starts radio connection quality forwarding if true, stops it otherwise.
    */
    virtual void setRFQualityPolling(const bool enable) = 0;

    /**
    * \if INTERN_IMPLANTAPI
    * 
    *   Starts measurement of data but uses all channels as reference.
    *   Compared to the normal measurement mode, this mode enforces the conductivity between the reference input and
    *   the measurement input for each channel of the recorder.
    *   The measurement results for all channels shall be zero in case of no recorder noise.
    * 
    *   This measurement mode:
    *       - Can be stopped by calling stopMeasurement.
    *       - Does not allow to run a stimulation while running. 
    *       - Always uses an amplification factor of 57.5 dB.
    *       - Always uses the ground electrode while measuring.
    *
    *   @throws CRuntimeException if the measurement is started already.
    * 
    * \else
    *   For internal use only (maintenance interface).
    * \endif
    */
    virtual void startNoiseMeasurement() = 0;
};

}}


#endif //IMPLANTAPI_IIMPLANT_H