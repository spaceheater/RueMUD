/*
 *  example_client2.cpp
 *  A chord-based matchmaking server client.
 *  Connects to server.cpp and can modify data in that chord.  Also
 *  continues the Chord if the initial connection goes down.  This one
 *  automatically adds a random key and value.
 *
 *  CMSC 23310 - Drew Williams and Rina Kelly, 2012
 *  cChord created by Laurent Vanni & Nicolas Goles Domic, 2010
 *
 */

#include <iostream>
#include <string>
#include "../chord/ChordNode.h"
#include <pthread.h>
#include "../chord/ProtocolSingleton.h"
#include <cstdlib> 
#include <ctime> 
#include <iostream>

using namespace std;

/*  need args, "ip", "port", "overlay identifier 
 *  (unique string)")" 
 */
 
int main() 
{
    string hostname = "127.0.0.1";

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

	srand((unsigned)time(0)); 		
	key = (int)rand(); 
	srand((unsigned)time(0)); 		
	value = rand(); 
    node->put(key, value);
 	node->shutDown();
    
}
