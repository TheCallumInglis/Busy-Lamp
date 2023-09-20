#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include "Secret.h"

const size_t MAX_CONTENT_SIZE = 6144;

WiFiClientSecure wifiClient;
HTTPClient httpClient;

/** OAuth Start **/
char* getOAuthToken(bool retry) {
  // TODO Check we have wifi

  Serial.println("Requesting OAuth Token... ");
  char* oAuthStr = buildOAuthString(MS_RESOURCE, MS_CLIENT_ID, MS_CLIENT_SECRET, MS_USER, MS_PASS);

  wifiClient.setInsecure();
  httpClient.begin(wifiClient, MS_LOGIN, 443, MS_LOGIN_PATH);
  httpClient.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = httpClient.POST(oAuthStr);

  free(oAuthStr); // Housekeeping

  if (httpCode != HTTP_CODE_OK) {
    Serial.print("Failed! HTTP response code: "); Serial.println(httpCode);

    if (retry) {
      return getOAuthToken(false); // Try once more
    }

    Serial.println("Failed. Giving Up.");
    return nullptr;  
  }

  return readOAuthResponse(); // Gets Access Token
}

/**
 * @brief Builds an OAuth request string for Microsoft Graph API authentication.
 *
 * @param msResource The resource to be accessed.
 * @param msClientID The client ID associated with the application.
 * @param msClientSecret The client secret associated with the application.
 * @param msUser The username of the user for authentication.
 * @param msPass The password of the user for authentication.
 * @return A dynamically allocated string representing the OAuth request. The caller is
 *         responsible for freeing the allocated memory using free().
 *         Returns nullptr if memory allocation fails.
 */
char* buildOAuthString(const char* msResource, const char* msClientID, const char* msClientSecret, const char* msUser, const char* msPass) {
  size_t bufferSize = strlen("resource=") + strlen(msResource) +
                      strlen("&client_id=") + strlen(msClientID) +
                      strlen("&client_secret=") + strlen(msClientSecret) +
                      strlen("&username=") + strlen(msUser) +
                      strlen("&password=") + strlen(msPass) +
                      strlen("&grant_type=password&scope=Presence.Read") + 1; // + null terminator

  char* oAuthStr = (char*)malloc(bufferSize);

  if (!oAuthStr) {
    return nullptr;
  }

  // Build the request string
  snprintf(oAuthStr, bufferSize, "resource=%s&client_id=%s&client_secret=%s&username=%s&password=%s&grant_type=password&scope=Presence.Read%%20Presence.Read.All",
           msResource, msClientID, msClientSecret, msUser, msPass);

  return oAuthStr;
}

char* readOAuthResponse() {
  DynamicJsonDocument* doc = strToJson(httpClient);

  if (!doc) {
    Serial.println("Failed to parse JSON response");
    return nullptr;
  }

  char* access_token = strdup((*doc)["access_token"]);
  delete doc; // Housekepping

  Serial.println("Got Access Token");
  return access_token;
}
/** OAuth End **/

/** User Presence Start **/
char* getUserPresence(char* accessToken) {
  // TODO Check we have wifi
  Serial.println("Getting User Presence...");

  // Calculate the total length needed for the Authorization header
  size_t headerLength = strlen("Bearer ") + strlen(accessToken) + 1;
  char* authorizationHeader = (char*)malloc(headerLength);
  if (!authorizationHeader) {
    Serial.println("Failed to allocate memory for the header");
    return;
  }
  snprintf(authorizationHeader, headerLength, "Bearer %s", accessToken);


  wifiClient.setInsecure();
  httpClient.begin(wifiClient, MS_GRAPH, 443, MS_GRAPH_PATH);
  httpClient.addHeader("Authorization", authorizationHeader);  
  int httpCode = httpClient.GET();
  free(authorizationHeader); // Housekeeping

  if (httpCode != HTTP_CODE_OK) {
    Serial.print("Failed! HTTP response code: "); Serial.println(httpCode);
    return;
  }

  // Handle Response
  DynamicJsonDocument* doc = strToJson(httpClient);

  if (!doc) {
    return;
  }

  char* availability = strdup((*doc)["availability"]); // Take copy of availability
  char* activity = strdup((*doc)["activity"]); // Take copy of activity

  delete doc; // Housekeeping

  Serial.print("Availability: "); Serial.print(availability);
  Serial.print(", Activity: "); Serial.println(activity);

  return availability;
}
/** User Presence End **/

/** Helpers Start **/
bool setupWifi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  wifiClient.setInsecure();
  Serial.println(" Connected!");
  return true;
}

DynamicJsonDocument* strToJson(HTTPClient& httpClient) {
  String resp = httpClient.getString();

  DynamicJsonDocument* doc = new DynamicJsonDocument(MAX_CONTENT_SIZE);
  DeserializationError error = deserializeJson(*doc, resp); 

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());

    delete doc;
    return nullptr;
  }

  return doc;
}
/** Helpers End **/

char* getAvailability() {
  char* accessToken = getOAuthToken(true);
  char* availability = getUserPresence(accessToken);
}

void setup() {
  Serial.begin(115200);

  setupWifi();
  getAvailability();
}


void loop() {

}
