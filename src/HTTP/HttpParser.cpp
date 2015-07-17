// Copyright (c) 2011-2015 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "HttpParser.h"

#include <stdexcept>

namespace cryptonote {

HttpResponse::HTTP_STATUS HttpParser::parseResponseStatusFromString(const std::string& status) {
  if (status == "200 OK" || status == "200 Ok") return cryptonote::HttpResponse::STATUS_200;
  else if (status == "404 Not Found") return cryptonote::HttpResponse::STATUS_404;
  else if (status == "500 Internal Server Error") return cryptonote::HttpResponse::STATUS_500;
  else throw std::runtime_error("Unknown HTTP status code is given");

  return cryptonote::HttpResponse::STATUS_200; //unaccessible
}


void HttpParser::receiveRequest(std::istream& stream, HttpRequest& request) {
  readWord(stream, request.method);
  readWord(stream, request.url);

  std::string httpVersion;
  readWord(stream, httpVersion);

  readHeaders(stream, request.headers);

  std::string body;
  size_t bodyLen = getBodyLen(request.headers);
  if (bodyLen) {
    readBody(stream, request.body, bodyLen);
  }
}


void HttpParser::receiveResponse(std::istream& stream, HttpResponse& response) {
  std::string httpVersion;
  readWord(stream, httpVersion);
  
  std::string status;
  char c;
  
  stream.get(c);
  while (stream.good() && c != '\r') { //Till the end
    status += c;
    stream.get(c);
  }

  if (!stream.good()) {
    throw std::runtime_error("Parser error: stream is not good");
  }

  if (c == '\r') {
    stream.get(c);
    if (c != '\n') {
      throw std::runtime_error("Parser error: '\\n' symbol is expected");
    }
  }

  response.setStatus(parseResponseStatusFromString(status));
  
  std::string name;
  std::string value;

  while (readHeader(stream, name, value)) {
    response.addHeader(name, value);
    name.clear();
    value.clear();
  }

  response.addHeader(name, value);
  auto headers = response.getHeaders();
  size_t length = 0;
  auto it = headers.find("Content-Length");
  if (it != headers.end()) {
    length = std::stoul(it->second);
  }
  
  std::string body;
  if (length) {
    readBody(stream, body, length);
  }

  response.setBody(body);
}


void HttpParser::readWord(std::istream& stream, std::string& word) {
  char c;

  stream.get(c);
  while (stream.good() && c != ' ' && c != '\r') {
    word += c;
    stream.get(c);
  }

  if (!stream.good()) {
    throw std::runtime_error("Parser error: stream is not good");
  }

  if (c == '\r') {
    stream.get(c);
    if (c != '\n') {
      throw std::runtime_error("Parser error: '\\n' symbol is expected");
    }
  }
}

void HttpParser::readHeaders(std::istream& stream, HttpRequest::Headers& headers) {
  std::string name;
  std::string value;

  while (readHeader(stream, name, value)) {
    headers[name] = value; //use insert
    name.clear();
    value.clear();
  }

  headers[name] = value; //use insert
}

bool HttpParser::readHeader(std::istream& stream, std::string& name, std::string& value) {
  char c;
  bool isName = true;

  stream.get(c);
  while (stream.good() && c != '\r') {
    if (c == ':') {
      if (stream.peek() == ' ') {
        stream.get(c);
      }

      if (name.empty()) {
        throw std::runtime_error("Header name must be not empty");
      }

      if (isName) {
        isName = false;
        stream.get(c);
        continue;
      }
    }

    if (isName) {
      name += c;
      stream.get(c);
    } else {
      value += c;
      stream.get(c);
    }
  }

  if (!stream.good()) {
    throw std::runtime_error("Parser error: stream is not good");
  }

  stream.get(c);
  if (c != '\n') {
    throw std::runtime_error("Parser error: '\\n' symbol is expected");
  }

  c = stream.peek();
  if (c == '\r') {
    stream.get(c).get(c);
    if (c != '\n') {
      throw std::runtime_error("Parser error: '\\n' symbol is expected");
    }

    return false; //no more headers
  }

  return true;
}

size_t HttpParser::getBodyLen(const HttpRequest::Headers& headers) {
  auto it = headers.find("Content-Length");
  if (it != headers.end()) {
    size_t bytes = std::stoul(it->second);
    return bytes;
  }

  return 0;
}

void HttpParser::readBody(std::istream& stream, std::string& body, const size_t bodyLen) {
  size_t read = 0;

  while (stream.good() && read < bodyLen) {
    body += stream.get();
    ++read;
  }

  if (!stream.good()) {
    throw std::runtime_error("stream is not good");
  }
}

}


