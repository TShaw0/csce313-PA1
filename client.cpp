/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name: Trenton Shaw
	UIN: 534007138
	Date: 9/18/25
*/
#include "common.h"
#include "FIFORequestChannel.h"
#include <sys/wait.h>
using namespace std;


int main (int argc, char *argv[]) {
  int pid = fork();
  if (pid == 0){//instance is child
    char* args[] = { (char*)"./server",NULL};
    execvp(args[0], args);
  }
  else {//instance is parent
    usleep(100000);
    FIFORequestChannel* control_channel = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
    vector<FIFORequestChannel*> data_channels;
    MESSAGE_TYPE new_channel_msg = NEWCHANNEL_MSG;
    control_channel->cwrite(&new_channel_msg, sizeof(new_channel_msg));
    
    char new_channel_name[1024];
    control_channel->cread(new_channel_name, sizeof(new_channel_name));

    FIFORequestChannel* new_data_channel = new FIFORequestChannel(string(new_channel_name), FIFORequestChannel::CLIENT_SIDE);
    data_channels.push_back(new_data_channel);

    datamsg dmsg(1, .004, 1);
    new_data_channel->cwrite(&dmsg, sizeof(datamsg));

    double data;
    new_data_channel->cread(&data, sizeof(double));
    cout << "Recieved data: " << data << endl;
    
    MESSAGE_TYPE q = QUIT_MSG;

    for (auto ch : data_channels){
      ch->cwrite(&q,sizeof(q));
      delete ch;
    }
    data_channels.clear();

    control_channel->cwrite(&q, sizeof(q));
    delete control_channel;


    wait(NULL);

    cout <<"Client-side is done and exited" << endl;
    cout << "Server terminated" << endl;
  }
    

  int opt;
  int p = 1;
  double t = 0.0;
  int e = 1;
  
  string filename = "";
  while ((opt = getopt(argc, argv, "p:t:e:f:")) != -1) {
    switch (opt) {
    case 'p':
      p = atoi (optarg);
      break;
    case 't':
      t = atof (optarg);
      break;
    case 'e':
      e = atoi (optarg);
      break;
    case 'f':
      filename = optarg;
      break;
    }
  }

  //FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);
  // example data point request
  char buf[MAX_MESSAGE]; // 256
  datamsg x(1, 0.0, 1);
  
  memcpy(buf, &x, sizeof(datamsg));
  chan.cwrite(buf, sizeof(datamsg)); // question
  double reply;
  chan.cread(&reply, sizeof(double)); //answer
  cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
  
  // sending a non-sense message, you need to change this
  filemsg fm(0, 0);
  string fname = "teslkansdlkjflasjdf.dat";
  
  int len = sizeof(filemsg) + (fname.size() + 1);
  char* buf2 = new char[len];
  memcpy(buf2, &fm, sizeof(filemsg));
  strcpy(buf2 + sizeof(filemsg), fname.c_str());
  chan.cwrite(buf2, len);  // I want the file length;
  
  delete[] buf2;
  
  // closing the channel    
  MESSAGE_TYPE m = QUIT_MSG;
  chan.cwrite(&m, sizeof(MESSAGE_TYPE));
}
