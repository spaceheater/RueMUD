/*
 *  server.cpp
 *  A chord-based matchmaking server
 *  CMSC 23310 - Drew Williams and Rina Kelly, 2012
 *  cChord created by Laurent Vanni & Nicolas Goles Domic, 2010
 *
 */

#include <iostream>
#include <string>
#include <stdlib.h>
#include "../chord/ChordNode.h"
#include <pthread.h>
#include "../chord/ProtocolSingleton.h"

using namespace std;

/*  need args, "ip", "port", "overlay identifier 
 *  (unique string)")" 
 */
 
int main() 
{
	
    Node *chord = NULL;
    ChordNode *node = NULL;
	//We are hardcoding in values to connect from.  This makes our other
	//stuff easier....
	
	//our chord server will always be run from mimosa.cs.uchicago.edu
	//port: 8000
	//"overlay identifier" - ruemud

	node = P_SINGLETON->initChordNode(std::string("mimosa.cs.uchicago.edu"), 
		   8000, std::string("ruemud"),std::string("."));
    chord = NULL;
        
	// since this is the server, we don't join to an existing chord

	char entry[256];
	string key;
	string value;
		
	while (1) { // sleep...
		cout << "\n0) Print status\n" << 
				"1) Exit\n\n";
		cout << "---> ";
		//All the server does is start the connection.  :)  It is 
		//assumed to always be running!
		cin >> entry;
		int chx = atoi(entry);

		switch (chx) {
    		case 0:
    			cout << "\n" << node->printStatus();
    			break;
    		case 1:
    			node->shutDown();
    		default:
			break;
		}       
	}
	return 0;
}
