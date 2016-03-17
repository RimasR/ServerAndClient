/*
	Remember to link to : wsock32.lib
*/
using namespace std;
// INCLUDE FILES //

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

// DEFINITIONS //

#define		PORT 4000		// Define the port to connect to
#define		MAX_CLIENTS 10	// Define the maximum number of clients we can receive
#define		BUFFER_SIZE 256 // Define the buffer size of the messages

// STRUCTURES //

struct _client
{
	bool		connected;
	sockaddr_in	address;
	int	socket;
	fd_set		socket_data;
	socklen_t	address_length;

	char		template_name[15];
	char		screen_name[15];
	char		siegepos[45];
};

// GLOBAL VARIABLES //

sockaddr_in	server_address;
sockaddr	server_socket_address;
int		server_socket;

_client		client[MAX_CLIENTS];
int			clients_connected = 0;

// FUNCTION DECLARATIONS //

bool	accept_client ( _client *current_client );
int		accept_connections ();
int		disconnect_client ( _client *current_client );
void	echo_message ( char *message );
void	end_server();
void	midcopy ( char* input, char* output, int start_pos, int stop_pos );
int		receive_client ( _client *current_client, char *buffer, int size );
void	receive_data();
int		send_data ( _client *current_client, char *buffer, int size );
void	start_server();

// FUNCTION DEFINITIONS //

bool accept_client ( _client *current_client )
{
	// Accept incoming connections
	current_client->address_length = sizeof ( sockaddr );
	current_client->socket = accept ( server_socket, ( sockaddr * ) &current_client->address, &current_client->address_length );

	if ( current_client->socket == 0 )
	{
		// No data in socket
		return ( false );
	}
	else if ( current_client->socket < 0 )
	{
		// Socket error
		return ( false );
	}
	else
	{
		// Occupy the client slot
		current_client->connected = true;
		FD_ZERO ( &current_client->socket_data );
		FD_SET ( current_client->socket, &current_client->socket_data );

		return ( true );
	}

	return ( false );
}

int accept_connections()
{
	if ( clients_connected < MAX_CLIENTS )
	{
		for ( int j = 0; j < MAX_CLIENTS; j++ )
		{
			if ( !client[j].connected )
			{
				if ( accept_client ( &client[j] ) )
				{
					// Increment the client count
					clients_connected++;

					// Grab the ip address of the client ... just for fun
					char *client_ip_address = inet_ntoa ( client[j].address.sin_addr );

					// Output connection
					cout << "ACCEPTING CLIENT to array position [" << j << "] with IP ADDRESS " << client_ip_address << endl;
				}
			}
		}
	}

	return ( 1 );
}

int disconnect_client ( _client *current_client )
{ // Disconnect a client
	if ( current_client->connected == true )
	{ // Close the socket for the client
		close ( current_client->socket );
	}

	// Set the new client state
	current_client->connected = false;

	// Clear the address length
	current_client->address_length = -1;

	// Decrement the current number of connected clients
	clients_connected--;

	// Declare a variable to store the disconnect message into
	char raw_data[BUFFER_SIZE];

	// Parse in the client data to send
	sprintf ( raw_data, "~4 %s", current_client->screen_name );

	// Echo out the disconnect message so all clients drop this client
	//echo_message ( raw_data );

	cout << "Disconnecting client[]" << endl;

	return ( 1 );
}

void echo_message ( char *message )
{
	for ( int j = 0; j < MAX_CLIENTS; j++ )
	{
		if ( client[j].connected )
		{ // Echo the message to all clients
			send_data ( &client[j], message, BUFFER_SIZE );
		}
	}
}

void end_server()
{ // Shut down the server by disconnecting all clients and clearing winsock
	// Disconnect all clients
	for ( int j = 0; j < MAX_CLIENTS, j++;) { disconnect_client ( &client[j] ); }

	// Close the listening socket for the server

	close ( server_socket );

	// Clean up winsock
	#ifdef _WIN32
        WSACleanup();
    #endif
}

void midcopy ( char* input, char* output, int start_pos, int stop_pos )
{
	int index = 0;

	for ( int i = start_pos; i < stop_pos; i++ )
	{
		output[index] = input[i];
		index++;
	}

	output[index] = 0;
}

int receive_client ( _client *current_client, char *buffer, int size )
{
	if ( FD_ISSET ( current_client->socket, &current_client->socket_data ) )
	{
		// Store the return data of what we have sent
		current_client->address_length = recv ( current_client->socket, buffer, size, 0 );

		if ( current_client->address_length == 0 )
		{ // Data error on client
			disconnect_client ( current_client );

			return ( false );
		}

		return ( true );
	}

	return ( false );
}

