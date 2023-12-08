/*
eGates 2.1 - by Mike
Easily create automatic or command-based gates with ONE function!

Function: CreateAutomaticGate or CreateAutoGate
Description: This function creates a gate that automatically opens and closes when a player approaches it.


Function: CreateSemiAutoGate
Description: Create a gate that must be manually opened by the script (for example; when a
player honks their horn or types a command) that automatically closes when they
get far enough away

Function: CreateManualGate or CreateGate
- Create a gate that must be 100% controlled by the script


### 'INSTALLATION' ###

Place this .inc file in your /pawno/include or /qawno/include folder.
You will also need foreach and y_hooks, which can be downloaded here:

foreach:                    http://forum.sa-mp.com/showthread.php?t=92679
y_hooks is part of YSI:     http://forum.sa-mp.com/showthread.php?p=1696956

Put all the .inc files in \pawno\include.

If you get an error on compile saying 'can not read from file' then you did this
step wrong.
If you are 100% certain the files are in your \pawno\include folder, you may
have multiple instances of pawno. If so, you may want to delete the bad one.
Search your PC.

### USAGE ###

Instructions will be here



### CHANGELOG ###
    2.1 (July 2013):
    - Gates now only trigger if the player is level with the gate (Z height checks)
    
    2.0 (May 2013):
    - Added semi-auto and manual gates.
    - Added 'playsound' parameter to gate creation functions to let eGates
    automatically handle the playing of sound effects when a gate opens/closes.
    (Defaults to false)
    - Small fixes and improvements.
    - Better naming of variables and functions.
    
    
    1.0 (7th of July 2012):
    - Initial release

### CREDITS ###

Mike - Creation
Y_Less - foreach and y_hooks

===============================================================================

There is only one function - CreateAutomaticGate (or alternatively, CreateAutoGate):

CreateAutoGate(modelid, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz, Float:x2, Float:y2, Float:z2, Float:rx2, Float:ry2, Float:rz2, Float:trigx, Float:trigy, Float:trigz, Float:trig_range, Float:movespeed, bool:auto_open=true, bool:auto_close=true, bool:condition=false, bool:playsound=true);

    Parameters:
    modelid - The object model to use for the gate
    x y z - The position of the CLOSED gate
    rx ry rz - The rotation of the CLOSED gate
    x2 y2 z2 - The position of the OPEN gate
    rx2 ry2 rz2 - The rotation of the OPEN gate
    Set ALL to -1000 to NOT change rotation
    trigx trigy trigz - The trigger point. Players in range of this point will
                        cause the gate to open (or close if nobody in range)
    trig_range - The range from the trigger point (trigx trigy trigz) at which
                 players will cause the gate to open (or close if not in range)
    movespeed - The movement speed for the object. The differs drastically for
                rotating and sliding gates. See tutorial below
    condition - If condition is set to true, you must return true in the the
                OnOnPlayerRequestGate callback for the player to pass,
                there you can check if the player meets a certain condition,
                for example you may only want cops to be able to trigger the
                gate to open. If set to false the gate will open for anyone.
                Do note however, someone that is NOT a cop could still get in
                after a cop triggers it to open.

    Returns:
    The ID of the gate that was created (NOTE: NOT the object ID!)
    -1 if the gate wasn't created (limit reached)

===============================================================================

To force a gate to open:

OpenGate(gateid);

    Parameters:
    gateid - The ID of the gate. Returned by CreateAutomaticGate().

    Returns:
    -1 - Gate doesn't exist
    0- Gate already open(ing)
    1 - Gate opening - success

===============================================================================

To force a gate to close:

CloseGate(gateid);

    Parameters:
    gateid - The ID of the gate. Returned by CreateAutomaticGate().

    Returns:
    -1 - Gate doesn't exist
    0- Gate already close(ing)
    1 - Gate closing - success

===============================================================================

To check the state of a gate (open/closed/opening/closing):

GateStatus(gateid);

    Parameters:
    gateid - The ID of the gate. Returned by CreateAutomaticGate().

    Returns:
    -1 - Gate doesn't exist
    GATE_STATUS_CLOSED 0 - Gate is fully closed
    GATE_STATUS_OPENING 1 - Gate is opening, not yet fully open
    GATE_STATUS_OPEN 2 - Gate is fully open
    GATE_STATUS_CLOSING 3 - Gate is closing, not yet fully closed

===============================================================================

To remove/delete/destroy a gate:

DestroyGate(gateid);
or
DeleteGate(gateid);
or
RemoveGate(gateid);

    Parameters:
    gateid -
    The ID of the gate to destroy/delete. Returned by CreateAutomaticGate().

    Returns:
    -1 - Invalid gate ID
    0 - Gate doesn't exist
    1 - Gate was destroyed successfully

===============================================================================

*/

