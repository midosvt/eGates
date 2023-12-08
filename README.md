# eGates
eGates include is designed to make the process of creating automatic gates EXTREMELY easy with just a single line of code. It's an edit of the original mGates include created by the developer of the Mike's Cops and Robbers (Mike/MP2) SA-MP server back in 2012, and now we want to share it with others who may find it useful. It also has an option for you to deny access for certain players based on things such as their team. For example, if you wish to create a gate that exclusively opens for the Police personnel, it's incredibly simple to achieve with this feature-rich include. Enjoy the ease and flexibility of implementing automatic gates in your server with the Automatic Gate Creator!

### Preview
<div align="left">
      <a href="https://www.youtube.com/watch?v=CH-zX4qEgzg">
         <img src="https://img.youtube.com/vi/CH-zX4qEgzg/0.jpg" style="width:50%;">
      </a>
</div>

## Functions

To create an automatic gate (opens when players are near)

```pawn
CreateAutomaticGate(modelid, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz, Float:x2, Float:y2, Float:z2, Float:rx2, Float:ry2, Float:rz2, Float:trigx, Float:trigy, Float:trigz, Float:trig_range, Float:movespeed, condition=0)
```

### Parameters

`modelid` is the object model to use for the gate

`x y z` is the position of the CLOSED gate

`rx ry rz` is the rotation of the CLOSED gate

`x2 y2 z2` is the position of the OPEN gate

`rx2 ry2 rz2` is the rotation of the OPEN gate
**Set ALL to -1000 or the same as rx, ry, rz to NOT change rotation**

`trigx trigy trigz` is the trigger point. Players in range of this point will
cause the gate to open (or close if nobody in range)

`trig_range` is The range from the trigger point (trigx trigy trigz) at which
players will cause the gate to open (or close if not in range)

`movespeed` is The movement speed for the object. The differs drastically for
rotating and sliding gates.

`condition` If condition is set to 1, you must return 1 in the the
`OnPlayerRequestGate` callback for the player to pass,
there you can check if the player meets a certain condition,
for example you may only want cops to be able to trigger the
gate to open. If set to 0 the gate will open for anyone.
Do note however, someone that is NOT a cop could still get in
after a cop triggers it to open.

This function has many parameters. Typing `CreateAutomaticGate` in pawno will pop-up a syntax box that will update in real-time as you specify parameters. In order for this to work you must include the .inc file and re-open pawno.

### Returns
The ID of the gate that was created (NOTE: NOT the object ID!)
`-1` if the gate wasn't created (limit reached)

### Usage

Example: Create a barrier at the LSPD that will have a condition (cops only):

```pawn
new LSPD_BARRIER;

public OnGameModeInit()
{
    // Los Santos Department Barrier
    LSPD_BARRIER = CreateAutomaticGate(
    968,                                // --> Model
    1544.69, -1630.8739, 13.0728,       // --> Closed Position
    0.0, 90.0, 90.0,                    // --> Closed Rotation
    1544.69, -1630.8739, 13.0728+0.001, // --> Open Position
    0.0, 00.0, 90.0,                    // --> Open Rotation
    1542.87, -1627.0, 13.37,            // --> Trigger Point Position
    20.0,                               // --> Trigger Point Radius
    0.001,                              // --> Object move speed
    true                                // --> Condition? True/False
);
    return 1;
}
```

