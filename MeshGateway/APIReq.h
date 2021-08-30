class HTTPReq
{
private:
    String data;
    String serverAddressv;
    String responseCode;
    String response;
    String headers = "";
    PubSubClient *p;
    String StringSeparator(String data, char separator, int index)
    {

        int found = 0;
        int strIndex[] = {0, -1};
        int maxIndex = data.length() - 1;

        for (int i = 0; i <= maxIndex && found <= index; i++)
        {
            if (data.charAt(i) == separator || i == maxIndex)
            {
                found++;
                strIndex[0] = strIndex[1] + 1;
                strIndex[1] = (i == maxIndex) ? i + 1 : i;
            }
        }

        return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
    }

public:
    void begin(PubSubClient *client, String serverAddress)
    {
        p = client;
        serverAddressv = serverAddress;
    }
    void addHeader(String h)
    {
        headers = h;
    }
    int POST(String requestData)
    {
        String payL=serverAddressv+String("^")+headers+String("^")+requestData;
        p->publish("BLEMesh/API/endpoint", payL.c_str());
    }

    int listenResponse()
    {
        p->subscribe("BLEMesh/API/response");
    }

    void updateResponse(String topic, String data)
    {
        if (topic == String("BLEMesh/API/response"))
        {
            responseCode=StringSeparator(data,'^',0);
            response=StringSeparator(data,'^',1);
            Serial.println(data);
        }
    }
};