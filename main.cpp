// docker run -v /home/jcastro/Desktop/cppweb:/usr/src/cppweb -p 8080:8080 -e PORT=8080 -it cppbox:latest /usr/src/cppweb/hello_crow/build/hello_crow

#include "crow_all.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace crow;

void sendFile(response & res, string filename, string contentType)
{
  ifstream in("../public/" + filename, ifstream::in);
  if (in) {
      ostringstream contents;
      contents << in.rdbuf();
      in.close();
      res.set_header("Content-Type:", contentType);
      res.write(contents.str());
  } else {
      res.code = 404;
      res.write("Not found");
  }
  res.end();
}

void sendHtml(response & res, string filename)
{
  sendFile(res, filename + ".html", "text/html");
}

void sendText(response & res, string filename)
{
  sendFile(res, filename + ".txt", "text/text");
}

void sendJson(response & res, string filename)
{
  sendFile(res, filename + ".json", "text/json");
}

void sendImage(response & res, string filename)
{
  sendFile(res, "/images/" + filename, "image/jpeg");
}

void sendStyle(response & res, string filename)
{
  sendFile(res, "/styles/" + filename + ".css", "text/css");
}

int main(int argc, char* argv[])
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/about")
      ([](const request & req, response & res){
        sendHtml(res, "about");
      });

    CROW_ROUTE(app, "/styles/<string>")
      ([](const request & req, response & res, string filename){
        sendStyle(res, "styles");
      });

    CROW_ROUTE(app, "/")
      ([](const request & req, response & res){
        sendHtml(res, "index");
      });

    CROW_ROUTE(app, "/text")
      ([](const request & req, response & res){
        sendText(res, "text");
      });

    char * port = getenv("PORT");
    uint16_t iport = static_cast<uint16_t>(port != NULL? stoi(port) : 8080);
    cout << "PORT=" << iport << endl;

    app.port(iport).multithreaded().run();
    cout << "Server started on port " << iport << endl;

    return 0;
}
