#include "BMP180.h"
#include "mbed.h"
#include "wifi_helper.h"
#include <mutex>
#include <string>
#include <vector>

using namespace std;

#if MBED_CONF_APP_USE_TLS_SOCKET
#include "root_ca_cert.h"

#ifndef DEVICE_TRNG
#error "mbed-os-example-tls-socket requires a device which supports TRNG"
#endif
#endif // MBED_CONF_APP_USE_TLS_SOCKET

I2C i2c(I2C_SDA, I2C_SCL);
BMP180 bmp180(&i2c);
PwmOut led(LED1);

static int time_step =
    750; // use for pid data sampling rate and fan speed update rate

static float temperature = 0.0;
static int fanMode = 0;
static bool connected = false;
static bool fanOn = false;
static bool manualMode = false;
static float desired_temperature = 26.5;
static float fanSpeedPwm = 0.0;
Thread t1, t2, t3, t4;

static Mutex temperature_mutex;
static Mutex fanMode_mutex;
static Mutex connected_mutex;
static Mutex fanOn_mutex;
static Mutex manualMode_mutex;
static Mutex desired_temperature_mutex;
static Mutex fanSpeedPwm_mutex;

void init_BMP180() {
  while (1) {
    if (bmp180.init() != 0) {
      printf("Error communicating with BMP180\n");
    } else {
      printf("Initialized BMP180\n");
      break;
    }
    ThisThread::sleep_for(std::chrono::seconds(5));
  }
}

void read_temperature() {
  while (1) {
    // printf("Reading temperature\r\n");
    bmp180.startTemperature();
    ThisThread::sleep_for(
        std::chrono::milliseconds(5)); // Wait for conversion to complete
    float temp;
    if (bmp180.getTemperature(&temp) != 0) {
      printf("Error getting temperature\n");
      // continue;
    }

    // printf("Temperature = %f C\n", temp);
    temperature_mutex.lock();
    temperature = temp;
    // printf("Temperature = %f C\n", temperature);
    temperature_mutex.unlock();
    ThisThread::sleep_for(std::chrono::seconds(1));
  }
}

struct KeyValue {
  string key;
  string value;
};

vector<KeyValue> parseJson(string jsonString) {
  vector<KeyValue> result;
  size_t pos = 0;
  while (pos < jsonString.length()) {
    size_t keyPos = jsonString.find_first_of("\"", pos);
    if (keyPos == string::npos) {
      break;
    }
    size_t keyEndPos = jsonString.find_first_of("\"", keyPos + 1);
    if (keyEndPos == string::npos) {
      break;
    }
    string key = jsonString.substr(keyPos + 1, keyEndPos - keyPos - 1);
    size_t valuePos = jsonString.find_first_of(":", keyEndPos + 1);
    if (valuePos == string::npos) {
      break;
    }
    size_t valueEndPos = jsonString.find_first_of(",", valuePos + 1);
    if (valueEndPos == string::npos) {
      valueEndPos = jsonString.length() - 1;
    }
    string value = jsonString.substr(valuePos + 1, valueEndPos - valuePos - 1);
    // Remove double quotes from value
    value.erase(remove(value.begin(), value.end(), '\"'), value.end());
    result.push_back({key, value});
    pos = valueEndPos + 1;
  }
  return result;
}

int stringToInt(const string &str) {
  int result = 0;
  bool negative = false;
  for (char c : str) {
    if (c == '-') {
      negative = true;
    } else if (c >= '0' && c <= '9') {
      result = result * 10 + (c - '0');
    } else {
      // Invalid character
      return 0;
    }
  }
  return negative ? -result : result;
}

class WifiSocket {
  static constexpr size_t MAX_NUMBER_OF_ACCESS_POINTS = 10;
  static constexpr size_t MAX_MESSAGE_RECEIVED_LENGTH = 100;

#if MBED_CONF_APP_USE_TLS_SOCKET
  static constexpr size_t REMOTE_PORT = 443;
#else
  //   static constexpr size_t REMOTE_PORT = 80; // standard HTTP port
  static constexpr size_t REMOTE_PORT = 65431;
  //   static constexpr char REMOTE_PORT[6] = MBED_CONF_APP_PORT;

#endif // MBED_CONF_APP_USE_TLS_SOCKET

public:
  WifiSocket() : _net(NetworkInterface::get_default_instance()) {}