/*     SETUP / CONFIG      */

// Number of gates that can be created by the script.
#define MAX_GATES 100

// The interval at which the gates are checked for players in range (in MS)
#define TRIGGER_INTERVAL 800 // Radius check in miliseconds (1000 = 1 second)

/*  END OF SETUP / CONFIG  */



// SCRIPT STARTS.
// FEEL FREE TO BROWSE THE CODE TO LEARN
// IT IS WELL COMMENTED FOR THIS EXACT PURPOSE.

#include <YSI_Coding\y_hooks>
#include <YSI_Data\y_iterate>

#include <streamer>
#if defined CreateDynamicObject
#define USE_INCOGNITO
#endif

/* FAKE NATIVES */
/*
native CreateAutomaticGate(modelid, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz, Float:x2, Float:y2, Float:z2, Float:rx2, Float:ry2, Float:rz2, Float:trigx, Float:trigy, Float:trigz, Float:trig_range, Float:movespeed, bool:condition=false);

native CreateAutoGate(modelid, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz, Float:x2, Float:y2, Float:z2, Float:rx2, Float:ry2, Float:rz2, Float:trigx, Float:trigy, Float:trigz, Float:trig_range, Float:movespeed, bool:condition=false);

native OpenGate(gateid);
native CloseGate(gateid);
native GateStatus(gateid);
native IsValidGateID(gateid);
native DeleteGate(gateid);
native DestroyGate(gateid);
native RemoveGate(gateid);

*/

forward OnGateStateChange(gateid, newstate, Float:gatex, Float:gatey, Float:gatez);
// OnGateStateChange is called when a gate's open/closed state changes.
//  GATE_STATUS_CLOSED 0 - Gate is fully closed
//  GATE_STATUS_OPENING 1 - Gate is opening, not yet fully open
//  GATE_STATUS_OPEN 2 - Gate is fully open
//  GATE_STATUS_CLOSING 3 - Gate is closing, not yet fully closed

forward OnPlayerRequestGate(playerid, gateid);
// OnPlayerRequestGate is called when a gate is triggered if 'condition' was set to true in CreateAutomaticGate.
// Returning 0 in this callback will prevent the gate from opening, so you can check for stuff
// For example you may only want cops to access a gate.

forward OnPlayerTriggerGate(playerid, gateid);


// Gate States
#define GATE_STATUS_CLOSED 0
#define GATE_STATUS_OPENING 1
#define GATE_STATUS_OPEN 2
#define GATE_STATUS_CLOSING 3

// Gate Types
#define MGATE_TYPE_FULL_AUTO 0
#define MGATE_TYPE_
#define MGATE_TYPE_
#define MGATE_TYPE_FU

enum E_MGATE_DATA
{
    mgate_type, // Type of gate (defined above)
    mgate_objectid, // The internal objectid for the gate
    mgate_model, // The object model of the gate
    Float:mgate_closed_pos[6], // The gate's pos and rot when closed
    Float:mgate_open_pos[6], // The gate's pos and rot when
    Float:mgate_trigger[3], // The trigger point
    Float:mgate_trigger_radius, // The radius from the trigger point at which the gate is triggered to open
    Float:mgate_movespeed, // The movement speed for the object
    bool:mgate_condition, // if 1 the gate will not open unless 'return 1;' is used in the OnPlayerRequestGate() callback
    mgate_status
}

new g_GateData[MAX_GATES][E_MGATE_DATA];

new Iterator:eGates<MAX_GATES>;

// The ID for the timer. Killed if filterscript is unloaded (if a FS)
new mgate_trigger_TIMERID;



/*   ## FUNCTIONS ##   */

#define CreateAutoGate CreateAutomaticGate

