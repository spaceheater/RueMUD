RueMUD
======

RueMUD is a P2P MUD created for CMSC 23310 at the University of Chicago.

Massively-multiplayer online games are a very popular (and currently, profitable) example of distributed systems. The common massively-multiplayer online game typically consists of one or more servers and many connecting clients. However, such a system has its caveats: servers can be single points of failure for the game overall, and individual server performance bottlenecks the number of players that are able to connect at one time, and the server/client system proves to be more expensive for the startup attempting to work its way into the realm of massively multiplayer online games.

We propose that a peer to peer (P2P) implementation be revisited. While such an idea is poor for some sorts of games (WoW, Everquest) due to the amount of data that would need to be transferred, P2P implementations might be able to provide a cheaper and lower maintenance setup for casual browser games where a client/server setup would be clunky and even inefficient.

Direct connections via P2P services are already used in some games, more specifically in MUDs (Multi-User Dungeons) where chat often takes place over a P2P connection. If chat can take place over P2P, why not the entire game? Thus, in order to prove the usefulness of P2P for browser/casual games, we will change a small MUD called MURK++ to use a P2P connection format using the strategies outlined in the paper "MOPAR: a mobile peer-to-peer overlay architecture for interest management of massively multiplayer online games" and the DHT implementation Chord.

MURK++ is a C++ implementation of MercMUD, a simple MUD released in 1992 and based off of DikuMUD, which "became the root of one of the largest trees of derived code from a MUD-like source code package" after its release. The MUD, as most, uses a server and client model to allow a player to telnet into a server and play the MUD this way. Game state information per player is saved on the server. Because of the qualities of peer to peer data transfer, our version of MURK++ will require game state information to be saved on the players' computers. A bit different from the traditional MUD, but necessary for our purposes.

MOPAR is the system of P2P gameplay outlined in "MOPAR: A Mobile Peer-to-Peer Overlay Architecture for Interest Management of Massively Multiplayer Online Games." Taking the advantages present in both DHT overlay and unstructured P2P architecture, MOPAR divides a game map into hexagonal zones, uses a DHT overlay to map each cell to a determined node, and assigns particular game participants "master node" privelleges, allowing them to keep track of the movements/departures/entrances of the other game participants. AOI is fixed, because of the properties of MUDs (a character must make a request to look at a given thing).

The order of events is as follows:
The participant joins the game. The coordinates of the participant are mapped to the Cell ID for his room.
Hash the CellID
Query the DHT to obtain the correct node in the table who is numerically closest to the hash. This is the home node for the character. (IE if the character begins in the first room, the DHT will be able to tell the character this is where he is).
If the home node has no master node, the player logging in becomes the master node. Otherwise, he remains a slave node (normal player node).
The master node queries the home nodes of the neighboring cells to build direct connections with the neighboring masters.
Unstructured P2P is used to discover neighbors in a hierarchal fashion. Neighbor lists are exchanged by master nodes, and slave nodes are only notified if there is some change in list.

Advantages of this setup include:
Improved Scalability
Cheaper for Startups
Improved Fault Tolerance (no longer a single point of failure (the server))
The authors continue their work in a longer thesis as well.

About Chord
Chord is a DHT implemented at MIT, and is considered one of the four original distributed hash tables. It was implemented in C++, and although the developers of MOPAR used Pastry, as MURK is written in C++, we chose to use Chord.

Deliverables
By our first check-in, we had a basic idea of what we wanted to and code that we wished to modify.

By the second check-in, we hope to have a modified version of MURK++ working with the chord architecture. Debugging and optimization may not be completed.

By the presentation, we should have our final debugged version of MURK++ working, as well as (if all goes well) a visual representation of the system resources saved by working with P2P-based multiplayer versus Client/Server-based games..

