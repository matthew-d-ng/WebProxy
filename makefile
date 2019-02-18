webproxy: console.cpp proxy.cpp hostcontact.cpp
	g++ -o webproxy console.cpp proxy.cpp hostcontact.cpp -pthread