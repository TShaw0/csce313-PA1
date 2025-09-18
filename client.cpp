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
  int opt;
  bool p_set = false, t_set = false, e_set = false, f_set = false;
  int p = 1;
  double t = 0.0;
  int e = 1;
  string filename = "";
  
  while ((opt = getopt(argc, argv, "p:t:e:f:c")) != -1) {
    switch (opt) {
    case 'p':
      p = atoi (optarg);
      p_set = true;
      break;
    case 't':
      t = atof (optarg);
      t_set = true;
      break;
    case 'e':
      e = atoi (optarg);
      e_set = true;
      break;
    case 'f':
      filename = optarg;
      f_set = true;
      break;
    }
  }
  
  pid_t pid = fork();
  if (pid == 0){//instance is child
    char* args[] = { (char*)"./server",NULL};
    execvp(args[0], args);
  }
  else {//instance is parent
    usleep(100000);
    FIFORequestChannel* control_channel = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
    vector<FIFORequestChannel*> data_channels;

    if (!p_set && !t_set && !e_set && !f_set){
      cout << "Client-side is done and exited" << endl;
      cout << "Server terminated" << endl;
    }
    else if (p_set && t_set && e_set) {
    // Test 2: single ECG reading
    datamsg d(p, t, e);
    control_channel->cwrite(&d, sizeof(datamsg));
    double result;
    control_channel->cread(&result, sizeof(double));
    cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << result << endl;
    }
    else if (p_set && !t_set && !e_set && !f_set) {
    system("mkdir -p received");
    string out_path = "received/x" + to_string(p) + ".csv";
    ofstream outfile(out_path);
    for (int i = 0; i < 1000; ++i) {
        double t_val = i * 0.004;
        datamsg d(p, t_val, 1);
        control_channel->cwrite(&d, sizeof(datamsg));
        double result;
        control_channel->cread(&result, sizeof(double));
        outfile << result << endl;
    }
    outfile.close();
}
else if (f_set) {
    // File transfer logic
    filemsg fm(0,0);
    int msg_size = sizeof(filemsg) + filename.size() + 1;
    char* request = new char[msg_size];
    memcpy(request, &fm, sizeof(filemsg));
    strcpy(request + sizeof(filemsg), filename.c_str());
    control_channel->cwrite(request, msg_size);
    __int64_t file_size;
    control_channel->cread(&file_size, sizeof(file_size));

    system("mkdir -p received");
    string out_path = "received/" + filename;
    ofstream outfile(out_path, ios::binary);
    __int64_t remaining = file_size;
    __int64_t offset = 0;
    while (remaining > 0) {
        int chunk = min((__int64_t)MAX_MESSAGE, remaining);
        filemsg fm_chunk(offset, chunk);
        int msg_len = sizeof(filemsg) + filename.size() + 1;
        char* msg_buf = new char[msg_len];
        memcpy(msg_buf, &fm_chunk, sizeof(filemsg));
        strcpy(msg_buf + sizeof(filemsg), filename.c_str());
        control_channel->cwrite(msg_buf, msg_len);
        char* response_buf = new char[chunk];
        int bytes_read = control_channel->cread(response_buf, chunk);
        outfile.write(response_buf, bytes_read);
        delete[] msg_buf;
        delete[] response_buf;
        offset += bytes_read;
        remaining -= bytes_read;
    }
    outfile.close();
    delete[] request;
}

    /*   
    if (filename != "") {
      filemsg fm(0,0);
      int msg_size = sizeof(filemsg) + filename.size() + 1;
      char* request = new char[msg_size];
      memcpy(request, &fm, sizeof(filemsg));
      strcpy(request + sizeof(filemsg), filename.c_str());
      control_channel->cwrite(request, msg_size);
      __int64_t file_size;
      control_channel->cread(&file_size, sizeof(file_size));
      system("mkdir -p received");
      string out_path = "received/" + filename;
      ofstream outfile(out_path, ios::binary);
      __int64_t remaining = file_size;
      __int64_t offset = 0;
      while (remaining > 0){
	int chunk = min((__int64_t)MAX_MESSAGE, remaining);
	filemsg fm_chunk(offset, chunk);
	int msg_len = sizeof(filemsg) + filename.size() + 1;
	char* msg_buf = new char[msg_len];
	memcpy(msg_buf, &fm_chunk,sizeof(filemsg));
	strcpy(msg_buf + sizeof(filemsg), filename.c_str());
	control_channel->cwrite(msg_buf,msg_len);
	char* response_buf = new char[chunk];
	int bytes_read = control_channel->cread(response_buf, chunk);
	outfile.write(response_buf, bytes_read);

	delete[] msg_buf;
	delete[] response_buf;
	offset += bytes_read;
	remaining -= bytes_read;
      }
      outfile.close();
      delete[] request;
    } else if (optind == argc) {
      string fname = to_string(p) + ".csv";
      filemsg fm(0, 0);
      int msg_size = sizeof(filemsg) + fname.size() + 1;
      char* request = new char[msg_size];
      memcpy(request, &fm, sizeof(filemsg));
      strcpy(request + sizeof(filemsg), fname.c_str());
      control_channel->cwrite(request, msg_size);
      __int64_t file_size;
      control_channel->cread(&file_size, sizeof(file_size));
      system("mkdir -p received");
      string out_path = "received/x" + to_string(p) + ".csv";
      ofstream outfile(out_path, ios::binary);
      __int64_t remaining = file_size;
      __int64_t offset = 0;
	while (remaining > 0) {
	  int chunk = min((__int64_t)MAX_MESSAGE, remaining);
	  filemsg fm_chunk(offset, chunk);
	  int msg_len = sizeof(filemsg) + fname.size() + 1;
	  char* msg_buf = new char[msg_len];
	  memcpy(msg_buf, &fm_chunk, sizeof(filemsg));
	  strcpy(msg_buf + sizeof(filemsg), fname.c_str());
	  control_channel->cwrite(msg_buf, msg_len);
	  char* response_buf = new char[chunk];
	  int bytes_read = control_channel->cread(response_buf, chunk);
	  outfile.write(response_buf, bytes_read);
	  delete[] msg_buf;
	  delete[] response_buf;
	  offset += bytes_read;
	  remaining -= bytes_read;
	}
      outfile.close();
      delete[] request;
    } else {
      datamsg d(p, t, e);
      control_channel->cwrite(&d, sizeof(datamsg));
      double result;
      control_channel->cread(&result, sizeof(double));
      cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << result << endl;
    }
    */
    MESSAGE_TYPE q = QUIT_MSG;
    for (auto i : data_channels){
      i->cwrite(&q,sizeof(q));
      delete i;
    }

    control_channel->cwrite(&q, sizeof(q));
    delete control_channel;

    wait(NULL);

    cout <<"Client-side is done and exited" << endl;
    cout << "Server terminated" << endl;
  }
  return 0;
}

/*
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
*/
