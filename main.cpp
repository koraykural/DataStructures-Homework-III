/* @Author
Student Name: Bekir Koray Kural
Student ID: 150170053
Date: 13.11.2019 */

#include <iostream>
#include <fstream>
#include <stdlib.h> 
#include <stdio.h>

using namespace std;


struct MessageNode {
  int target; // Target mobile host
  string text;
  MessageNode *next_message; // Next MessageNode in the queue
};

struct MobileNode {
  int ID;
  MobileNode *next_mobile;
};

struct BaseNode {
  int ID;
  BaseNode *first_child; // Every BaseNode knows only its first child
  BaseNode *next_sibling; // Program can access other childs from its previous siblings
  MobileNode *first_mobile; // Also BaseNode knows only first mobile host connected to it
};

struct MessageQueue {
  MessageNode *head;
  MessageNode *tail;
  MessageNode* pop();
  void push(MessageNode*);
  void add_from_file(char*);
  bool is_empty();
} message_queue;

struct Network {
  BaseNode central_controller;
  void add_base(int,int);
  void add_mobile(int,int);
  void send_message(MessageNode);
  void clear_network(BaseNode*);
  void create_network_from_file(char*);
  BaseNode* find_base(BaseNode*,int);
  string* path_to_MH(BaseNode*,int);
} network;


void MessageQueue::add_from_file(char* file_path) {
	ifstream in(file_path);
	if(!in.is_open())
		cout<<"ERROR: File could not been open"<<endl;

	while(in.peek() != EOF )
	{		
		string message_text, target;
		getline(in,message_text,'>');
		getline(in,target,'\n');
    if(target == "") continue;
    MessageNode* new_message = new MessageNode();
    new_message->next_message = NULL;

		char cstr[message_text.size() + 1];
		message_text.copy(cstr, message_text.size() + 1);
		cstr[message_text.size()] = '\0';
		new_message->text=cstr;
		new_message->target=stoi(target);

    message_queue.push(new_message);
	}
	in.close();
}

bool MessageQueue::is_empty() {
  return head==NULL;
}

MessageNode* MessageQueue::pop() {
  MessageNode* node_to_return;
  node_to_return = head;
  message_queue.head = head->next_message;
  return node_to_return;
}

void MessageQueue::push(MessageNode* new_message) {
  if(head == NULL) {
    head = new_message;
    tail = new_message;
    return;
  }
  else {
    tail->next_message = new_message;
    tail = new_message;
    return;
  }
}


void Network::create_network_from_file(char* file_path) {
  // Open network file to read
  FILE *network_data;
  if ( !(network_data = fopen( file_path, "r+" ) ) ) {
    cerr << "File could not be opened" << endl;
    exit(1);
	}

  // Read network file and add bases and mobiles
  char node_type[3];
  int ID;
  int parent_ID;
  while (1) {  
		fscanf(network_data, "%s %d %d", &node_type[0], &ID, &parent_ID); 
    if(node_type[0] == 'B') {
      network.add_base(ID, parent_ID);
    }
    else if(node_type[0] == 'M') {
      network.add_mobile(ID, parent_ID);
    }
    if ( feof(network_data) ) break;
	}
  fclose(network_data);
}

BaseNode* Network::find_base(BaseNode* current_base, int ID) {
  // Base is found. Return it to upper levels in tree
  if(current_base->ID == ID) {
    return current_base;
  }

  // Base not found yet. Go to child (left most child)
  if(current_base->first_child) {
    BaseNode* returned_node = network.find_base(current_base->first_child, ID);

    // Base found deeper in this branch. Return it
    if(returned_node) {
      return returned_node;
    }
  }

  // Base not found yet. Go to sibling (Right child of parent)
  if(current_base->next_sibling) {
    BaseNode* returned_node = network.find_base(current_base->next_sibling, ID);

    // Base found deeper in this branch. Return it
    if(returned_node) {
      return returned_node;
    }
  }

  // Base not found in this branch. Return NULL so that it continues searching
  return NULL;
}

