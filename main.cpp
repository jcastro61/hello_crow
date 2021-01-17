// docker run -v /home/jcastro/Desktop/cppweb:/usr/src/cppweb -p 8080:8080 -e PORT=8080 -it cppbox:latest /usr/src/cppweb/hello_crow/build/hello_crow

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <boost/filesystem.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <ostream>

#include "crow_all.h"

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::kvp;

using mongocxx::cursor;

using namespace std;
using namespace crow;
using namespace crow::mustache;

string getView(const string & filename, context & x)
{
  return load("../public/" + filename + ".html").render(x);
}

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
  sendFile(res, "styles/" + filename + ".css", "text/css");
}

int main(int argc, char* argv[])
{
    crow::SimpleApp app;
    set_base(".");

    mongocxx::instance inst {};
    // Use for Heroku, not local db
    // string mongoConnect = std::string(getenv("MONGO_URI"))
    string mongoConnect = std::string("mongodb://localhost:37017/contact");
    mongocxx::client conn {mongocxx::uri(mongoConnect)};
    auto collection = conn["CRM"]["contacts"];

    CROW_ROUTE(app, "/contact/<string>")
      ([&collection](string email){
        auto doc = collection.find_one(make_document(kvp("email", email)));
        crow::json::wvalue dto;
        dto["contact"] = json::load(bsoncxx::to_json(doc.value().view()));
        //return crow::response(bsoncxx::to_json(doc.value().view()));
        return getView("contact", dto);
      });

    CROW_ROUTE(app, "/contacts")
      ([&collection](){
        mongocxx::options::find opts;
        opts.limit(10);
        auto docs = collection.find({}, opts);
        crow::json::wvalue dto;
        std::vector<crow::json::rvalue> contacts;
        contacts.reserve(10);

        for (auto doc : docs) {
          contacts.push_back(json::load(bsoncxx::to_json(doc)));
          //std::cout << bsoncxx::to_json(doc) << std::endl;
        }
        dto["contacts"] = contacts;
        return getView("contacts", dto);

      });

    CROW_ROUTE(app, "/mongo")
      ([](){
        return "mongodb://localhost:37017/contact";
      });

    CROW_ROUTE(app, "/about")
      ([](const request & req, response & res){
        sendHtml(res, "about");
      });

    CROW_ROUTE(app, "/image/<string>")
      ([](const request & req, response & res, string filename){
        sendImage(res, filename);
      });

    CROW_ROUTE(app, "/styles/<string>")
      ([](const request & req, response & res, string filename){
        sendStyle(res, filename);
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
