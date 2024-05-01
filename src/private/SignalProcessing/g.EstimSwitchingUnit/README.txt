==Location==
http://{{SERVERNAME}}/svn/trunk/src/private/SignalProcessing/g.EstimSwitchingUnit
==Versioning==

===Authors===
Alex Belsten (belsten@neurotechcenter.org)

===Version History===
* 2020/05/22: Initial release for the switching unit prototype


===Source Code Revisions===
*Initial development: 6125
*Tested under: 6
*Known to compile under: 6125
*Broken since: --

==Functional Description==
This filter adds support for the g.tec g.ESTIM PRO Switching unit. This device is used in conjunction with a g.ESTIM device and a g.tec amplifier. It allows for the channels the automatic switching of channels from a recording channels to a stimulating channel. The filter has been developed using:

Estim Switching Unit Prototype
* SN: SR-2019.10.01

* API: 0.1.0537


==Integration into BCI2000==
Compile the filter by enabling compilation of private in your CMake configuration.

==Known Issues== 
===Relience on gEstimProSwitchingUnitApi.lib===
Current implementation relies on gEstimProSwitchingUnitApi.lib to link to switching unit API. This becomes problematic when compiling the module for the first time in release or debug as MCVS won't know where the .lib is and will report linker errors. To fix this error, find gEstimProSwitchingUnit in the solution explorer in MCVS. Then right click on the project and go to Configuration Properties>Linker>Input>Additional Dependencies and click edit. Add the .lib and its path, then recompile. 

This issue has not been resolved because using the output of the dylib tool results in more errors. I believe this is because there are more functions implemented in the gEstimProSwitchingUnitApi.dll than the ones enumeratied in the gEstimProSwitchingUnitApi.h. 

==Parameters==
The ParallelPortFilter is configured in the filtering tab. The configurable parameters are:

===EnableSU===
This parameter enables the switching unit. 

===DeviceID===
Index of device to connect to. Usually 0.

===Trigger===
There are two options for trigger, a software trigger and a digital trigger. 

====Software trigger====
Channel switching is triggered when the StimulusCode state meets the stimulus condition defined in the first row of the ''StimConfig'' parameter. Note: if only one state (one column) is defined in ''StimConfig'', the configuration of channel information will not need to be reuploaded to the device. This results in a dramatic decrease in latency of transition between recording to stimulating.

====Digital Trigger====
Channel switching is triggered by the digital in on the switching unit. To use the digital trigger, only one state (one column) can be defined in the ''StimConfig'' parameter. When the digital trigger occurs the configuration for that one state will be performed. 
  
===StimConfig===
This matrix configures the channel routing. The first row that is labeled by 'StimulusCode' is the condition for the channel routing to occur. When 'StimulusCode' is equal to this condition, the channels will be routed for stimulation. The Stim+ ch and Stim- ch nested lists define what channels should be routed to the positive side of the stimulation input and to the negative side of stimulation input. 

==See also==

[[Category:Contributions]][[Category:SignalProcessing]]
