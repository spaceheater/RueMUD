/*
 *  example_client.cpp
 *  A chord-based matchmaking server client.
 *  Connects to server.cpp and can modify data in that chord.  Also
 *  continues the Chord if the initial connection goes down.
 *
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
    char hn[128];
    string hostname(hn);


    gethostname(hn, sizeof hn);
    /* Originally "backbone" */
	
    Node *chord = NULL;
    ChordNode *node = NULL;
    
    //This will always join to the mimosa.cs.uchicago.edu chord.
    cout << "client now joining...\n";
    int i = 0;
    node = P_SINGLETON->initChordNode(std::string("mimosa.cs.uchicago.edu"), 
			   8000, std::string("ruemud"),std::string("."));
    chord = new Node(hostname, 8000);
    node->join(chord);

    char entry[256];
    string key;
    string value;
		
    while (1) { // sleep...
	cout << "\n0) Print status\n" << 
		  "1) Put\n" << 
	  	  "2) Get\n" <<
	 	  "3) Remove\n" << 
		  "4) Exit\n\n";
	cout << "---> ";
	cin >> entry;
	int chx = atoi(entry);

	switch (chx) {
    	case 0:
    		cout << "\n" << node->printStatus();
    		break;
    	case 1:
    		cout << "Key = ";
    		cin >> key;
    		cout << "Value = ";
    		cin >> value;
    		node->put(key, value);
    		break;
    	case 2:
    		cout << "Key = ";
    		cin >> key;
    		cout << "\n" << node->get(key) << "------> found!" << endl;
    		break;
    	case 3:
    		cout << "Key = ";
    		cin >> key;
    		node->removekey(key);
    		break;
    	case 4:
    		node->shutDown();
    		default:
		break;
	}       
	}

    }
