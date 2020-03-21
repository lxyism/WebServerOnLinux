#include"HttpServer.h"
#include"base/Logger/Logging.h"

const int SERVER_PORT = 9000;

int main(){
    HttpServer server(SERVER_PORT, 4, "log_test", 3);
    server.Start();
}