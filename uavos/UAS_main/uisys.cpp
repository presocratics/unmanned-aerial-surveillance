/*
 * uisys.cpp
 *
 *  Created on: Mar 06, 2013
 *      Author: Shubham Gupta
 */
#include <uisys.hpp>

using namespace std;
static UAS_ui uiModule;
static struct termios initial_settings, new_settings;
static int peek_character = -1;

void initUisys()
{
 	uiModule.init_ui();
}
void* runUisys(void*)
{
	if(uiModule.isInitialized())
    {
        uiModule.load_ui();
        cout<<"Module initialized!!\n";
    }
    else
    cout<<"Module not initialized!!\n";
    while(true){
    if(uiModule.poll_state("READ DATA")==ON)
    {
        nodelay(stdscr, TRUE);
        cout<<"Reading State\n";
        if(kbhit()==1)
        {
            //cout<<"Keyboard interrupt received\n";
            close_keyboard();
            uiModule.change_state_val("GPS",OFF);
            uiModule.change_state_val("IMU",OFF);
            uiModule.change_state_val("CAMERA ANGLE",OFF);
            uiModule.change_state_val("READ DATA",OFF);
        }
        if(uiModule.poll_state("GPS")==ON)
        //Print GPS Coordinates
        if(uiModule.poll_state("IMU")==ON)
        //Print IMU Values
        if(uiModule.poll_state("CAMERA ANGLE")==ON);
        //Print Camera Angle Values
    }
    else if(uiModule.poll_state("CHECK STATE")==ON)
    {
    	//print system state
        cout<<"Checking State\n";
    	uiModule.change_state_val("CHECK STATE",OFF);
    }
    else if(uiModule.poll_state("CHANGE STATE")==ON)
    {
        cout<<"Setting State\n";
        bool flag=true;
        while(true){
        cout<<"Current Status of SYSTEM : \n";//Print out current system state
        cout<<"Select from the following options >>>\n";
        cout<<"Idle					     1\n";
        cout<<"Auto 				     2\n";
        cout<<"Semi-Auto			     3\n";
        cout<<"End					     4\n";
        char ch;
        cin>>ch;
        switch(ch)
        {
        	case '1':
        	//Switch to Manual;
            cout<<"Switching to Idle state\n";
        	break;
        	case '2':
        	//Switch to Auto
            cout<<"Switching to Auto state\n";
        	break;
        	case '3':
        	//Switch to Semi-Auto
            cout<<"Switching to Semi-Auto state\n";
        	break;
        	case '4':
        	//Switch to end state
            cout<<"Switching to end state\n";
        	break;
        	default : 
        	cout<<"Error in selection\n";
        	flag=false;
        }
        if(flag==true)
        break;
    		}
        uiModule.change_state_val("CHANGE STATE",OFF);
    }
    else if(uiModule.poll_state("EXIT")==ON)
    {
    	cout<<"Hello! Thanks for using this UI!\n";
    	exit(0);
    }
    else
    {
        bool flag=true;
        while(true){
        cout<<"Welcome to the UI\n";
        cout<<"Select from the options below\n";
        cout<<"Read Data             1\n";
        cout<<"Check State           2\n";
        cout<<"Set State             3\n";
        cout<<"Exit                  4\n";
        //char ch=getchar();
        char ch;
        cin>>ch;
        //cout <<ch<<"\n";
        flag=true;
        switch(ch)
        {
        	case '1':
            {
        	uiModule.change_state_val("READ DATA",ON);
            bool read_flag=false;
            while(true){
            printf("Select from the following options >>\n ");
            printf("GPS                 1\n");
            printf("IMU                 2\n");
            printf("CAMERA ANGLE        3\n");
            printf("All selected        4\n");
            char ch1;
            cin>>ch1;
            switch(ch1)
            {
                case '1':
                uiModule.change_state_val("GPS",ON);
                break;
                case '2':
                uiModule.change_state_val("GPS",ON);
                break;
                case '3':
                uiModule.change_state_val("GPS",ON);
                break;
                case '4':
                read_flag=true;

            }
            if(read_flag==true)
            {
                init_keyboard();
                break;
            }
            }
           	break;
            }
        	case '2':
        	uiModule.change_state_val("CHECK STATE",ON);
           	break;
        	case '3':
        	uiModule.change_state_val("CHANGE STATE",ON);
            break;
        	case '4':
        	uiModule.change_state_val("EXIT",ON);
            close_keyboard();
          	break;
        	default:
        	cout<<"Error in selection!!\n";
        	flag=false;
        }
        if(flag==true)
       	break;
   			}
	}
    	}
}

//changing terminal keyboard input definitions for the time read_keyboard is called 
void init_keyboard()
{
    tcgetattr(0,&initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_lflag &= ~ISIG;
    new_settings.c_cc[VMIN] = 1;
    new_settings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
}
//resetting it back to the original settings
void close_keyboard()
{
    tcsetattr(0, TCSANOW, &initial_settings);
}

//defined my own version of kbhit...
int kbhit()
{
    char ch;
    int nread;

    if(peek_character != -1)
        return 1;
    new_settings.c_cc[VMIN]=0;
    tcsetattr(0, TCSANOW, &new_settings);
    nread = read(0,&ch,1);
    new_settings.c_cc[VMIN]=1;
    tcsetattr(0, TCSANOW, &new_settings);

    if(nread == 1) {
        peek_character = ch;
        return 1;
    }
    return 0;
}