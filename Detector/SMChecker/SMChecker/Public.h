/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that app can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_SMChecker,
    0x74c3080e,0x02bc,0x4838,0x81,0x4e,0x98,0x6e,0x1f,0xa8,0x65,0x3e);
// {74c3080e-02bc-4838-814e-986e1fa8653e}
