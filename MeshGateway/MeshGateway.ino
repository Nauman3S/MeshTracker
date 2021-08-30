//************************************************************
// this is a simple example that uses the painlessMesh library to
// connect to a another network and relay messages from a MQTT broker to the nodes of the mesh network.
// To send a message to a mesh node, you can publish it to "painlessMesh/to/12345678" where 12345678 equals the nodeId.
// To broadcast a message to all nodes in the mesh you can publish it to "painlessMesh/to/broadcast".
// When you publish "getNodes" to "painlessMesh/to/gateway" you receive the mesh topology as JSON
// Every message from the mesh which is send to the gateway node will be published to "painlessMesh/from/12345678" where 12345678
// is the nodeId from which the packet was send.
//************************************************************
#include <Arduino.h>
#include "SStack.h"
#include "WiFiCreds.h"
#include "NTPHandler.h"

#include <painlessMesh.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include "APIServerCreds.h"
#include "APIReq.h"

#define MESH_PREFIX "meshService"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555

#define STATION_SSID ssid
#define STATION_PASSWORD password

#define HOSTNAME "MQTT_Bridge"

#define LED 2 // GPIO number of connected LED, ON ESP-12 IS GPIO2

// Prototypes
void receivedCallback(const uint32_t &from, const String &msg);
void mqttCallback(char *topic, byte *payload, unsigned int length);

IPAddress getlocalIP();

IPAddress myIP(0, 0, 0, 0);
//44.195.192.158
IPAddress mqttBroker(44, 195, 192, 158);


Scheduler userScheduler; // to control your personal task
painlessMesh mesh;
WiFiClient wifiClient;
PubSubClient mqttClient(mqttBroker, 1883, mqttCallback, wifiClient);
String TrackerID = "";

bool onFlag = false;

void APIPostReq();

Task taskAPIPostReq(TASK_SECOND * 10, TASK_FOREVER, &APIPostReq); // start with a one second interval
HTTPReq httpClient;
class TrackerList
{
private:
  const int size = 100;
  int TrackerPointer = 0;
  String TrackerList[100];
  String TimeStampList[100];

public:
  //  TrackerList(int size){
  //    size=0;
  // }
  uint8_t addMAC(String MAC)
  {
    for (int i = 0; i < size; i++)
    {
      if (TrackerList[i].indexOf(MAC) >= 0)
      {
        TimeStampList[i] = getTimeStamp();
        ///  break;
        return 0;
      }
    }

    if (TrackerPointer < size)
    {

      TrackerList[TrackerPointer] = MAC;
      TimeStampList[TrackerPointer] = getTimeStamp();
      TrackerPointer++;
      return 1;
    }
  }

  int getTrackerPointer()
  {
    return TrackerPointer;
  }

  String getJSON(int i)
  {
    if (i < size)
    {
      String v = "{\n\"Tracker_ID__c\":\"";
      v = v + TrackerList[i] + String("\",\n\"Tracker_DateTime__c\":\"");
      v = v + TimeStampList[i] + String("\"\n}");
      return v;
    }
    else
    {
      return String("NA");
    }
  }
  void printAll()
  {
    Serial.println("--------------");
    for (int i = 0; i < TrackerPointer; i++)
    {
      Serial.println(getJSON(i));
    }
    Serial.print("Trackers: ");
    Serial.println(TrackerPointer);
    Serial.println("--------------");
  }
};

TrackerList TL;
void setup()
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  setupNTP();
  TrackerID = String(WiFi.macAddress());
  TrackerID = StringSeparator(TrackerID, ':', 0) + StringSeparator(TrackerID, ':', 1) + StringSeparator(TrackerID, ':', 2) + StringSeparator(TrackerID, ':', 3) +
              StringSeparator(TrackerID, ':', 4) + StringSeparator(TrackerID, ':', 4) + StringSeparator(TrackerID, ':', 5);
  Serial.print("GatewayTrackerID: ");
  Serial.println(TrackerID);

  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION); // set before init() so that you can see startup messages

  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 6);
  mesh.onReceive(&receivedCallback);

  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);

  // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
  mesh.setRoot(true);
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);
  userScheduler.addTask(taskAPIPostReq);
  taskAPIPostReq.enable();
  httpClient.begin(&mqttClient,serverAddress);
  httpClient.addHeader(header);
  
}

void loop()
{
  mesh.update();
  digitalWrite(LED, !onFlag);
  mqttClient.loop();

  if (myIP != getlocalIP())
  {
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());

    if (mqttClient.connect("painlessMeshClient"))
    {
      mqttClient.publish("painlessMesh/from/gateway", "Ready!");
      mqttClient.subscribe("painlessMesh/to/#");
      httpClient.listenResponse();
    }
  }
}
void APIPostReq()
{
  Serial.println("Making Post Request");
  String topic = "BLEMesh/from/all";
  for (int i = 0; i < TL.getTrackerPointer(); i++)
  {
    String v = TL.getJSON(i);
  //  mqttClient.publish(topic.c_str(), v.c_str());
    httpClient.POST(v);
  }
  
  taskAPIPostReq.setInterval(TASK_SECOND * 10);
}
void receivedCallback(const uint32_t &from, const String &msg)
{
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  String topic = "painlessMesh/from/" + String(from);
  String temp = StringSeparator(msg, ' ', 2);
  String NodeMAC = StringSeparator(temp, '_', 0);
  Serial.print("HeartBeat of Node : ");
  Serial.println(NodeMAC);
  TL.addMAC(NodeMAC);
  TL.printAll();
  onFlag = !onFlag; //changeLEDState
  // for (int i = 0; i < TL.getTrackerPointer(); i++)
  // {
  //   String v = TL.getJSON(i);
  //   mqttClient.publish(topic.c_str(), v.c_str());
  // }
}

void mqttCallback(char *topic, uint8_t *payload, unsigned int length)
{
  char *cleanPayload = (char *)malloc(length + 1);
  memcpy(cleanPayload, payload, length);
  cleanPayload[length] = '\0';
  String msg = String(cleanPayload);
  free(cleanPayload);

  String targetStr = String(topic).substring(16);
  httpClient.updateResponse(topic,msg);
  if (targetStr == "gateway")
  {
    if (msg == "getNodes")
    {
      auto nodes = mesh.getNodeList(true);
      String str;
      for (auto &&id : nodes)
        str += String(id) + String(" ");
      mqttClient.publish("painlessMesh/from/gateway", str.c_str());
    }
  }
  else if (targetStr == "broadcast")
  {
    mesh.sendBroadcast(msg);
  }
  else
  {
    uint32_t target = strtoul(targetStr.c_str(), NULL, 10);
    if (mesh.isConnected(target))
    {
      mesh.sendSingle(target, msg);
    }
    else
    {
      mqttClient.publish("painlessMesh/from/gateway", "Client not connected!");
    }
  }
}

IPAddress getlocalIP()
{
  return IPAddress(mesh.getStationIP());
}