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
#ifndef IMPLANTAPI_ISTIMULATION_COMMAND_FACTORY_H
#define IMPLANTAPI_ISTIMULATION_COMMAND_FACTORY_H
#include "IStimulationCommand.h"
#include "IStimulationFunction.h"
#include "IStimulationAtom.h"
#include <stdint.h>


namespace cortec { namespace implantapi {

    /**
      * @brief Factory interface for creation of stimulation commands.
      *
      * An instance of IStimulationCommandFactory can be used to compose stimulation commands and stimulation functions.
      *
      * Typical usage: 
      * 1. Create empty stimulation command.
      * 2. Create stimulation function. Set repetition parameter and virtual stimulation electrodes.
      * 3. Append stimulation function to command.
      * 4. Repeat steps 2. + 3. until all functions are created.
      *
      * @see IStimulationCommand
      * @see IStimulationFunction
      * @see IStimulationAtom
      */
    class IStimulationCommandFactory
    {
    public:
        virtual ~IStimulationCommandFactory() {}

        /**
          * @return Empty stimulation command. Caller is responsible for deletion of returned pointer.
          */
        virtual IStimulationCommand* createStimulationCommand() = 0;

        /**
          * @return Empty stimulation function with repetition count 1. Caller is responsible for deletion of returned 
          *         pointer.
          */
        virtual IStimulationFunction* createStimulationFunction() = 0;

        /**
         * Creates a stimulation pulse that consists of the following five atoms:
         * Main Pulse: The main part of the stimulation pulse.
         * Dead Zone0: The pause between the main pulse and the counter pulse.
         * Counter Pulse: The counter pulse of the main pulse 
         * Dead Zone0: Identical to previous DZ0
         * Dead Zone1: The pause at the end of a stimulation.
         *  
         * A more detailed description of stimulation pulses can be found in IStimulationFunction.h.
         *
         * @param[in] amplitude   The amplitude of the main pulse, either in V or uA.
         * @param[in] duration    The duration of the main pulse in micro seconds of the stimulation.
         * @param[in] dz0duration The duration of the pause between main and counter pulse in micro seconds.
         * @param[in] dz1duration The duration at the end of the pulse in micro seconds.
         *
         * @return A stimulation function with the specified parameters. Caller is responsible for deletion
         *         of the returned pointer.
         */
        virtual IStimulationFunction* createRect4AmplitudeStimulationFunction(const double amplitude, const uint64_t duration,
            const uint64_t dz0duration, const uint64_t dz1duration) = 0;

        /**
         * Creates a stimulation function that consists of one pause atom of the specified duration. 
         *
         * @param[in] duration The duration of the stimulation pause function in micro seconds. The maximum allowed
         *                     value is 57.600.000 micro seconds.
         * @return A stimulation function with a stimulation pause. Caller is responsible for deletion
         *         of the returned pointer.
         */
        virtual IStimulationFunction* createPauseStimulationFunction(const uint64_t duration) = 0;

        /** 
          * Creates a "rectangular" stimulation atom. Each atom represents a constant amplitude for a fixed time 
          * duration.
          * 
          * Consider an implant that does current stimulation. Then createRectStimulationAtom(12, 2000000) would return
          * a stimulation atom that stimulates for 2 seconds with an amplitude of 12 micro ampere.
          *
          * If this atom would be sent to an implant that does voltage stimulation, this atom would elicit a stimulation
          * with an amplitude of 12 Volts for 2 seconds.
          *
          * @param[in] amplitude    Stimulation amplitude, either in V or uA.
          * @param[in] duration Duration in micros of the stimulation.
          * @return Initialized stimulation atom. Caller is responsible for deletion of returned pointer.
          */
        virtual IStimulationAtom* createRectStimulationAtom(const double amplitude, const uint64_t duration) = 0;

        /**
        * Creates a "rectangular" stimulation atom with four different amplitudes. Each atom represents four constant
        * amplitudes for a fixed time duration.
        *
        * @param[in] amplitude0 First stimulation amplitude, either in V or uA.
        * @param[in] amplitude1 Second stimulation amplitude, either in V or uA.
        * @param[in] amplitude2 Third stimulation amplitude, either in V or uA.
        * @param[in] amplitude3 Fourth stimulation amplitude, either in V or uA.
        * @param[in] duration   Duration in micros of the stimulation.
        * @return Initialized stimulation atom. Caller is responsible for deletion of returned pointer.
        */
        virtual IStimulationAtom* createRect4AmplitudeStimulationAtom(const double amplitude0, const double amplitude1,
            const double amplitude2, const double amplitude3, const uint64_t duration) = 0;

        /**
        * Creates a pause stimulation atom. Each atom represents a pause while stimulating for a fixed time
        * duration.
        *
        * @param[in] duration Duration in micros of the pause.
        * @return Initialized stimulation atom. Caller is responsible for deletion of returned pointer.
        */
        virtual IStimulationAtom* createStimulationPauseAtom(const uint64_t duration) = 0;
    };

}}

#endif //IMPLANTAPI_ISTIMULATION_COMMAND_FACTORY_H