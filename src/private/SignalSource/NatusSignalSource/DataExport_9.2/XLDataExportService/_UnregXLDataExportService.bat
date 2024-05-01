@Echo off
Rem The following should be set to the unmapped drive path if you want the services to start on system bootup

Set NWSrvcList=XLDataExportSrv 
Echo Stopping Services
@Echo on
FOR %%I IN (%NWSrvcList%) DO net stop %%I
@Echo off

Echo XLDataExportSrv stopped.

Echo Unregistering Services

Set NWSrvcList=XLDataExportSrv
@Echo on
FOR %%I IN (%NWSrvcList%) DO sc delete %%I
@Echo off

Echo XLDataExportSrv unregistered.

PAUSE