stock CreateAutomaticGate(modelid, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz, Float:x2, Float:y2, Float:z2, Float:rx2, Float:ry2, Float:rz2, Float:trigx, Float:trigy, Float:trigz, Float:trig_range, Float:movespeed, bool:condition=false)
{
    if(Iter_Count(eGates) == MAX_GATES) // Gate limit (MAX_GATES) reached
    {
        // Alert the scripter
        printf("\
        [eGates] ERROR: Gate limit (#MAX_GATES) reached. Could not create additional gate.\n\
        Increase MAX_GATES define in eGates.inc to increase limit.\
        ");
        
        // Return -1 so action could be taken in the script
        // (0 would be a valid gate ID, so -1 must be used)
        return -1;
    }

    new i = Iter_Free(eGates); // Get the free ID in the iterator


    // Create the object
    #if defined USE_INCOGNITO
        g_GateData[i][mgate_objectid] = CreateDynamicObject(modelid, x, y, z, rx, ry, rz);
    #else
        g_GateData[i][mgate_objectid] = CreateObject(modelid, x, y, z, rx, ry, rz, 300.0);
    #endif
    
    // Save all the data
    
    //NModel
    g_GateData[i][mgate_model] = modelid;
    
    // Closed Position
    g_GateData[i][mgate_closed_pos][0] = x;
    g_GateData[i][mgate_closed_pos][1] = y;
    g_GateData[i][mgate_closed_pos][2] = z;
    
    // Closed Rotation
    g_GateData[i][mgate_closed_pos][3] = rx;
    g_GateData[i][mgate_closed_pos][4] = ry;
    g_GateData[i][mgate_closed_pos][5] = rz;
    
    // Open Position
    g_GateData[i][mgate_open_pos][0] = x2;
    g_GateData[i][mgate_open_pos][1] = y2;
    g_GateData[i][mgate_open_pos][2] = z2;
    
    // Open Rotation
    g_GateData[i][mgate_open_pos][3] = rx2;
    g_GateData[i][mgate_open_pos][4] = ry2;
    g_GateData[i][mgate_open_pos][5] = rz2;
    
    // Trigger Point
    g_GateData[i][mgate_trigger][0] = trigx;
    g_GateData[i][mgate_trigger][1] = trigy;
    g_GateData[i][mgate_trigger][2] = trigz;
    
    // Trigger Radius
    g_GateData[i][mgate_trigger_radius] = trig_range;
    
    // Move Speed
    g_GateData[i][mgate_movespeed] = movespeed;
    
    // Condition?
    g_GateData[i][mgate_condition] = condition;
    
    // Starting status - closed of course
    g_GateData[i][mgate_status] = GATE_STATUS_CLOSED;
    
    // Add the gate to the iterator
    Iter_Add(eGates, i);
    
    // The function will return the gate ID
    // (not object ID, the internal gate ID to be used in other functions
    // and callbacks throughout the script)
    return i;
}

#define IsValidGateID(%0) (%0 >= 0 && %0 < MAX_GATES)
/*
Macro Substitution:
#define IsValidGateID(69) (69 >= 0 && 69 < MAX_GATES)

Breakdown:
If 69 is more than or equal to 0, and less than MAX_GATES
If 69 is between 0 and MAX_GATES
if %0 is between 0 and MAX_GATES
*/

stock OpenGate(gateid)
{
    if(!IsValidGateID(gateid)) return 0;
    if(Iter_Contains(eGates, gateid) == false) return 0; // Doesn't exist

    if(g_GateData[gateid][mgate_status] == GATE_STATUS_OPEN || g_GateData[gateid][mgate_status] == GATE_STATUS_OPENING) return 0; // Already open

    // Move the object to the open position
    #if defined USE_INCOGNITO
        MoveDynamicObject(
        g_GateData[gateid][mgate_objectid], // Model
        g_GateData[gateid][mgate_open_pos][0], g_GateData[gateid][mgate_open_pos][1], g_GateData[gateid][mgate_open_pos][2], // X, Y, Z
        g_GateData[gateid][mgate_movespeed], // Speed
        g_GateData[gateid][mgate_open_pos][3], g_GateData[gateid][mgate_open_pos][4], g_GateData[gateid][mgate_open_pos][5] // RX, RY, RZ
        );
    #else
        MoveObject(
        g_GateData[gateid][mgate_objectid], // Model
        g_GateData[gateid][mgate_open_pos][0], g_GateData[gateid][mgate_open_pos][1], g_GateData[gateid][mgate_open_pos][2], // X, Y, Z
        g_GateData[gateid][mgate_movespeed], // Speed
        g_GateData[gateid][mgate_open_pos][3], g_GateData[gateid][mgate_open_pos][4], g_GateData[gateid][mgate_open_pos][5] // RX, RY, RZ
        );
    #endif
    
    // Set the status to 'opening'
    g_GateData[gateid][mgate_status] = GATE_STATUS_OPENING;

    // Trigger OnGateStateChange
    CallLocalFunction("OnGateStateChange", "iifff", gateid, GATE_STATUS_OPENING, g_GateData[gateid][mgate_closed_pos][0], g_GateData[gateid][mgate_closed_pos][1], g_GateData[gateid][mgate_closed_pos][2]);
    return 1;
}