Setting it out in this way allows for you to easily see, understand and edit a gate quickly, as opposed to having all the parameters on one line.
You should also comment them (//) so you know which is which - as there are quite a few parameters to pass.


To force a gate to open.

```pawn
OpenGate(gateid)
```

### Parameters
`gateid` is the ID of the gate. Returned by CreateAutomaticGate().

### Returns
`-1` - Gate doesn't exist
`0` - Gate already open(ing)
`1` - Gate opening - success!

### Usage

Example of opening a gate using `OpenGate`

```pawn
public OnPlayerCommandText(playerid, cmdtext[])
{
    if(!strcmp(cmdtext, "/LSPD", true))
    {
        OpenGate(LSPD_BARRIER);
        return 1;
    }
    return 0;
}
```

To force a gate to close.

```pawn
CloseGate(gateid)
```

### Parameters
`gateid` is the ID of the gate. Returned by CreateAutomaticGate().

### Returns
`-1` - Gate doesn't exist
`0` - Gate already close(ing)
`1` - Gate closing - success!

### Usage

Example of closing a gate using `CloseGate`

```pawn
public OnPlayerCommandText(playerid, cmdtext[])
{
    if(!strcmp(cmdtext, "/LSPD", true))
    {
        CloseGate(LSPD_BARRIER);
        return 1;
    }
    return 0;
}
```

To check the state of a gate (open/closed/opening/closing).

```pawn
GateStatus(gateid)
```

### Parameters
`gateid` is the ID of the gate. Returned by CreateAutomaticGate().

### Returns
`-1`                     - Gate doesn't exist
`GATE_STATUS_CLOSED 0`   - Gate is fully closed
`GATE_STATUS_OPENING 1`  - Gate is opening, not yet fully open
`GATE_STATUS_OPEN 2`     - Gate is fully open
`GATE_STATUS_CLOSING 3`  - Gate is closing, not yet fully closed

### Usage

Example of using `GateStatus`

```pawn
public OnPlayerCommandText(playerid, cmdtext[])
{
    if(!strcmp(cmdtext, "/LSPD", true))
    {
        if(GateStatus(LSPD_BARRIER) == GATE_STATUS_OPEN || GateStatus(LSPD_BARRIER) == GATE_STATUS_OPENING) return SendClientMessage(playerid, COLOR_RED, "Gate is already open!");
        OpenGate(LSPD_BARRIER);
        return 1;
    }
    return 0;
}
```

To destroy a gate.

```pawn
DestroyGate(gateid)
```

To delete a gate.

```pawn
DeleteGate(gateid)
```

To remove a gate.

```pawn
RemoveGate(gateid)
```

### Parameters
`gateid` is the id of the gate to destroy/delete/remove a gate. Returned by `CreateAutomaticGate()`

### Returns

`-1` - Invalid gate ID
`0` - Gate not found or doesn't exist
`1` - Gate was destroyed successfully

### Usage

Example of destroying a gate using `DestroyGate(gateid)`

```pawn
new BARRIER_LSPD;

public OnGameModeInit()
{
    BARRIER_LSPD = CreateAutomaticGate(...);
    return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
    if(!strcmp(cmdtext, "/LSPD", true))
    {
        DestroyGate(BARRIER_LSPD);
        return 1;
    }
    return 0;
}
```

## Callbacks

```pawn
public OnGateOpenClose(gateid, openclose, Float:gatex, Float:gatey, Float:gatez)
```

OnGateOpenClose is called when a gate opens or closes.
It is called 4 times per gate: when the gate starts to open, when it's fully open, when it starts to close and when it's fully closed again.

`gateid` is the ID of the gate that triggered this callback (returned by CreateAutomaticGate).

`GATE_STATUS_CLOSED 0` - Gate is fully closed
`GATE_STATUS_OPENING 1` - Gate is opening, not yet fully open
`GATE_STATUS_OPEN 2` - Gate is fully open
`GATE_STATUS_CLOSING 3` - Gate is closing, not yet fully closed

`gatex`, `gatey`, and `gatez` are the coordinates of the gate currently (whether opening or closing) - can be used, for example, to play a sound for players near-by


```pawn
public OnPlayerRequestGate(playerid, gateid)
```

The core of the 'condition' system.
Returning true/false in this callback will deny the player access to the gate.
Is it important to note, however, if one player triggers the gate open, it opens for everyone, even those without access. This is unavoidable unless the gate objects are per-player, which would not look or work well.
See the second post below for detailed explanation of condition system.


```pawn
public OnPlayerTriggerGate(playerid, gateid)
```

Called when a player triggers a gate to open.
One use I had for this was playing the 'They'll give a pilots license to anybody these days' sound when players triggered the airport gates to open.
This callback may not be needed, as the code could simply be put under OnPlayerRequestGate, but this makes things easier.