  ~WifiSocket() {
    if (_net) {
      _net->disconnect();
    }
  }

  void run() {
    if (!_net) {
      printf("Error! No network interface found.\r\n");
      return;
    }

    // printf("Connecting to the network...\r\n");

    nsapi_size_or_error_t result = _net->connect();
    if (result != 0) {
      printf("Error! _net->connect() returned: %d\r\n", result);
      return;
    }
    // printf("Connected to the network\r\n\r\n");

    // print_network_info();

    /* opening the socket only allocates resources */
    result = _socket.open(_net);
    if (result != 0) {
      printf("Error! _socket.open() returned: %d\r\n", result);
      return;
    }

#if MBED_CONF_APP_USE_TLS_SOCKET
    result = _socket.set_root_ca_cert(root_ca_cert);
    if (result != NSAPI_ERROR_OK) {
      printf("Error: _socket.set_root_ca_cert() returned %d\n", result);
      return;
    }
    _socket.set_hostname(MBED_CONF_APP_HOSTNAME);
#endif // MBED_CONF_APP_USE_TLS_SOCKET

    /* now we have to find where to connect */

    SocketAddress address;

    if (!resolve_hostname(address)) {
      return;
    }

    address.set_port(REMOTE_PORT);

    /* we are connected to the network but since we're using a connection
     * oriented protocol we still need to open a connection on the socket */

    // printf("Opening connection to remote port %d\r\n", REMOTE_PORT);

    result = _socket.connect(address);
    if (result != 0) {
      printf("Error! _socket.connect() returned: %d\r\n", result);
      return;
    }
    connected_mutex.lock();
    connected = true;
    connected_mutex.unlock();
    /* exchange an HTTP request and response */

    if (!send_http_request()) {
      return;
    }

    if (!receive_http_response()) {
      return;
    }

    // printf("Demo concluded successfully \r\n");
  }

private:
  bool resolve_hostname(SocketAddress &address) {
    const char hostname[] = MBED_CONF_APP_HOSTNAME;

    /* get the host address */
    // printf("\nResolve hostname %s\r\n", hostname);
    nsapi_size_or_error_t result = _net->gethostbyname(hostname, &address);
    if (result != 0) {
      printf("Error! gethostbyname(%s) returned: %d\r\n", hostname, result);
      return false;
    }

    // printf("%s address is %s\r\n", hostname,
    //        (address.get_ip_address() ? address.get_ip_address() : "None"));

    return true;
  }

  bool send_http_request() {
    /* loop until whole request sent */
    const char buffer[] = "Give me that dang mode\r\n";

    nsapi_size_t bytes_to_send = strlen(buffer);
    nsapi_size_or_error_t bytes_sent = 0;

    // printf("\r\nSending message: \r\n%s", buffer);

    while (bytes_to_send) {
      bytes_sent = _socket.send(buffer + bytes_sent, bytes_to_send);
      if (bytes_sent < 0) {
        printf("Error! _socket.send() returned: %d\r\n", bytes_sent);
        return false;
      }
      // printf("sent %d bytes\r\n", bytes_sent);

      bytes_to_send -= bytes_sent;
    }

    // printf("Complete message sent\r\n");

    return true;
  }

