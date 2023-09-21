#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <FastLED.h>

#include "Secret.h"
#include "Config.h"

const size_t MAX_CONTENT_SIZE = 6144;
CRGB leds[NUM_LEDS];

WiFiClientSecure wifiClient;
HTTPClient httpClient;

char* accessToken = nullptr;

/** OAuth Start **/
char* getOAuthToken(bool reAuth = false) {
  // TODO Check we have wifi

  // We already have an access token and not wanting it refreshed
  if (accessToken != nullptr && !reAuth) {
    Serial.println("Using Existing Access Token");
    return accessToken; 
  }

  // Otherwise request new token
  Serial.println("Requesting New OAuth Token... ");
  char* oAuthStr = buildOAuthString(MS_RESOURCE, MS_CLIENT_ID, MS_CLIENT_SECRET, MS_USER, MS_PASS);

  wifiClient.setInsecure();
  httpClient.begin(wifiClient, MS_LOGIN, 443, MS_LOGIN_PATH);
  httpClient.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = httpClient.POST(oAuthStr);

  free(oAuthStr); // Housekeeping

  if (httpCode != HTTP_CODE_OK) {
    Serial.print("Failed! HTTP response code: "); Serial.println(httpCode);
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

  accessToken = strdup((*doc)["access_token"]);
  delete doc; // Housekepping

  Serial.println("Got Access Token");
  return accessToken;
}
/** OAuth End **/

/** User Presence Start **/
char* getUserPresence() {
  // TODO Check we have wifi

  // TODO Check if we have token

  Serial.println("Getting User Presence...");

  // Calculate the total length needed for the Authorization header
  size_t headerLength = strlen("Bearer ") + strlen(accessToken) + 1;
  char* authorizationHeader = (char*)malloc(headerLength);
  if (!authorizationHeader) {
    Serial.println("Failed to allocate memory for the header");
    return nullptr;
  }
  snprintf(authorizationHeader, headerLength, "Bearer %s", accessToken);


  wifiClient.setInsecure();
  httpClient.begin(wifiClient, MS_GRAPH, 443, MS_GRAPH_PATH);
  httpClient.addHeader("Authorization", authorizationHeader);  
  int httpCode = httpClient.GET();
  free(authorizationHeader); // Housekeeping

  if (httpCode != HTTP_CODE_OK) {
    Serial.print("Failed! HTTP response code: "); Serial.println(httpCode);

    if (httpCode == HTTP_CODE_UNAUTHORIZED) {
      Serial.println("Unathorized, refreshing token...");
      getOAuthToken(true); // Refresh Token
      return getUserPresence();
    }

    return nullptr;
  }

  // Handle Response
  DynamicJsonDocument* doc = strToJson(httpClient);

  if (!doc) {
    return nullptr;
  }

  char* availability = strdup((*doc)["availability"]); // Take copy of availability
  char* activity = strdup((*doc)["activity"]); // Take copy of activity

  delete doc; // Housekeeping

  // Serial.print("Availability: "); Serial.print(availability);
  // Serial.print(", Activity: "); Serial.println(activity);

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

void setupLED() {
  FastLED.addLeds<LED_TYPE, LED_PIN, LED_COLOUR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(LED_BRIGHTNESS);
}

void setLED(CRGB colour) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = colour;
  }

  FastLED.show();
  FastLED.delay(10);
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
  getOAuthToken();
  char* availability = getUserPresence();

  return availability;
}

CRGB getAvailabilityColour(const char* availability) {
  if (strcmp(availability, "Available") == 0) {
    return CRGB::LimeGreen;
  } else if (strcmp(availability, "Busy") == 0) {
    return CRGB::DarkMagenta;
  } else if (strcmp(availability, "DoNotDisturb") == 0) {
    return CRGB::Red;
  } else if (strcmp(availability, "BeRightBack") == 0 || strcmp(availability, "Away") == 0) {
    return CRGB::Orange;
  } else if (strcmp(availability, "Offline") == 0) {
    return CRGB::Black;
  } else {
    return CRGB::Black;  // Default case
  }
}

void setup() {
  Serial.begin(115200);

  setupWifi();
  setupLED();
}

void loop() {
  // Get Status
  const char* availability = getAvailability();
  Serial.print("Availability: "); Serial.println(availability);

  // Update Colour
  CRGB colour = getAvailabilityColour(availability);
  setLED(colour);

  delay(5000);
}
