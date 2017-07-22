/**
 * Provides a TCP socket connection to an address that is persistent. When there is a
 * disconnect or error, this transparently reconnects on the next socket operation
 * and keeps on going as though nothing went wrong. All sockets used are non-blocking.
 *
 * By its very nature, persistent sockets hide errors, so the functions don't
 * typically return errors. This is essentially a UDP socket that has guaranteed delivery
 * while connected.
 *
 * @file rsocket.h
 * @author baoxue <huiqi.qian@sihua.com>
 * @copyright 2017
 */

/**
 * Everything behind the reconnecting socket
 */
typedef struct rsocket rsocket_t;

int rsocket_open(const char *host, const int port, struct rsocket *rsocket);

/**
 * Create a new connection.
 *
 * @param host The hostname to connect to.
 * @param port The port to connect to.
 * @param[out] rsocket Where the new reconnecting socket should be put.
 *
 * @return 0 on success.
 * @return -1 on address lookup error.
 */
int rsocket_connect(struct rsocket *rsocket);

/**
 * Closes a rsocket connection.
 *
 * @param rsocket The reconnecting socket to close.
 */
void rsocket_close(struct rsocket *rsocket);

/**
 * Send some data out to the server.
 *
 * @param rsocket The reconnecting socket
 * @param msg The message to send
 * @param len The length of the message
 */
int rsocket_send(struct rsocket *rsocket, char *buff, int len);

/**
 * Read data from the server.
 *
 * @param rsocket The socket to read from
 * @param buff Where the data should be put
 * @param len The length of the data buffer
 *
 * @return The length of data read from the socket, 0 or greater.
 */
int rsocket_recv(struct rsocket *rsocket, char *buff, size_t len);
