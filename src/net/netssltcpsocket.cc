#include <iostream>
#ifdef WIN32
# include <winsock2.h>
#endif
#include <openssl/err.h>

#include "util/helpers.h"
#include "util/exception.h"

#include "networking.h"
#include "netssltcpsocket.h"

NET_NAMESPACE_BEGIN


SSLTCPSocket::SSLTCPSocket (void)
    : TCPSocket(), ssl(0), sbio(0)
{
}

/* ---------------------------------------------------------------- */

SSLTCPSocket::SSLTCPSocket (std::string const& host, int port)
    : TCPSocket(), ssl(0), sbio(0)
{
    this->connect(host, port);
}

/* ---------------------------------------------------------------- */

SSLTCPSocket::~SSLTCPSocket (void)
{
}

/* ---------------------------------------------------------------- */

void
SSLTCPSocket::ssl_handshake (void)
{
    /* Setup SSL object. */
    this->ssl = SSL_new(ssl_context());
    this->sbio = BIO_new_socket(this->sock, BIO_NOCLOSE);
    SSL_set_bio(ssl, this->sbio, this->sbio);

    //std::cout << "Establishing SSL connection..." << std::endl;

    /* Connect to host (initialize handshake). */
    int sslret = SSL_connect(this->ssl);
    if (sslret <= 0)
    {
        std::cout << "SSL error connecting!" << std::endl;
        ERR_print_errors_fp(stdout);
        //std::cout << "  Error code " << errcode << ": "
        //    << ERR_reason_error_string(errcode) << std::endl;
        this->close();
        throw Exception("SSL error connecting.");
    }
    this->check_cert();
}

/* ---------------------------------------------------------------- */

std::size_t
SSLTCPSocket::write (void const* buffer, std::size_t size, std::size_t offset)
{
    int ret = SSL_write(this->ssl, (char const*)buffer + offset, (int)size);
    if (ret <= 0)
    {
        int err = SSL_get_error(this->ssl, ret);
        std::cout << "Error writing to SSL!" << std::endl
            << "  Error code: " << err << ": "
            << ERR_reason_error_string(err) << std::endl;
        throw Exception("Could not write");
    }

    return ret;
}

/* ---------------------------------------------------------------- */

std::size_t
SSLTCPSocket::read (void* buffer, std::size_t size, std::size_t offset)
{
    int ret = SSL_read(this->ssl, (char*)buffer + offset, (int)size);
    if (ret <= 0)
    {
        int err = SSL_get_error(this->ssl, ret);
        std::cout << "Error writing to SSL!" << std::endl
            << "  Error code: " << err << ": "
            << ERR_reason_error_string(err) << std::endl;
        throw Exception("Could not read");
    }

    return ret;
}

/* ---------------------------------------------------------------- */

void
SSLTCPSocket::close (void)
{
#if 1
    if (this->ssl)
    {
        SSL_shutdown(this->ssl);
        SSL_free(this->ssl);
        //BIO_free(this->sbio); // SSL_free is already freeing
        this->ssl = 0;
        this->sbio = 0;
    }

    this->TCPSocket::close();
#else
    /* Close down SSL connection first... */
    std::cout << "Closing connection... SSL" << std::flush;
    int ret = SSL_shutdown(this->ssl);
    if (ret < 0)
        std::cout << " (error)" << std::flush;

    /* ... now close the TCP connection. */
    std::cout << ", TCP" << std::flush;
    this->TCPSocket::close();
    std::cout << "." << std::endl;
#endif
}

/* ---------------------------------------------------------------- */

bool
SSLTCPSocket::match_hostname (std::string const& ca, std::string const& host)
{
    // Will not work for wildcard certs.
    //if (::strcmp(peer_CN, host.c_str()))
    //    throw Exception("Common name does not match host name!");

    if (host.size() < ca.size())
        return false;

    std::string::const_reverse_iterator i_ca = ca.rbegin();
    std::string::const_reverse_iterator i_host = host.rbegin();
    while (i_ca != ca.rend())
    {
        if (*i_ca == '*')
            return true;
        if (*i_ca != *i_host)
            return false;
        i_ca++; i_host++;
    }

    return true;
}

/* ---------------------------------------------------------------- */

void
SSLTCPSocket::check_cert (void)
{
    //std::cout << "Checking certificate..." << std::endl;

    /* Check if server certificate is properly signed. */
    long vcode = SSL_get_verify_result(this->ssl);
    if (vcode != X509_V_OK)
    {
        std::cout << "Error: The certificate did not verify!"
            << " Code " << vcode << std::endl;

        /* Certificate debugging. */
        X509* peer_cert = SSL_get_peer_certificate(this->ssl);
        X509_NAME* peer_name = X509_get_subject_name(peer_cert);
        X509_NAME* issuer_name = X509_get_issuer_name(peer_cert);
        X509_NAME_print_ex_fp(stdout, peer_name, 0, 0);
        std::cout << std::endl;
        X509_NAME_print_ex_fp(stdout, issuer_name, 0, 0);
        std::cout << std::endl;

        //SSL_shutdown(this->ssl);
        this->close();
        throw Exception("Certificate did not verify! Code: "
            + Helpers::get_string_from_int((int)vcode));
    }

    /* Verify common name of certificate. */
    if (!this->hostname.empty())
    {
        std::string peer_cn;
        {
            X509* peer = SSL_get_peer_certificate(this->ssl);
            char peer_CN[256];
            X509_NAME_get_text_by_NID(X509_get_subject_name(peer),
                NID_commonName, peer_CN, 256);
            peer_cn = peer_CN;
        }

        if (!this->match_hostname(peer_cn, this->hostname))
        {
            std::cout << "Refusing connection to " << this->hostname << std::endl;
            std::cout << "  Certificate CN: " << peer_cn << std::endl;
            std::cout << "  Hostname: " << this->hostname << std::endl;
            std::cout << "  The CN and hostname do not match. This may "
                << "be an attempt to trick you!" << std::endl;

            this->close();
            throw Exception("Certificate common name CN=" + peer_cn
                + " does not match host name " + this->hostname + ". "
                "This may be an attempt to trick you!");
        }
    }
}

NET_NAMESPACE_END
