/*
 * This file is part of GtkEveMon.
 *
 * GtkEveMon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NET_TCP_SOCKET_HEADER
#define NET_TCP_SOCKET_HEADER

#include <string>
#include <curl/curl.h>

class TCPSocket
{
  public:
    /**
     * This constructor creates an unconnected TCPSocket object. The results
     * of all member functions but `connect' are undefined until the
     * socket is connected.
     */
    TCPSocket (void);

    /**
     * This constructor creates a connected TCPSocket object. Internally
     * the `connect' function is called with the same arguments. Refer
     * to `connect' for further information.
     */
    TCPSocket (std::string const& host, int port);

    ~TCPSocket (void);

    /**
     * Sets private class variables to default values.
     */
    void init();

    /**
     * The `connect' function initiates a connection to the socket whose
     * address is specified by the "host" and "port" arguments. This
     * socket is typically on another machine, and it must be already set
     * up as a server. The "host" argument is assumed to be in the
     * standard dots-and-numbers format.
     *
     * Might return before socket is ready for send or receive. The send
     * and receive functions will wait if necessary.
     */
    void connect (std::string const& host, int port);

    /**
     * The `set_connect_timeout' function allows to define a timeout in
     * milli seconds for the connect function. The connect function will
     * fail if no connection has been established if the timeout expires.
     */
    void set_connect_timeout (std::size_t timeout_ms);

    /**
     * Read bytes from the socket. Waits until data is available, or until
     * timeout ms, before returning.
     */
    std::size_t read (unsigned char * buffer, std::size_t max_bytes);
    
    /**
     * Writes bytes to the socket. Waits until socket is ready for writing,
     * or until timeout ms, before returning.
     */
    std::size_t write (const unsigned char * buffer, std::size_t bytes);

  private:
    std::size_t timeout; // In milliseconds
    CURL * curl_handle;
    bool is_connected;
};

#endif /* NET_TCP_SOCKET_HEADER */
