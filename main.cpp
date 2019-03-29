#include "mbed.h"
#include "zest-radio-atzbrf233.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"

// Network interface
NetworkInterface *net;

int valeur_arrive = 0;

/* Printf the message received and its configuration */
void messageArrived(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
    ++valeur_arrive;
}
namespace{
#define PERIOD_MS 500
}
static DigitalOut led1(LED1);

static AnalogIn Capteur_humidite(ADC_IN1);
static float temperature_air = 0.000244 * 3.3;
static float humidite_eau = 0.748962 * 3.3;

I2C i2c(I2C1_SDA, I2C1_SCL);
uint8_t lm75_adress = 0x48 << 1;


void sendData(MQTT::Client<MQTTNetwork, Countdown>& client, char* topic, float humidity)
{
    //const char* topic = "floborie/feeds/temperature";
    printf("On envoie sur le topic %s\n", topic);

    int rc = 0;
    if ((rc = client.subscribe(topic, MQTT::QOS2, messageArrived)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);

    MQTT::Message message;
    char buf[100];
    sprintf(buf, "%f", humidity);
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)buf;
    message.payloadlen = strlen(buf)+1;

    rc = client.publish(topic, message);
    while (valeur_arrive < 1)
        client.yield(100);

    valeur_arrive = 0;
}


float getHumidity()
{
    return ((Capteur_humidite.read() * 3.3) - temperature_air) * 100.0 / (humidite_eau - temperature_air);
}

float getTemperature()
{
    char cmd[2];
    cmd[0] = 0x00;
    i2c.write(lm75_adress, cmd, 1);
    i2c.read(lm75_adress, cmd, 2);
    return ((cmd[0] << 8 | cmd[1] ) >> 7) * 0.5;
}



// MQTT demo
int main() {
	int result;

    // Add the border router DNS to the DNS table
    nsapi_addr_t new_dns = {
        NSAPI_IPv6,
        { 0xfd, 0x9f, 0x59, 0x0a, 0xb1, 0x58, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x01 }
    };
    nsapi_dns_add_server(new_dns);

    // humidity
        float humidity = getHumidity();
        printf("Humidité du sol = %f % \n\r", humidity);

	// temperature
	float temperature = getTemperature();
	printf("Température de l'air = %f °C\r\n", temperature);

    printf("Starting MQTT demo\n");

    // Get default Network interface (6LowPAN)
    net = NetworkInterface::get_default_instance();
    if (!net) {
        printf("Error! No network interface found.\n");
        return 0;
    }



    // Connect 6LowPAN interface
    result = net->connect();
    if (result != 0) {
        printf("Error! net->connect() returned: %d\n", result);
        return result;
    }

    // Build the socket that will be used for MQTT
    MQTTNetwork mqttNetwork(net);

    // Declare a MQTT Client
    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

    // Connect the socket to the MQTT Broker
    const char* hostname = "io.adafruit.com";
    uint16_t port = 1883;
    printf("Connecting to %s:%d\r\n", hostname, port);
    int rc = mqttNetwork.connect(hostname, port);
    if (rc != 0)
        printf("rc from TCP connect is %d\r\n", rc);

    // Connect the MQTT Client
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = "mbed-sample";
    data.username.cstring = "floborie";
    data.password.cstring = "b2a66f84bb5e440f8341e374d908e294";
    if ((rc = client.connect(data)) != 0)
        printf("rc from MQTT connect is %d\r\n", rc);

    sendData(client, "floborie/feeds/Temperature", temperature_air);
    sendData(client, "floborie/feeds/Humidité", humidite_eau);


    // Disconnect client and socket
    client.disconnect();
    mqttNetwork.disconnect();

    // Bring down the 6LowPAN interface
    net->disconnect();
    printf("Done\n");
}


