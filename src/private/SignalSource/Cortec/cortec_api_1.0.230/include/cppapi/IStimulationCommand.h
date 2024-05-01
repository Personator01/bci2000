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
#ifndef IMPLANTAPI_ISTIMULATION_COMMAND_H
#define IMPLANTAPI_ISTIMULATION_COMMAND_H
#include "cppapi/IIterator.h"
#include "cppapi/IStimulationFunction.h"
#include <stdint.h>


namespace cortec { namespace implantapi {

/**
* @brief Interface for classes that can be sent to an implant to elicit electrical stimulation.
*
* This is a generic interface that is intended to be used with different types of implants with different 
* stimulation capabilities each. Stimulation capabilities may vary regarding the number of channels used for 
* stimulation, stimulation amplitude, form of the stimulation signal over time. To reflect this, a stimulation
* command consists of a sequence of stimulation functions. A stimulation function can be composed arbitrarily
* complex.
*
* The API currently supports three types of stimulation modes on the implant: The volatile command preloading,
* the persistent command preloading and the persistent function preloading. The user is required to assign
* the type of preloading mode when calling the enqueue function. See @ref StimulationMode for mode details.
* 
* - Volatile Command Preloading
* 
*   In the volatile command preloading mode, the stimulation functions are deleted when the execution is finished. 
*   This implies that an enqueued stimulation command can only be started once in this mode.
* - Persistent Command Preloading
* 
*   On the contrary, the stimulation functions of the command are not deleted from the implant queue when using the
*   persistent modes. Starting the stimulation in the command preloading mode will execute all enqueued 
*   stimulation functions after each other until the command is finished (including repetitions). However, the persistent
*   command preloading mode has the constraint, that the number of enqueued stimulation functions must not exceed 16.
* 
* - Persistent Function Preloading
* 
*   In the persistent function preloading mode, single stimulation functions from the enqueued stimulation command are executed. 
*   The enqueued stimulation functions are not deleted from the implant after the stimulation. When starting a stimulation, 
*   the user is required to pick a stimulation function to stimulate through the stimulation function ID. The stimulation function 
*   IDs are auto generated in range [1, number of enqueue stimulation functions], 
*   with the first stimulation function corresponding to the ID 1. Note that the stimulation functions IDs are not the indices of the functions.
*   The start stimulation call will stimulate only the stimulation function picked with the ID. The persistent function preloading mode 
*   has the same constraint as the persistent command preloading mode, that the size of the stimulation command must not exceed 16. 
*   Note that a command repetition > 1 has no effect in the persistent function preloading mode.
*
* Each command holds a tracing id that can be set arbitrarily. This id is used to identify executions of the
* command in the application logs.
*
* The execution of a command can be repeated by setting a number of (command) repetitions > 1. This number of
* command repetitions differs from the stimulation function repetitions (burst and pulse repetitions), since all 
* functions in the command are repeated.
*
* Example: Given a command that contains stimulation functions A, B and C that shall execute after one another, 
* and a command repetition number of 3. Then, the command functions will be executed as follows:
*
* A | B | C |  A | B | C | A | B | C
*
* If the pulse repetitions of stimulation function A is additionally set to 2, the execution changes to: 
*
* A | A | B | C | A | A | B | C | A | A | B | C
*
* Typical usage of an IStimulationCommand:
* 1. Create an empty stimulation command instance via the IStimulationCommandFactory instance.
* 2. Set an id for tracing in the logs.
* 3. Repeatedly add IStimulationFunction instances.
* 4. Send IStimulationCommand instance to implant by calling 
*    IImplant::enqueueStimulationCommand(command, preloading_mode).
*
* @see IImplant
* @see IStimulationFunction
*/
class IStimulationCommand
{
public:
    virtual ~IStimulationCommand() {}

    /**
    * Append a stimulation function. The sequence of appends defines the sequence of execution.
    * The ownership of IStimulationFunction is passed to the IStimulationCommand if this method call succeeds 
    * (i.e., does not throw any exception).
    *
    * \if INTERN_IMPLANTAPI
    *     @throws CInvalidArgumentException if duration is 0 microseconds or function is a nullptr.
    * \else
    *     @throws std::exception if duration is 0 or function is a nullptr.
    * \endif
    */
    virtual void append(IStimulationFunction* function) = 0;

    /**
    * Return iterator that can be used to iterate through all functions currently contained in the 
    * IStimulationCommand. It is aware neither of function repetitions nor of command repetitions.
    *
    * The ownership of the iterator is passed to the caller.
    *
    * @return Iterator pointing to the first IStimulationFunction of the IStimulationCommand.
    */
    virtual IIterator<IStimulationFunction>* getFunctionIterator() const = 0;

    /**
    * Return iterator that can be used to iterate through all functions currently contained in the
    * IStimulationCommand and is aware of command repetitions (but not of function repetitions!).
    *
    * The ownership of the iterator is passed to the caller.
    *
    * @return Command repetition aware iterator pointing to the first IStimulationFunction.
    */
    virtual IIterator<IStimulationFunction>* getCommandRepetitionAwareFunctionIterator() const = 0;

    /**
    * Return iterator that can be used to iterate through all functions currently contained in the
    * IStimulationCommand and is aware of function repetitions (but not of command repetitions!).
    *
    * For example, if a function has n pulse repetitions, then the iterator will return the next stimulation 
    * function after n calls to getNext().
    *
    * The ownership of the iterator is passed to the caller.
    * 
    * @return Function repetition aware iterator pointing to the first IStimulationFunction.
    */
    virtual IIterator<IStimulationFunction>* getRepetitionAwareFunctionIterator() const = 0;

    /**
    * @return The total duration of the stimulation command in microseconds. Is aware of all repetitions.
    */
    virtual uint64_t getDuration() const = 0;

    /**
    * Set the name of the command.
    */
    virtual void setName(const std::string& commandName) = 0;

    /**
    * @return The set name of the command. If the command name was not set, empty string is returned.
    */
    virtual std::string getName() const = 0;

    /**
    * Return a deep copy of the command. Caller is responsible for deletion of the cloned command.
    *
    * @return An identical deep copy of the command.
    */
    virtual IStimulationCommand* clone() const = 0;

    /**
    * @return The number of stimulation functions and pauses contained in the command.The given size is only
    * aware of command repetitions but not of function repetitions.
    */
    virtual uint64_t getSize() const = 0;

    /**
    * @return The command's tracing id.
    */
    virtual uint16_t getTracingId() const = 0;

    /**
    * Set the command's tracing id.
    *
    * @param[in] id The tracing id.
    */
    virtual void setTracingId(const uint16_t id) = 0;

    /**
    * @return The number of command repetitions.
    */
    virtual uint16_t getRepetitions() const = 0;

    /**
    * Set the number of times that the command (all functions in the command) should be executed,
    * i.e. if the repetitions value is set to 1, the command is executed once.
    *
    * @param[in] repetitions The number of repetitions.
    */
    virtual void setRepetitions(const uint16_t repetitions) = 0;

    /**
    * Type of the stimulation command fault.
    */
    enum class FaultType
    {
        FT_WARNING = 0,
        FT_ERROR,
        FT_COUNT // number of types
    };

    /**
    * Struct describing a fixable fault in a stimulation command.
    * 
    * Contains the index of the faulty function, the fault criticality,
    * source attribute and repaired value.
    */
    struct StimulationCommandFault 
    {
        uint64_t functionIndex;
        FaultType criticality;
        IStimulationFunction::PulseAttributes attribute;
        int32_t sanitizedValue;
    };

};
}}


#endif //IMPLANTAPI_ISTIMULATION_COMMAND_H