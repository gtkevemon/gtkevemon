#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>

#include "util/exception.h"
#include "http.h"

void
HttpData::dump_headers (void)
{
  std::cout << "==== HTTP Headers ====" << std::endl;
  for (std::size_t i = 0; i < this->headers.size(); ++i)
    std::cout << this->headers[i] << std::endl;
}

/* ---------------------------------------------------------------- */

void
HttpData::dump_data (void)
{
  std::cout << "==== HTTP Data dump ====" << std::endl;
  std::cout.write(&this->data[0], this->data.size());
  std::cout << std::endl;
}

/* ================================================================ */

Http::Http (void)
{
  this->initialize_defaults();
}

/* ---------------------------------------------------------------- */

Http::Http (std::string const& host, std::string const& path)
{
  this->host = host;
  this->path = path;
  this->initialize_defaults();
}

/* ---------------------------------------------------------------- */

void
Http::initialize_defaults (void)
{
  this->method = HTTP_METHOD_GET;
  this->port = 80;
  this->agent = "GtkEveMon HTTP Requester";
  this->proxy_port = 80;
  this->use_ssl = false;

  this->http_state = HTTP_STATE_READY;
  this->bytes_read = 0;
  this->bytes_total = 0;
}

/* ---------------------------------------------------------------- */

HttpDataPtr
Http::request (void)
{
  // Set up variables
  HttpDataPtr result = HttpData::create();
  std::stringstream url;
  unsigned int i;

/*  // Just exit early
  result->http_code = 500;
  http_state = HTTP_STATE_ERROR;
  return result;
*/
  CURL * curl_handle = NULL;
  CURLcode res = CURLE_OK;
  try
  {
    curl_handle = curl_easy_init();
  
    if (use_ssl)
      url << "https://";
    else
      url << "http://";
    url << host;
    if (port != 443 && port != 80)
      url << ":" << port;
    url << path;

    HttpCombo combo;
    combo.http = this;
    combo.httpdataptr = &result;

    curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, Http::data_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *) &combo);
    curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, Http::header_callback);
    curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void *) &combo);
    curl_easy_setopt(curl_handle, CURLOPT_URL, url.str().c_str());
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, agent.c_str());
  
    if (proxy.size() > 0) {
      curl_easy_setopt(curl_handle, CURLOPT_PROXY, proxy.c_str());
      curl_easy_setopt(curl_handle, CURLOPT_PROXYPORT, (long) proxy_port);
    }
  
    if (data.size() > 0) {
      curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
      curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data.c_str());
    }
  
    struct curl_slist *list = NULL;
    if (headers.size() > 0) {
      for (i = 0; i < headers.size(); i++)
        list = curl_slist_append(list, headers[i].c_str());
      curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, list);
    }
  
    // std::cout << "URL: " << url.str() << std::endl;
  
    http_state = HTTP_STATE_CONNECTING;
    res = curl_easy_perform(curl_handle);
    
    result->data.push_back(0);
    curl_slist_free_all(list);
  
    long lhttp_code;
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &lhttp_code);
    result->http_code = (HttpStatusCode) lhttp_code;
  
    // Error checking
    if (res == CURLE_OK)
      http_state = HTTP_STATE_DONE;
    else
      throw Exception(curl_easy_strerror(res));
  }
  catch (Exception & e)
  {
    http_state = HTTP_STATE_ERROR;
    std::cout << "HTTP Failure: " << curl_easy_strerror(res) << std::endl;
    curl_easy_cleanup(curl_handle);
    throw Exception(e);
  }


/*  // Debugging:
  // Print headers
  for (i = 0; i < result->headers.size(); i++) {
   printf("Header: %s\n", result->headers[i].c_str());
  }
  // Print data
  printf("bytes_total: %lu\n", bytes_total);
  printf("Data: \n");
  for (i = 0; i < (int) result->data.size(); i++)
   printf("%c", result->data[i]);
*/
  
  curl_easy_cleanup(curl_handle);
  return result;
}

/* ---------------------------------------------------------------- */

unsigned int
Http::get_uint_from_str (std::string const& str)
{
  std::stringstream ss(str);
  int ret;
  ss >> ret;
  return ret;
}

/* ---------------------------------------------------------------- */

std::size_t
Http::header_callback(char * buffer, std::size_t size, std::size_t nitems, void * combo)
{
  std::size_t buffer_size = size * nitems;
  if (buffer_size > 2) {
    Http * http = ((HttpCombo *) combo)->http;
    HttpDataPtr result = *((HttpCombo *) combo)->httpdataptr;

    http->http_state = HTTP_STATE_RECEIVING;

    std::string header_line;
    header_line.assign(buffer, buffer_size);
    if (header_line[buffer_size - 2] == '\r')
      header_line.erase(buffer_size - 2);

    result->headers.push_back(header_line);

    if (header_line.substr(0, 16) == "Content-Length: ")
      http->bytes_total = http->get_uint_from_str(header_line.substr(16));
  }
  return buffer_size;
}

/* ---------------------------------------------------------------- */

std::size_t
Http::data_callback(char * buffer, std::size_t size, std::size_t nmemb, void * combo)
{
  Http * http = ((HttpCombo *) combo)->http;
  HttpDataPtr result = *((HttpCombo *) combo)->httpdataptr;

  unsigned long previous_size = result->data.size();
  unsigned long current_size = previous_size + size * nmemb;
  http->bytes_read = current_size;
  result->data.resize(current_size);
  memcpy(&result->data[previous_size], buffer, size * nmemb);

  return size * nmemb;
}