stock CloseGate(gateid)
{
    if(!IsValidGateID(gateid)) return 0;
    if(Iter_Contains(eGates, gateid) == false) return 0; // Doesn't exist

    if(g_GateData[gateid][mgate_status] == GATE_STATUS_CLOSED || g_GateData[gateid][mgate_status] == GATE_STATUS_CLOSING) return 0; // Already closed

    // Move the object to the closed position
    #if defined USE_INCOGNITO
        MoveDynamicObject(
        g_GateData[gateid][mgate_objectid],
        g_GateData[gateid][mgate_closed_pos][0], g_GateData[gateid][mgate_closed_pos][1], g_GateData[gateid][mgate_closed_pos][2], // X, Y, Z
        g_GateData[gateid][mgate_movespeed], // Speed
        g_GateData[gateid][mgate_closed_pos][3], g_GateData[gateid][mgate_closed_pos][4], g_GateData[gateid][mgate_closed_pos][5] // RX, RY, RZ
        );
    #else
        MoveObject(
        g_GateData[gateid][mgate_objectid],
        g_GateData[gateid][mgate_closed_pos][0], g_GateData[gateid][mgate_closed_pos][1], g_GateData[gateid][mgate_closed_pos][2], // X, Y, Z
        g_GateData[gateid][mgate_movespeed], // Speed
        g_GateData[gateid][mgate_closed_pos][3], g_GateData[gateid][mgate_closed_pos][4], g_GateData[gateid][mgate_closed_pos][5] // RX, RY, RZ
        );
    #endif

    // Set the status to 'closing'
    g_GateData[gateid][mgate_status] = GATE_STATUS_CLOSING;

        // Trigger OnGateStateChange
    CallLocalFunction("OnGateStateChange", "iifff", gateid, GATE_STATUS_CLOSING, g_GateData[gateid][mgate_open_pos][0], g_GateData[gateid][mgate_open_pos][1], g_GateData[gateid][mgate_open_pos][2]);
    return 1;
}

#define DeleteGate DestroyGate // alternative function name (alias)
#define RemoveGate DestroyGate // alternative function name (alias)
stock DestroyGate(gateid)
{
    if(!IsValidGateID(gateid)) return 0;
    if(Iter_Contains(eGates, gateid) == 0) return 0; // Doesn't exist

    #if defined USE_INCOGNITO
        DestroyDynamicObject(g_GateData[gateid][mgate_objectid]);
    #else
        DestroyObject(g_GateData[gateid][mgate_objectid]);
    #endif
    
    Iter_Remove(eGates, gateid);
    return 1;
}

stock GateStatus(gateid)
{
    if(!IsValidGateID(gateid)) return -1;
    if(Iter_Contains(eGates, gateid) == 0) return -1; // Doesn't exist
    // -1 must be used as 0 is a valid return value (GATE_STATUS_CLOSED)
    
    return g_GateData[igateidgate_status];
}

/*   ## TIMER ##   */

