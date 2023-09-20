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
