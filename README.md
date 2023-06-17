# MazEE RunEEr REST API Documentation

`backend` binary to run, prebuilt for Ubuntu 22.04

In case of failure of appropriate request, error message sent under `{message, ""}`

## Build

    docker-compose build

## Run the app

    docker-compose up


# REST API

The REST API to the backend of MazEE RunnEER WebServer is described below.

## Session
This is only for connectivity to the frontend.

### DisplayAll

`GET /Session/DisplayAll`

Reponse: `SessionList`  if success <br> 
Type: `200`
### Ping

`PATCH /Session/Ping`

Params: `SessionId`

Reponse: `SessionList` if success<br> 
Type: `200`

## BugInformation
This is for connectivity to the balance-bug and frontend.

### DisplayAll

`GET /BugInformation/DisplayAll`

Reponse: `BugInformationList` if success<br> 
Type: `200`

### Add

`GET /BugInformation/Add`

Params: `BugId, BugName`

Reponse: `BugInformationList` if success<br> 
Type: `201`

### UpdateBugName

`GET /BugInformation/UpdateBugName`

Params: `BugId, BugName`

Reponse: `BugInformation` if success<br> 
Type: `200`

### Ping

`GET /BugInformation/Ping`

Params: `BugId`

Reponse: `BugInformationList` if success<br> 
Type: `200`

### Online

`GET /BugInformation/Online`

Params: `Timeout`

Reponse: `BugInformationList` if success<br> 
Type: `200`

## Nodes
This is only for connectivity to the balance-bug and frontend.

### DisplayAll

`GET /Nodes/DisplayAll`

Params: `SessionId`

Reponse: `NodeLists` if success<br> 
Type: `200`

### CreateTable

`GET /Nodes/CreateTable`

Params: `BugId`

Reponse: `{"message": "successfully created new table"}` if success<br> 
Type: `201`

### Add

`GET /Nodes/Add`

Params:`BugId, NodeId, XCoord, YCoord`

Reponse: `NodeNew` if success<br> 
Type: `201`

## Edges
This is only for connectivity to the balance-bug and frontend.

### DisplayAll

`GET /Edges/DisplayAll`

Params: `SessionId`

Reponse: `EdgeLists` if success <br> 
Type: `200`

### CreateTable

`GET /Edges/CreateTable`

Params: `BugId`

Reponse: `{"message": "successfully created new table"}` if success<br> 
Type: `201`

### Add

`GET /Edges/Add`

Params: `BugId, NodeId, EdgeNodeId, Distance, Angle`

Reponse: `EdgeNew` if success<br> 
Type: `201`

## Beacon
This is only for connectivity to the beacon and balance-bug.

### Beacon
For use on the balance-bug
`GET /Beacon/TurnOn`

Params: `BugId, BeaconOn`<br> 

Reponse if success:
- `0` if Session associated to BugId doesn't exit
- `1` if ChargeStatus of Beacons is set to 0 (insufficient charge)
- `2` if trying to assert Beacon to current value
- `3` if successfully asserted requestion value
Type: `200`

### BeaconPing
For use on the beacons. Latches onto the latest SessionId Creates

`GET /Beacon/BeaconPing`

Params: `ChargeStatus`

Reponse: `On` if success<br> 
Type: `200`