forward TIMER_mGateChecks();
public TIMER_mGateChecks()
{
    new count;
    
    // Loop through all gates (iterator)
    foreach(new i : eGates)
    {
        if(g_GateData[i][mgate_status] == GATE_STATUS_CLOSED || g_GateData[i][mgate_status] == GATE_STATUS_CLOSING) // Gate is closed/closing, check if any players are near and open if so
        {
            // Loop through players
            foreach(new playerid : Player)
            {
                if(mGate_CheckProx(playerid, i)) // if in range of the gate
                {
                    if(g_GateData[i][mgate_condition] == true) // Must pass condition!
                    {
                        if(CallLocalFunction("OnPlayerRequestGate", "ii", playerid, i) == 1)
                        {
                            OpenGate(i); // Open the gate
                            CallLocalFunction("OnPlayerTriggerGate", "ii", playerid, i);
                            break; // Break out of loop, we know at least one player is in the trigger radius, no need to continue to check for more
                        }
                    }
                    else // No condition required
                    {
                        OpenGate(i); // Open the gate
                        CallLocalFunction("OnPlayerTriggerGate", "ii", playerid, i);
                        break; // Break out of loop, we know at least one player is in the trigger radius, no need to continue to check for more
                    }
                }
            }
        }
        else if(g_GateData[i][mgate_status] == GATE_STATUS_OPEN) // Gate is fully open, check if there are no players near trigger zone and close if so
        {
            foreach(new playerid : Player)
            {
                if(mGate_CheckProx(playerid, i)) // if in range of the gate
                {
                    if(g_GateData[i][mgate_condition] == true) // Must pass condition!
                    {
                        if(CallLocalFunction("OnPlayerRequestGate", "ii", playerid, i) == 1)
                        {
                            count = 1;
                            break; // Break out of loop, we know at least one player is in the trigger radius, no need to continue to check for more
                        }
                    }
                    else
                    {
                        count = 1;
                        break; // Break out of loop, we know at least one player is in the trigger radius, no need to continue to check for more
                    }
                }
            }
            if(!count) CloseGate(i);
        }
    }
    return 1;
}



/*   ## HOOKS ##   */

hook OnGameModeInit()
{
    // Start main timer
    mgate_trigger_TIMERID = SetTimer("TIMER_mGateChecks", TRIGGER_INTERVAL, true);
    return 1;
}

hook OnFilterScriptInit()
{
    // Start main timer
    mgate_trigger_TIMERID = SetTimer("TIMER_mGateChecks", TRIGGER_INTERVAL, true);
    return 1;
}

hook OnFilterScriptExit()
{
    // Kill main timer
    KillTimer(mgate_trigger_TIMERID);
    return 1;
}

// Timer doesn't need to be killed ongamemodeexit

#if defined USE_INCOGNITO
public OnDynamicObjectMoved(objectid)
#else
hook OnObjectMoved(objectid)
#endif
{
    // Check if it was a gate that moved
    foreach(new i : eGates)
    {
        if(g_GateData[i][mgate_model] != 0 && g_GateData[i][mgate_objectid] == objectid) // We found our gate.
        {
            if(g_GateData[i][mgate_status] == GATE_STATUS_CLOSING)
            {
                g_GateData[i][mgate_status] = GATE_STATUS_CLOSED;
                CallLocalFunction("OnGateStateChange", "iifff", i, GATE_STATUS_CLOSED, g_GateData[i][mgate_closed_pos][0], g_GateData[i][mgate_closed_pos][1], g_GateData[i][mgate_closed_pos][2]);
            }
            else
            {
                g_GateData[i][mgate_status] = GATE_STATUS_OPEN;
                CallLocalFunction("OnGateStateChange", "iifff", i, GATE_STATUS_OPEN, g_GateData[i][mgate_open_pos][0], g_GateData[i][mgate_open_pos][1], g_GateData[i][mgate_open_pos][2]);
            }
        }
    }
    #if defined USE_INCOGNITO
        #if defined MGATE_OnDynamicObjectMoved
            return MGATE_OnDynamicObjectMoved(objectid);
        #else
            return 1;
        #endif
    #else
        return 1;
    #endif
}
#if defined USE_INCOGNITO
    #if defined MGATE_OnDynamicObjectMoved
        forward MGATE_OnDynamicObjectMoved(objectid);
    #endif
    #if defined _ALS_OnDynamicObjectMoved
        #undef OnDynamicObjectMoved
    #else
        #define _ALS_OnDynamicObjectMoved
    #endif
    #define OnDynamicObjectMoved MGATE_OnDynamicObjectMoved
#endif

static mGate_CheckProx(playerid, i) // i = gate ID
{
    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    return (IsPlayerInRangeOfPoint(playerid, g_GateData[i][mgate_trigger_radius], g_GateData[i][mgate_trigger][0], g_GateData[i][mgate_trigger][1], g_GateData[i][mgate_trigger][2]) && (z-2 < g_GateData[i][mgate_trigger][2] < z+2));
}