void Network::add_base(int ID, int parent_ID) {
  // Get adress of parent node
  BaseNode* parent_node = network.find_base(&central_controller, parent_ID);

  BaseNode* new_base = new BaseNode();
  new_base->ID = ID;
  new_base->first_child = NULL;
  new_base->first_mobile = NULL;
  new_base->next_sibling = NULL;

  if(parent_node->first_child == NULL) {
    // New node is the first child
    parent_node->first_child = new_base;
  }
  else {
    // Get the latest sibling of new node 
    BaseNode* previous_sibling = parent_node->first_child;
    while(previous_sibling->next_sibling) {
      previous_sibling = previous_sibling->next_sibling;
    }
    previous_sibling->next_sibling = new_base;
  }
}

void Network::add_mobile(int ID, int parent_ID) {
  // Get adress of base
  BaseNode* base_node = network.find_base(&central_controller, parent_ID);

  MobileNode* new_mobile = new MobileNode();
  new_mobile->ID = ID;
  new_mobile->next_mobile = NULL;
  if(base_node->first_mobile == NULL) {
    // New mobile is the first mobile connected to that base
    base_node->first_mobile = new_mobile;
  }
  else {
    // Get the latest mobile connected to that base
    MobileNode* previous_mobile = base_node->first_mobile;
    while(previous_mobile->next_mobile) {
      previous_mobile = previous_mobile->next_mobile;
    }
    previous_mobile->next_mobile = new_mobile;
  }
}

string* Network::path_to_MH(BaseNode* current_base, int mobile_ID) {
  // Print which base program currently traversing
  cout << current_base->ID << " ";

  // Check MHs connected to this base
  if(current_base->first_mobile) {
    MobileNode* mobile_node = new MobileNode();
    mobile_node = current_base->first_mobile;
    while(mobile_node) {
      if(mobile_node->ID == mobile_ID) {
        // MH found, start creating path from the furthest base to CC and return it
        string* path_to_MH = new string;
        *path_to_MH += to_string(current_base->ID);
        return path_to_MH;
      }
      mobile_node = mobile_node->next_mobile;
    }
  }

  // Mobile Host not found yet. Go to child (left most child)
  if(current_base->first_child) {
    string* path_to_MH = network.path_to_MH(current_base->first_child, mobile_ID);
    if(*path_to_MH != "") {
      // MH found deeper in this branch. Add current base_ID to path and return it
      *path_to_MH = to_string(current_base->ID) + " " + *path_to_MH;
      return path_to_MH;
    }
    else {
      delete path_to_MH;
    }
  }

  // Mobile host not found yet. Go to sibling (Right child of parent)
  if(current_base->next_sibling) {
    string* path_to_MH = network.path_to_MH(current_base->next_sibling, mobile_ID);
    if(*path_to_MH != "") {
      // MH is found in siblings. Return it without changing it
      return path_to_MH;
    }
    else {
      delete path_to_MH;
    }
  }

  // Mobile host not found in this branch. Return NULL so that it continues searching
  string* empty_string = new string;
  *empty_string = "";
  return empty_string;
}

void Network::send_message(MessageNode message_data) {
  cout << "Traversing:";
  // Traverse tree recursively and return path to mobile host
  string* path_to_MH = network.path_to_MH(&central_controller, message_data.target);
  cout << endl;

  if(*path_to_MH != "")
    cout << "Message:" << message_data.text << " To:" << *path_to_MH << " mh_" << message_data.target << endl;
  else 
    cout << "Can not be reached the mobile host mh_" << message_data.target << " at the moment" << endl;

  delete path_to_MH;
}

void Network::clear_network(BaseNode* current_base) {
  if(current_base->first_child)
    network.clear_network(current_base->first_child);

  if(current_base->next_sibling)
    network.clear_network(current_base->next_sibling);

  if(current_base != &central_controller)
    delete current_base;
}


int main(int argc, char *argv[]) {

  // Get file names from command line
  if(argc != 3) {
    cerr << "Two argument is expected!" << endl;
			exit(1);
  }
  char* network_filename = argv[1];
  char* messages_filename = argv[2];

  // Create network
  network.central_controller.ID = 0;
  network.create_network_from_file(network_filename);

  // Create message queue
  message_queue.add_from_file(messages_filename);

  while(!message_queue.is_empty()) {
    MessageNode* message_to_send = message_queue.pop();
    network.send_message(*message_to_send);
    delete message_to_send;
  }

  network.clear_network(&network.central_controller);
  return EXIT_SUCCESS;
}