void receive_data()
{
	char buffer[BUFFER_SIZE];

	for (int j = 0; j < MAX_CLIENTS; j++)
	{
		if (client[j].connected)
		{
			if (receive_client(&client[j], buffer, BUFFER_SIZE))
			{		// Declare the buffer to store new client information into
				cout << "Recieved data: " << buffer << endl;
				char raw_data[BUFFER_SIZE];
				char answer[BUFFER_SIZE];
				midcopy(buffer, raw_data, 0, strlen(buffer));
				long num1 = 0;
				long num2 = 0;
				char* number1 = (char*)malloc(sizeof(10));
				int k = 0;
				char oper;
				sscanf(raw_data, "%d%c%d", &num1, &oper, &num2);
				
				if (oper == '+')
				{
					long full = num1 + num2;
					sprintf(answer, "%d", full);
				}
				else if (oper == '-')
				{
					long full = num1 - num2;
					sprintf(answer, "%d", full);
				}
				else if (oper == '*')
				{
					long full = num1 * num2;
					sprintf(answer, "%d", full);
				}
				else if (oper == '/')
				{
					float full = num1 / num2;
					sprintf(answer, "%f", full);
				}



				cout << "First digit: " << num1 << " Second digit: " << num2 << " Operator: " << oper << " Answer: " << answer << endl;
				
				send_data(&client[j], answer, BUFFER_SIZE);
				buffer[0] = '\0';
				
			}
		}
	}
}

int send_data ( _client *current_client, char *buffer, int size )
{
	// Store the return information about the sending
	current_client->address_length = send ( current_client->socket, buffer, size, 0 );
	cout << "Sent data: " << current_client->address_length << endl;

	if ( ( current_client->address_length < 0 ) || ( current_client->address_length == 0 ) )
	{ // Check for errors while sending
		disconnect_client ( current_client );

		return ( false );
	}
	else return ( true );
}

void start_server()
{ // Initialize the server and start listening for clients
	// LOCAL VARIABLES //
	#ifdef _WIN32
    WSADATA wsaData;
    #endif
	int res, i = 1;

	// Set up the address structure
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons ( PORT);

	// IM GUESSING : Copy over some addresses, conversions of some sort ?
	memcpy ( &server_socket_address, &server_address, sizeof ( server_socket_address ) );
    #ifdef _WIN32
	res = WSAStartup ( MAKEWORD ( 1, 1 ), &wsaData );
	// Start winsock
	if ( res != 0 )
	{
		cout << "WSADATA ERROR : Error while attempting to initialize winsock." << endl;
	}
    #endif
	// Create a listening socket for the server
	server_socket = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	if ( server_socket == 0 )
	{
		cout << "SOCKET ERROR : Invalid socket." << endl;
	}
	else if ( server_socket < 0 )
	{
		cout << "SOCKET ERROR : Socket error." << endl;
	}
	else
	{
		cout << "SOCKET ESTABLISHED" << endl;
	}

	// Sets the option to re-use the address the entire run of the program
	setsockopt ( server_socket, SOL_SOCKET, SO_REUSEADDR, ( char * ) &i, sizeof ( i ) );

	// Bind the socket to the address
	res = bind ( server_socket, &server_socket_address, sizeof ( server_socket_address ) );

	cout << "Binding socket:" << res << endl;

	if ( res != 0 )
	{
		cout << "BINDING ERROR : Failed to bind socket to address." << endl;
	}
	else
	{
		cout << "Socket Bound to port : "<< PORT << endl;
	}

	// Start listening for connection requests
	res = listen ( server_socket, 8 );

	// This makes the server non blocking, hence it won't wait for a response
	unsigned long b = 1;
#ifdef _WIN32
	ioctlsocket ( server_socket, FIONBIO, &b );
#else
	ioctl(server_socket, FIONBIO, &b);
#endif
	// Clear all clients state
	for ( int j = 0; j < MAX_CLIENTS; j++ ) { client[j].connected = false; }
}

////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////

// MAIN FUNCTION //

int main()
{
	cout << "Non-Blocking Multi-Client Echo Server\n" << endl;

	// Initialize winsock and start listening
	start_server();

	// Loop forever
	bool looping = true;

	while ( looping )
	{
		// Accept all incoming client connections
		accept_connections();

		// Receive all data from clients
		receive_data();
	}

	// Shut down winsock
	end_server();

	return 0;
}
