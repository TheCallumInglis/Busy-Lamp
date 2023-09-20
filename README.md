# Busy Lamp  
A teams presence indicator for ESP8266.  

Utilise the Microsoft Graph API to fetch user presence information and display it with Neopixels.

## Usage  
`getAvailability()` handles user authentication and returns user's availability as a string. Use the output of this function as you desire.

Possible return values are:  
| Property | Type | Description |
| --- | --- | --- |
| activity | string collection | The supplemental information to a user's availability. Possible values are `Available`, `Away`, `BeRightBack`, `Busy`, `DoNotDisturb`, `InACall`, `InAConferenceCall`, `Inactive`, `InAMeeting`, `Offline`, `OffWork`, `OutOfOffice`, `PresenceUnknown`, `Presenting`, `UrgentInterruptionsOnly`. |
| availability | string collection | The base presence information for a user. Possible values are `Available`, `AvailableIdle`, `Away`, `BeRightBack`, `Busy`, `BusyIdle`, `DoNotDisturb`, `Offline`, `PresenceUnknown` |

## Hardware
This project is designed to run on a Wemos D1 Mini, with ESP8266, though can be easily adapted to something more powerful if required.

## Setup
### App Registration
1. Register an application in Entra (Previously Azure AD). Details [here](https://learn.microsoft.com/en-gb/graph/auth-register-app-v2).
2. Grant your application access to:
    - `Presence.Read`
3. Provide consent for your application to access this data.
4. Note your `Client ID` and `Client Secret`. These become `MS_CLIENT_ID` and `MS_CLIENT_SECRET` respectively in the `secret.h` file.

### Client Secrets - Secret.h
Copy `Example Secret.h` to `Secret.h` and fill in the details.
```c++
// Network Config
#define WIFI_SSID "" // Your WiFi SSID (2.4GHz)
#define WIFI_PASS "" // Your WiFi Password

// Microsoft Account, Tennant
#define MS_USER "" // Your MS Username, e.g. username@domain.com
#define MS_PASS "" // Your MS Password

#define MS_LOGIN "login.microsoftonline.com" // Unlikely to change
#define MS_LOGIN_PATH "/common/oauth2/token" // You may need to scope "common" to your tennant, e.g. "/{tennant}/oauth2/token

// Your MS Application. See https://learn.microsoft.com/en-gb/graph/auth-register-app-v2?view=graph-rest-1.0#register-an-application
#define MS_RESOURCE "https://graph.microsoft.com"
#define MS_CLIENT_ID "" // Your MS Client ID. 
#define MS_CLIENT_SECRET "" // Your MS Client Secret

// Microsoft Graph API
#define MS_GRAPH "graph.microsoft.com"
#define MS_GRAPH_PATH "/v1.0/me/presence"
```