  bool receive_http_response() {
    char buffer[MAX_MESSAGE_RECEIVED_LENGTH];
    int remaining_bytes = MAX_MESSAGE_RECEIVED_LENGTH;
    int received_bytes = 0;

    /* loop until there is nothing received or we've ran out of buffer space */
    nsapi_size_or_error_t result = remaining_bytes;
    while (result > 0 && remaining_bytes > 0) {
      result = _socket.recv(buffer + received_bytes, remaining_bytes);
      if (result < 0) {
        printf("Error! _socket.recv() returned: %d\r\n", result);
        return false;
      }

      received_bytes += result;
      remaining_bytes -= result;
    }
    string jsonString(buffer);
    int fanModeTemp = 0;
    // printf("JSON string: %s\r\n", jsonString.c_str());
    vector<KeyValue> keyValuePairs = parseJson(jsonString);
    for (auto keyValue : keyValuePairs) {
      if (keyValue.key == "fanMode") {
        // printf("Fan mode: %s\r\n", keyValue.value.c_str());
        fanModeTemp = stringToInt(keyValue.value);
      }
    }
    fanMode_mutex.lock();
    fanMode = fanModeTemp;
    fanMode_mutex.unlock();
    // printf("Fan mode in int: %d\r\n", fanMode);

    // printf("Fan mode: %d\r\n", fanModeStr);
    // fanMode = stringToInt(fanModeStr);

    /* the message is likely larger but we only want the HTTP response code */

    // printf("received %d bytes:\r\n%.*s\r\n\r\n", received_bytes,
    //        strstr(buffer, "\n") - buffer, buffer);

    return true;
  }

private:
  NetworkInterface *_net;

#if MBED_CONF_APP_USE_TLS_SOCKET
  TLSSocket _socket;
#else
  TCPSocket _socket;
#endif // MBED_CONF_APP_USE_TLS_SOCKET
};

void read_fanMode() {
  while (1) {
    printf("Reading fan mode\r\n");
    WifiSocket *mySocket = new WifiSocket();
    mySocket->run();
    delete mySocket;
    ThisThread::sleep_for(std::chrono::seconds(1));
  }
}

void printf_all() {
  while (1) {
    printf("now printing lastest status:\r\n");
    connected_mutex.lock();
    fanOn_mutex.lock();
    temperature_mutex.lock();
    fanMode_mutex.lock();
    printf("Connected: %d\r\n", connected);
    printf("Fan on: %d\r\n", fanOn);
    printf("Temperature: %f\r\n", temperature);
    printf("Fan mode: %d\r\n", fanMode);
    printf("\r\n\r\n");
    connected_mutex.unlock();
    fanOn_mutex.unlock();
    temperature_mutex.unlock();
    fanMode_mutex.unlock();
    ThisThread::sleep_for(std::chrono::seconds(3));
  }
}

const int TABLE_SIZE = 256;
float sine_table[TABLE_SIZE];

void init_sine_table() {
  for (int i = 0; i < TABLE_SIZE; i++) {
    float angle = 2.0f * 3.14159 * i / TABLE_SIZE;
    sine_table[i] = (sinf(angle) + 1.0f) / 2.0f;
  }
}

void pwm_output_test() {
  init_sine_table();
  int index = 0;
  while (1) {
    led.write(sine_table[index]);
    index = (index + 1) % TABLE_SIZE;
    ThisThread::sleep_for(std::chrono::milliseconds(1));
  }
}

class PIDController {
public:
  PIDController(double kp, double ki, double kd) : kp(kp), ki(ki), kd(kd) {}

  double calculate(double input, double setpoint) {
    double error = setpoint - input;
    integral += error;
    double derivative = error - previous_error;
    previous_error = error;
    double output =
        kp * error + ki * integral + kd * derivative / (time_step / 1000.0);
    // return std::max(0.0, std::min(1.0, output));
    return output;
  }

private:
  double kp = 1.0, ki = 0.1, kd = 0.5;
  double integral = 0.0;
  double previous_error = 0.0;
};

void controlTemperature() {
  PIDController pid(0.5, 0.1, 0.05);
  while (1) {
    temperature_mutex.lock();
    double input = temperature;
    temperature_mutex.unlock();
    desired_temperature_mutex.lock();
    double setpoint = desired_temperature;
    // desired_temperature-=0.05;
    desired_temperature_mutex.unlock();
    double output = 0.0;
    // if (input - setpoint > 0.3)
    // {
    output = pid.calculate(input, setpoint);
    // }

    printf("input: %f, setpoint: %f, output: %f\r\n", input, setpoint, output);
    ThisThread::sleep_for(std::chrono::milliseconds(time_step));
  }
}

int main(void) {
  // init
  init_BMP180();

  t1.start(read_temperature);
  //   t2.start(read_fanMode);
  //   t3.start(printf_all);
  t4.start(controlTemperature);
}