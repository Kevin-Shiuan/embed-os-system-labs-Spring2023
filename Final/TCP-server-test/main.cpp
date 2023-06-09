#include "ESP8266Interface.h"
#include "mbed.h"

ESP8266Interface wifi(D8, D2);

TCPSocket server;

int main() {
  // Connect to Wi-Fi
  wifi.connect("exys305", "305305abcd");

  // Bind the server to port 80
  server.open(&wifi);
  server.bind(80);

  // Listen for incoming connections
  server.listen();

  while (true) {
    // Accept a new connection
    TCPSocket *client;
    client = server.accept();

    if (client != NULL) {
      // Receive the request from the client
      char buffer[256];
      int n = client->recv(buffer, sizeof(buffer));

      if (n > 0) {
        // Print the request
        printf("Received: %.*s\n", n, buffer);
      }

      // Send a response to the client
      const char *response = "Hello, world!";
      client->send(response, strlen(response));

      // Close the connection
      client->close();
      delete client;
    }
  }
}