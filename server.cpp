#include<asio.hpp>
#include<memory>
#include<iostream>
#include<thread>
#include<string>
#include<unordered_map>
#include<mutex>

using namespace std;

class Session: public enable_shared_from_this<Session>{
public:
	Session(asio::ip::tcp::socket socket, string id,function<void(const string&)>on_disconnect): socket_(move(socket)), client_id_(move(id)), on_disconnect_(move(on_disconnect)){}

	void start(){
		cout<<"Client connected: "<<client_id_<<endl;
		send_message("Welcome, " + client_id_ + "!");
		read_message();
	}

	void send_message(const string& message){
		auto self = shared_from_this();
		asio::async_write(
			socket_,
			asio::buffer(message + "\n"),
			[this, self](error_code ec, size_t){
				if(ec){
					cerr<<"Error sending message to"<<client_id_<<":"<<ec.message()<<endl;
				}
			});
	}

	string get_id() const { return client_id_; }

	void stop(){
		send_message("Server disconnecting you:");
		error_code ec;
		socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
		socket_.close(ec);
		if(ec){
			cerr<<"Error closing connection for "<<client_id_<<":"<<ec.message()<<endl;
		}else{
			cout<<"Client "<<client_id_<<"disconnected by server: "<<endl;
		}

		on_disconnect_(client_id_);
	}
private:
	void read_message(){
		auto self = shared_from_this();
		socket_.async_read_some(
			asio::buffer(data_),
			[this, self](error_code ec, size_t length){
				if(!ec){
					string received_message(data_, length);
					cout<<"Message from "<<client_id_<<":"<<received_message;

					//Echo message back to client
					send_message("Server: " + received_message);

					read_message();//continue reading
				}else if(ec == asio::error::eof){
					cout<<"Client "<<client_id_ <<"diconnected. "<<endl;
					on_disconnect_(client_id_);
				}
	else{
					cerr<<"Error reading from "<<client_id_<<":"<<ec.message()<<endl;
					on_disconnect_(client_id_);
				}
			});
	}
	asio::ip::tcp::socket socket_;
	string client_id_;
	function<void(const string&)> on_disconnect_;
	char data_[1024] = {0};
};

class Server{
public:
	Server(asio::io_context& io_context, short port):acceptor_(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)){
		accept_connection();

		server_input_thread_ = thread([this](){
			handle_server_commands();
				});
	}

	~Server() {
		if(server_input_thread_.joinable()){
			server_input_thread_.join();
		}
	}
private:
	void accept_connection(){
		acceptor_.async_accept(
			[this](error_code ec, asio::ip::tcp::socket socket){
				if(!ec){
					auto endpoint = socket.remote_endpoint();
					string client_id = endpoint.address().to_string()+":"+to_string(endpoint.port());
					auto session = make_shared<Session>(
				move(socket),
				client_id,
				[this](const string& id){
					remove_client(id);
				});
			{
				lock_guard<mutex> lock(clients_mutex_);
				clients_[client_id] = session;
					}

					session -> start();
				}
				else{
					cerr<<"Accept error: "<<ec.message()<<endl;
				}

				accept_connection();
	});
	}
	void remove_client(const string& client_id){
		lock_guard<mutex> lock(clients_mutex_);
		clients_.erase(client_id);
		cout<<"Client "<<client_id<<"removed from server. "<<endl;
	}

	void broadcast_message(const string& message){
		lock_guard<mutex> lock(clients_mutex_);
		for(auto& [id,session] : clients_){
			session -> send_message("Server (to all): " + message);
		}
	}

	void send_message_to_client(const string& client_id, const string& message){
		lock_guard<mutex> lock(clients_mutex_);
		auto it = clients_.find(client_id);
		if(it != clients_.end()){
			it -> second -> send_message("Server (to you): " + message);
		}
		else{
			cout<<"Client "<<client_id <<"not found."<<endl;
		}
	}

	void disconnect_client(const string& client_id){
		lock_guard<mutex> lock(clients_mutex_);
		auto it = clients_.find(client_id);
		if(it!=clients_.end()){
			it->second ->stop();
			clients_.erase(it);
		}
		else{
			cout<<"Client "<<client_id<<"not found. "<<endl;
		}
	}

	void handle_server_commands(){
		string command;
		while(getline(cin, command)){
			if(command == "exit"){
				break;
			}
			else if (command.find("broadcast:")== 0){
				broadcast_message(command.substr(10));
			}
			else if(command.find("send:") == 0){
				auto space_pos = command.find(' ', 5);
				if(space_pos != string::npos){
					string client_id = command.substr(5, space_pos -5);
					string message = command.substr(space_pos + 1);
					send_message_to_client(client_id, message);
				}
			}
			else if(command.find("disconnect: ")==0){
				string client_id = command.substr(11);
				disconnect_client(client_id);
			}
			else{
				cout<<"unknown command."<<endl;
			}
		}
	}

	asio::ip::tcp::acceptor acceptor_;
	unordered_map<string, shared_ptr<Session>> clients_;
	mutex clients_mutex_;
	thread server_input_thread_;
};

int main(){
	try{
asio::io_context io_context;

		Server server(io_context, 8080);

		cout<<"Server is running on port 8080."<<endl;
		io_context.run();
	}catch(const exception& e){
		cerr<<"Error: "<<e.what()<<endl;
	}

	return 0;
}


